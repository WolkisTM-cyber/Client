#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"

class AutoGapple : public Module {
public:
    AutoGapple() : Module("AutoGapple", "Auto Gapple", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Health", "Health Threshold", 12.0f, 1.0f, 20.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID getHealth = env->GetMethodID(c.entityLivingBase, "getHealth", "()F");
        if (getHealth) {
            float hp = env->CallFloatMethod(player, getHealth);
            if (hp <= GetSetting("Health")->fVal) {
                // Auto eat Golden Apple from hotbar
                jobject inv = env->CallObjectMethod(player,
                    env->GetMethodID(c.entityPlayer, "getInventory", "()Lnet/minecraft/entity/player/InventoryPlayer;"));
                if (inv) {
                    jmethodID getStack = env->GetMethodID(env->GetObjectClass(inv), "getStackInSlot", "(I)Lnet/minecraft/item/ItemStack;");
                    for (int slot = 0; slot < 9; slot++) {
                        jobject stack = env->CallObjectMethod(inv, getStack, slot);
                        if (stack) {
                            jobject item = env->CallObjectMethod(stack, c.getItemFromStack);
                            if (item) {
                                jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
                                if (name) {
                                    const char* str = env->GetStringUTFChars(name, nullptr);
                                    if (str && strstr(str, "appleGold")) {
                                        env->SetIntField(inv, env->GetFieldID(env->GetObjectClass(inv), "currentItem", "I"), slot);
                                    }
                                    env->ReleaseStringUTFChars(name, str);
                                    env->DeleteLocalRef(name);
                                }
                                env->DeleteLocalRef(item);
                            }
                            env->DeleteLocalRef(stack);
                        }
                    }
                    env->DeleteLocalRef(inv);
                }
            }
        }
        env->DeleteLocalRef(player);
    }
};
