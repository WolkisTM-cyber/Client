#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Eagle : public Module {
public:
    Eagle() : Module("Eagle", "Eagle", Category::Player, 0) {}

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

            // At edge - sneak
            if (speed > 0.01) {
                env->SetBooleanField(player, c.onGround, false);
                env->SetDoubleField(player, c.motionY, -0.1);
            }
        }

        env->DeleteLocalRef(player);
    }
};
