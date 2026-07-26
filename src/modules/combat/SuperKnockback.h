#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class SuperKnockback : public Module {
public:
    SuperKnockback() : Module("SuperKnockback", "Super Knockback", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("MotionX", "Motion X", 3.0f, 1.0f, 10.0f));
        AddSetting(Setting::FloatSetting("MotionY", "Motion Y", 0.5f, 0.1f, 3.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        int hitTime = env->GetIntField(player, c.hurtTime);
        if (hitTime > 0 && hitTime < 3) {
            float mx = GetSetting("MotionX")->fVal;
            float my = GetSetting("MotionY")->fVal;
            double motionX = env->GetDoubleField(player, c.motionX);
            double motionZ = env->GetDoubleField(player, c.motionZ);
            env->SetDoubleField(player, c.motionX, motionX * mx);
            env->SetDoubleField(player, c.motionZ, motionZ * mx);
            env->SetDoubleField(player, c.motionY, my);
        }

        env->DeleteLocalRef(player);
    }
};
