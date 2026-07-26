#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoSlowdown : public Module {
public:
    NoSlowdown() : Module("NoSlowdown", "No Slowdown", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.02f, 0.01f, 0.1f));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto* speed = GetSetting("Speed");
        float v = speed ? speed->fVal : 0.02f;
        auto& c = JNIHelper::Get();
        env->SetFloatField(player, c.speedInAir, v);
        env->SetFloatField(player, c.jumpMovementFactor, v);

        env->DeleteLocalRef(player);
    }
};
