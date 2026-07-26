#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoPush : public Module {
public:
    NoPush() : Module("NoPush", "No Push", Category::Combat, 0) {
        AddSetting(Setting::BoolSetting("Players", "Players", true));
        AddSetting(Setting::BoolSetting("Blocks", "Blocks", false));
        AddSetting(Setting::BoolSetting("Water", "Water", false));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        if (GetSetting("Players")->bVal) {
            env->SetDoubleField(player, c.motionX, 0);
            env->SetDoubleField(player, c.motionZ, 0);
        }
        if (GetSetting("Water")->bVal) {
            jmethodID isInWater = env->GetMethodID(env->GetObjectClass(player), "isInWater", "()Z");
            if (isInWater) {
                bool inWater = env->CallBooleanMethod(player, isInWater);
                if (inWater) {
                    env->SetDoubleField(player, c.motionX, 0);
                    env->SetDoubleField(player, c.motionZ, 0);
                }
            }
        }

        env->DeleteLocalRef(player);
    }
};
