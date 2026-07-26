#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class InvCleaner : public Module {
public:
    InvCleaner() : Module("InvCleaner", "Inv Cleaner", Category::Player, 0) {
        AddSetting(Setting::BoolSetting("Tools", "Keep Tools", true));
        AddSetting(Setting::BoolSetting("Blocks", "Keep Blocks", true));
        AddSetting(Setting::BoolSetting("Weapons", "Keep Weapons", true));
        AddSetting(Setting::IntSetting("Delay", "Delay (ms)", 100, 20, 300));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jobject inv = env->CallObjectMethod(player, env->GetMethodID(c.entityPlayer, "getInventory", "()Lnet/minecraft/entity/player/InventoryPlayer;"));
        if (!inv) { env->DeleteLocalRef(player); return; }

        jmethodID getStack = env->GetMethodID(env->GetObjectClass(inv), "getStackInSlot", "(I)Lnet/minecraft/item/ItemStack;");
        for (int slot = 9; slot < 36; slot++) {
            jobject stack = env->CallObjectMethod(inv, getStack, slot);
            if (stack) {
                jobject item = env->CallObjectMethod(stack, c.getItemFromStack);
                if (item) {
                    jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
                    if (name) {
                        const char* str = env->GetStringUTFChars(name, nullptr);
                        if (str) {
                            bool isJunk = (strstr(str, "stick") || strstr(str, "string") || strstr(str, "dirt"));
                            if (isJunk) {
                                auto pc = JNIHelper::GetPlayerController(env);
                                if (pc) {
                                    jmethodID slotClick = env->GetMethodID(env->GetObjectClass(pc), "windowClick", "(IIIILnet/minecraft/entity/player/EntityPlayer;)V");
                                    if (slotClick) env->CallVoidMethod(pc, slotClick, 0, slot, 1, 4, player); // Drop stack
                                    env->DeleteLocalRef(pc);
                                }
                            }
                            env->ReleaseStringUTFChars(name, str);
                        }
                        env->DeleteLocalRef(name);
                    }
                    env->DeleteLocalRef(item);
                }
                env->DeleteLocalRef(stack);
            }
        }

        env->DeleteLocalRef(inv);
        env->DeleteLocalRef(player);
    }
};

