#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class ChestStealer : public Module {
public:
    ChestStealer() : Module("ChestStealer", "Chest Stealer", Category::Player, 0) {
        AddSetting(Setting::IntSetting("Delay", "Delay (ms)", 50, 10, 200));
        AddSetting(Setting::BoolSetting("AutoClose", "Auto Close", true));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // Check if open chest screen
        jfieldID currentScreenField = env->GetFieldID(c.minecraft, "currentScreen",
            "Lnet/minecraft/client/gui/GuiScreen;");
        if (!currentScreenField) { env->DeleteLocalRef(player); return; }

        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) { env->DeleteLocalRef(player); return; }

        jobject currentScreen = env->GetObjectField(mc, currentScreenField);
        if (!currentScreen) {
            env->DeleteLocalRef(mc);
            env->DeleteLocalRef(player);
            return;
        }

        // Get lower chest inventory
        jmethodID getLowerInv = env->GetMethodID(
            env->GetObjectClass(currentScreen), "getLowerInv",
            "()Lnet/minecraft/inventory/IInventory;");

        jclass guiChest = env->FindClass("net/minecraft/client/gui/inventory/GuiChest");
        if (!guiChest || !env->IsInstanceOf(currentScreen, guiChest)) {
            env->DeleteLocalRef(currentScreen);
            env->DeleteLocalRef(mc);
            env->DeleteLocalRef(player);
            return;
        }

        if (!getLowerInv) getLowerInv = env->GetMethodID(
            env->GetObjectClass(currentScreen), "getLowerInventory",
            "()Lnet/minecraft/inventory/IInventory;");
        if (!getLowerInv) { env->DeleteLocalRef(currentScreen); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jobject chestInv = env->CallObjectMethod(currentScreen, getLowerInv);
        if (!chestInv) { env->DeleteLocalRef(currentScreen); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jmethodID getSize = env->GetMethodID(
            env->GetObjectClass(chestInv), "getSizeInventory", "()I");
        jmethodID getSlot = env->GetMethodID(
            env->GetObjectClass(chestInv), "getStackInSlot", "(I)");
        jmethodID slotClick = env->GetMethodID(
            JNIHelper::Get().playerControllerMP, "windowClick",
            "(IIIILnet/minecraft/entity/player/EntityPlayer;)V");

        int size = env->CallIntMethod(chestInv, getSize);
        int delay = GetSetting("Delay") ? GetSetting("Delay")->iVal : 50;
        auto* autoClose = GetSetting("AutoClose");

        if (timer_ < delay) { timer_++; }
        else {
            timer_ = 0;
            // Try to take items from chest (slots 0 to size-1)
            for (int i = 0; i < size; i++) {
                jobject stack = env->CallObjectMethod(chestInv, getSlot, i);
                if (stack) {
                    // Take item - click slot to player inventory
                    auto pc = JNIHelper::GetPlayerController(env);
                    if (pc && slotClick) {
                        env->CallVoidMethod(pc, slotClick, 0, i, 0, 1, player);
                        env->DeleteLocalRef(pc);
                    }
                    env->DeleteLocalRef(stack);

                    // Check if chest is empty
                    if (i == size - 1 && autoClose && autoClose->bVal) {
                        // Close GUI
                        jmethodID closeScreen = env->GetMethodID(
                            JNIHelper::Get().entityPlayer, "closeScreen", "()V");
                        if (closeScreen) env->CallVoidMethod(player, closeScreen);
                    }
                    break;
                }
            }
        }

        env->DeleteLocalRef(chestInv);
        env->DeleteLocalRef(currentScreen);
        env->DeleteLocalRef(mc);
        env->DeleteLocalRef(player);
    }

private:
    int timer_ = 0;
};
