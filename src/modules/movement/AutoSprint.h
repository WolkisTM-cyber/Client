#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoSprintMod : public Module {
public:
    AutoSprintMod() : Module("AutoSprint", "Auto Sprint", Category::Movement, 0) {
        AddSetting(Setting::BoolSetting("CheckHunger", "Check Hunger", true));
        AddSetting(Setting::BoolSetting("CheckWater", "Check Water", true));
        AddSetting(Setting::BoolSetting("CheckSneak", "Check Sneak", true));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Forward", "Always", "Legit"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        auto* mode = GetSetting("Mode");
        auto* checkHunger = GetSetting("CheckHunger");
        auto* checkWater = GetSetting("CheckWater");
        auto* checkSneak = GetSetting("CheckSneak");
        if (!mode || !checkHunger || !checkWater || !checkSneak) {
            env->DeleteLocalRef(player);
            return;
        }

        // Edge case: already sprinting
        if (env->CallBooleanMethod(player, c.isSprinting)) {
            env->DeleteLocalRef(player);
            return;
        }

        // Edge case: sneaking
        if (checkSneak->bVal && env->CallBooleanMethod(player, c.isSneaking)) {
            env->DeleteLocalRef(player);
            return;
        }

        // Edge case: check food/hunger
        if (checkHunger->bVal) {
            jmethodID getFoodStats = env->GetMethodID(c.entityPlayer, "getFoodStats",
                "()Lnet/minecraft/util/FoodStats;");
            if (getFoodStats) {
                jobject foodStats = env->CallObjectMethod(player, getFoodStats);
                if (foodStats) {
                    jmethodID getFoodLevel = env->GetMethodID(
                        env->GetObjectClass(foodStats), "getFoodLevel", "()I");
                    if (getFoodLevel) {
                        int food = env->CallIntMethod(foodStats, getFoodLevel);
                        if (food <= 6) { // Hunger threshold
                            env->DeleteLocalRef(foodStats);
                            env->DeleteLocalRef(player);
                            return;
                        }
                    }
                    env->DeleteLocalRef(foodStats);
                }
            }
        }

        // Edge case: check if in water/lava
        if (checkWater->bVal) {
            jmethodID isInWater = env->GetMethodID(c.entity, "isInWater", "()Z");
            jmethodID isInLava = env->GetMethodID(c.entity, "isInLava", "()Z");
            if ((isInWater && env->CallBooleanMethod(player, isInWater)) ||
                (isInLava && env->CallBooleanMethod(player, isInLava))) {
                env->DeleteLocalRef(player);
                return;
            }
        }

        int modeVal = mode->modeVal;
        bool shouldSprint = false;

        switch (modeVal) {
        case 0: // Forward
            {
                jobject mi = env->GetObjectField(player, c.movementInputF);
                if (mi) {
                    float forward = env->GetFloatField(mi, c.moveForward);
                    shouldSprint = forward > 0.0f;
                    env->DeleteLocalRef(mi);
                }
            }
            break;
        case 1: // Always
            shouldSprint = true;
            break;
        case 2: // Legit
            {
                jobject mi = env->GetObjectField(player, c.movementInputF);
                if (mi) {
                    float forward = env->GetFloatField(mi, c.moveForward);
                    float strafe = env->GetFloatField(mi, c.moveStrafe);
                    shouldSprint = forward > 0.0f && strafe == 0.0f;
                    env->DeleteLocalRef(mi);
                }
            }
            break;
        }

        if (shouldSprint) {
            env->CallVoidMethod(player, c.setSprinting, JNI_TRUE);
        }

        env->DeleteLocalRef(player);
    }
};
