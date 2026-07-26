#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoArmor : public Module {
public:
    AutoArmor() : Module("AutoArmor", "Auto Armor", Category::Player, 0) {
        AddSetting(Setting::BoolSetting("OpenInventory", "Open Inventory", true));
        AddSetting(Setting::IntSetting("Delay", "Delay (ticks)", 10, 1, 40));
    }

    void OnTick(JNIEnv* env) override {
        tick_++;
        auto* delay = GetSetting("Delay");
        if (tick_ < (delay ? delay->iVal : 10)) return;
        tick_ = 0;

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        jobject inventory = env->CallObjectMethod(player,
            env->GetMethodID(JNIHelper::Get().entityPlayer, "getInventory",
                "()Lnet/minecraft/entity/player/InventoryPlayer;"));
        if (!inventory) { env->DeleteLocalRef(player); return; }

        // Check armor slots
        jmethodID getStackInSlot = env->GetMethodID(
            env->GetObjectClass(inventory), "getStackInSlot", "(I)Lnet/minecraft/item/ItemStack;");
        jmethodID armorInventory = env->GetMethodID(
            env->GetObjectClass(inventory), "armorItemInSlot", "(I)Lnet/minecraft/item/ItemStack;");

        static const int armorSlots[] = { 3, 2, 1, 0 }; // helmet, chest, legs, boots
        for (int slot : armorSlots) {
            if (!getStackInSlot) break;
            jobject current = env->CallObjectMethod(inventory, armorInventory ? armorInventory : getStackInSlot, slot);
            if (!current) {
                // Empty slot - find best armor in hotbar
                for (int i = 9; i < 36; i++) {
                    jobject stack = env->CallObjectMethod(inventory, getStackInSlot, i);
                    if (stack) {
                        // Check if it's armor
                        jobject item = env->CallObjectMethod(stack, JNIHelper::Get().getItemFromStack);
                        if (item) {
                            jstring name = (jstring)env->CallObjectMethod(item, JNIHelper::Get().getUnlocalizedName);
                            if (name) {
                                const char* str = env->GetStringUTFChars(name, nullptr);
                                if (str) {
                                    std::string itemName(str);
                                    bool isRightType = false;
                                    if (slot == 3 && itemName.find("helmet") != std::string::npos) isRightType = true;
                                    if (slot == 2 && itemName.find("chestplate") != std::string::npos) isRightType = true;
                                    if (slot == 1 && itemName.find("leggings") != std::string::npos) isRightType = true;
                                    if (slot == 0 && itemName.find("boots") != std::string::npos) isRightType = true;

                                    if (isRightType) {
                                        // Equip it
                                        jmethodID getFirstStack = env->GetMethodID(
                                            JNIHelper::Get().entityPlayer,
                                            "dropPlayerItemWithRandomChoice",
                                            "(Lnet/minecraft/item/ItemStack;Z)V");
                                        // Use right-click to equip
                                        jmethodID rightClick = env->GetMethodID(
                                            JNIHelper::Get().entityPlayer,
                                            "dropItem", "(ZI)Lnet/minecraft/entity/item/EntityItem;");
                                    }
                                    env->ReleaseStringUTFChars(name, str);
                                }
                                env->DeleteLocalRef(name);
                            }
                            env->DeleteLocalRef(item);
                        }
                        env->DeleteLocalRef(stack);
                        break;
                    }
                }
            }
            if (current) env->DeleteLocalRef(current);
        }

        env->DeleteLocalRef(inventory);
        env->DeleteLocalRef(player);
    }

private:
    int tick_ = 0;
};
