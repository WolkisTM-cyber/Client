#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoRod : public Module {
public:
    AutoRod() : Module("AutoRod", "Auto Rod", Category::Combat, 0) {
        AddSetting(Setting::BoolSetting("AutoSwitch", "Auto Switch", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jobject heldItem = env->CallObjectMethod(player, c.getHeldItem);
        if (heldItem) {
            bool isRod = IsFishingRod(env, heldItem);
            env->DeleteLocalRef(heldItem);
            if (isRod) return;
        }

        if (GetSetting("AutoSwitch")->bVal) {
            for (int slot = 0; slot < 9; slot++) {
                jobject inv = env->CallObjectMethod(player,
                    env->GetMethodID(c.entityPlayer, "getInventory",
                        "()Lnet/minecraft/entity/player/InventoryPlayer;"));
                if (!inv) { env->DeleteLocalRef(player); return; }

                jmethodID getStack = env->GetMethodID(
                    env->GetObjectClass(inv), "getStackInSlot", "(I)Lnet/minecraft/item/ItemStack;");
                jobject stack = env->CallObjectMethod(inv, getStack, slot);
                if (stack && IsFishingRod(env, stack)) {
                    env->SetIntField(inv, env->GetFieldID(
                        env->GetObjectClass(inv), "currentItem", "I"), slot);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                    env->DeleteLocalRef(stack);
                    env->DeleteLocalRef(inv);
                    break;
                }
                if (stack) env->DeleteLocalRef(stack);
                env->DeleteLocalRef(inv);
            }
        }

        env->DeleteLocalRef(player);
    }

    bool IsFishingRod(JNIEnv* env, jobject stack) {
        auto& c = JNIHelper::Get();
        jobject item = env->CallObjectMethod(stack, c.getItemFromStack);
        if (!item) return false;

        jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
        bool isRod = false;
        if (name) {
            const char* str = env->GetStringUTFChars(name, nullptr);
            if (str) isRod = strstr(str, "fishingRod") != nullptr;
            env->ReleaseStringUTFChars(name, str);
            env->DeleteLocalRef(name);
        }
        env->DeleteLocalRef(item);
        return isRod;
    }
};
