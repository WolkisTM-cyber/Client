#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class SafeWalk : public Module {
public:
    SafeWalk() : Module("SafeWalk", "Safe Walk", Category::Movement, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        bool onGround = env->GetBooleanField(player, c.onGround);

        if (onGround) {
            double mx = env->GetDoubleField(player, c.motionX);
            double mz = env->GetDoubleField(player, c.motionZ);
            double speed = sqrt(mx*mx + mz*mz);
            if (speed > 0.1) {
                env->SetDoubleField(player, c.motionX, mx * 0.3);
                env->SetDoubleField(player, c.motionZ, mz * 0.3);
            }
        }

        env->DeleteLocalRef(player);
    }
};
