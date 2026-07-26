#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class IceSpeed : public Module {
public:
    IceSpeed() : Module("IceSpeed", "Ice Speed", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Boost", "Speed Boost", 1.4f, 1.0f, 2.5f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        bool onGround = env->GetBooleanField(player, c.onGround);
        if (onGround) {
            double mx = JNIHelper::GetMotionX(env, player);
            double mz = JNIHelper::GetMotionZ(env, player);
            float boost = GetSetting("Boost")->fVal;
            JNIHelper::SetMotion(env, player, mx * boost, JNIHelper::GetMotionY(env, player), mz * boost);
        }

        env->DeleteLocalRef(player);
    }
};

