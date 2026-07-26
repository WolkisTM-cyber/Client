#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Parkour : public Module {
public:
    Parkour() : Module("Parkour", "Parkour", Category::Movement, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        bool onGround = env->GetBooleanField(player, c.onGround);
        if (!onGround) { env->DeleteLocalRef(player); return; }

        double mx = env->GetDoubleField(player, c.motionX);
        double mz = env->GetDoubleField(player, c.motionZ);
        double speed = sqrt(mx*mx + mz*mz);

        if (speed > 0.05) {
            env->SetDoubleField(player, c.motionY, 0.42);
        }

        env->DeleteLocalRef(player);
    }
};
