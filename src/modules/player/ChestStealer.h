#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <random>

class ChestStealer : public Module {
public:
    ChestStealer() : Module("ChestStealer", "Chest Stealer", Category::Player, 0) {
        AddSetting(Setting::IntSetting("Delay", "Delay (ms)", 80, 40, 200));
        AddSetting(Setting::BoolSetting("Randomize", "Randomize", true));
        AddSetting(Setting::BoolSetting("AutoClose", "Auto Close", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

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

        jclass guiChest = env->FindClass("net/minecraft/client/gui/inventory/GuiChest");
        if (!guiChest || !env->IsInstanceOf(currentScreen, guiChest)) {
            if (guiChest) env->DeleteLocalRef(guiChest);
            env->DeleteLocalRef(currentScreen);
            env->DeleteLocalRef(mc);
            env->DeleteLocalRef(player);
            return;
        }
        env->DeleteLocalRef(guiChest);

        jmethodID getLowerInv = env->GetMethodID(
            env->GetObjectClass(currentScreen), "getLowerChestInventory",
            "()Lnet/minecraft/inventory/IInventory;");
        if (!getLowerInv) getLowerInv = env->GetMethodID(
            env->GetObjectClass(currentScreen), "getLowerInv",
            "()Lnet/minecraft/inventory/IInventory;");
        if (!getLowerInv) { env->DeleteLocalRef(currentScreen); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jobject chestInv = env->CallObjectMethod(currentScreen, getLowerInv);
        if (!chestInv) { env->DeleteLocalRef(currentScreen); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jmethodID getSize = env->GetMethodID(
            env->GetObjectClass(chestInv), "getSizeInventory", "()I");
        jmethodID getSlot = env->GetMethodID(
            env->GetObjectClass(chestInv), "getStackInSlot", "(I)");

        int size = env->CallIntMethod(chestInv, getSize);
        int delay = GetSetting("Delay")->iVal;
        if (GetSetting("Randomize")->bVal) {
            static std::mt19937 rng(99);
            std::uniform_int_distribution<int> dist(-15, 25);
            delay += dist(rng);
        }

        timer_ += 50;
        if (timer_ >= delay) {
            timer_ = 0;
            for (int i = 0; i < size; i++) {
                jobject stack = env->CallObjectMethod(chestInv, getSlot, i);
                if (stack) {
                    auto pc = JNIHelper::GetPlayerController(env);
                    if (pc) {
                        jmethodID slotClick = env->GetMethodID(
                            env->GetObjectClass(pc), "windowClick",
                            "(IIIILnet/minecraft/entity/player/EntityPlayer;)V");
                        if (slotClick) env->CallVoidMethod(pc, slotClick, 0, i, 0, 1, player);
                        env->DeleteLocalRef(pc);
                    }
                    env->DeleteLocalRef(stack);
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

