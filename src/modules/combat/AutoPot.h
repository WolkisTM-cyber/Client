#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoPot : public Module {
public:
    AutoPot() : Module("AutoPot", "Auto Pot", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Health", "Health", 10.0f, 1.0f, 20.0f));
        AddSetting(Setting::FloatSetting("Delay", "Delay (s)", 1.0f, 0.1f, 5.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        clock_t now = clock();
        if ((float)(now - lastPot_) / CLOCKS_PER_SEC < GetSetting("Delay")->fVal) return;

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        float health = 20.0f;
        jmethodID getHealth = env->GetMethodID(c.entityLivingBase, "getHealth", "()F");
        if (getHealth) health = env->CallFloatMethod(player, getHealth);
        if (env->ExceptionCheck()) env->ExceptionClear();

        if (health >= GetSetting("Health")->fVal) { env->DeleteLocalRef(player); return; }

        // Search hotbar for potion
        jobject inv = env->CallObjectMethod(player,
            env->GetMethodID(c.entityPlayer, "getInventory",
                "()Lnet/minecraft/entity/player/InventoryPlayer;"));
        if (!inv) { env->DeleteLocalRef(player); return; }

        jmethodID getStack = env->GetMethodID(
            env->GetObjectClass(inv), "getStackInSlot", "(I)Lnet/minecraft/item/ItemStack;");

        for (int slot = 36; slot < 45; slot++) {
            jobject stack = env->CallObjectMethod(inv, getStack, slot);
            if (!stack) continue;

            jobject item = env->CallObjectMethod(stack, c.getItemFromStack);
            if (!item) { env->DeleteLocalRef(stack); continue; }

            jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
            bool isPot = false;
            if (name) {
                const char* str = env->GetStringUTFChars(name, nullptr);
                if (str) isPot = strstr(str, "potion") != nullptr;
                env->ReleaseStringUTFChars(name, str);
                env->DeleteLocalRef(name);
            }
            env->DeleteLocalRef(item);

            if (isPot) {
                int hotbarSlot = slot - 36;
                env->SetIntField(inv, env->GetFieldID(
                    env->GetObjectClass(inv), "currentItem", "I"), hotbarSlot);
                if (env->ExceptionCheck()) env->ExceptionClear();

                // Right click
                jobject controller = JNIHelper::GetPlayerController(env);
                if (controller) {
                    jmethodID rightClick = env->GetMethodID(
                        env->GetObjectClass(controller), "sendUseItem",
                        "(Lnet/minecraft/entity/player/EntityPlayer;Lnet/minecraft/world/World;Lnet/minecraft/item/ItemStack;)V");
                    if (rightClick) {
                        env->CallVoidMethod(controller, rightClick, player, nullptr, nullptr);
                        if (env->ExceptionCheck()) env->ExceptionClear();
                        lastPot_ = clock();
                    }
                }
                env->DeleteLocalRef(controller);
                env->DeleteLocalRef(stack);
                break;
            }
            env->DeleteLocalRef(stack);
        }

        env->DeleteLocalRef(inv);
        env->DeleteLocalRef(player);
    }

private:
    clock_t lastPot_ = 0;
};
