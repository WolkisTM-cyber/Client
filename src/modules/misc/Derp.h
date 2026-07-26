#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Derp : public Module {
public:
    Derp() : Module("Derp", "Derp", Category::Misc, 0) {
        AddSetting(Setting::IntSetting("Speed", "Speed", 15, 1, 90));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        float yaw = env->GetFloatField(player, c.rotationYaw);
        auto* speed = GetSetting("Speed");
        yaw += speed ? speed->iVal : 15.0f;
        if (yaw > 360.0f) yaw -= 360.0f;

        env->SetFloatField(player, c.rotationYaw, yaw);
        env->SetFloatField(player, c.rotationPitch, 90.0f);

        env->DeleteLocalRef(player);
    }
};
