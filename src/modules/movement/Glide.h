#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Glide : public Module {
public:
    Glide() : Module("Glide", "Glide", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.5f, 0.1f, 1.0f));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        if (!env->GetBooleanField(player, c.onGround)) {
            double motionY = env->GetDoubleField(player, c.motionY);
            auto* speed = GetSetting("Speed");
            double factor = speed ? (1.0 - speed->fVal * 0.5) : 0.5;
            if (motionY < 0) {
                env->SetDoubleField(player, c.motionY, motionY * factor);
            }
        }

        env->DeleteLocalRef(player);
    }
};
