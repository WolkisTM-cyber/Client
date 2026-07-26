#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AntiHunger : public Module {
public:
    AntiHunger() : Module("AntiHunger", "Anti Hunger", Category::Movement, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        bool onGround = env->GetBooleanField(player, c.onGround);

        // Prevent sprinting and jumping flag from consuming hunger
        if (onGround) {
            double mx = env->GetDoubleField(player, c.motionX);
            double mz = env->GetDoubleField(player, c.motionZ);
            double speed = sqrt(mx*mx + mz*mz);
            if (speed > 0.1) {
                env->SetDoubleField(player, c.motionX, mx * 0.7);
                env->SetDoubleField(player, c.motionZ, mz * 0.7);
            }
        }

        // Set foodExhaustionLevel = 0
        jfieldID foodExhaustionField = env->GetFieldID(c.entityPlayer, "foodExhaustionLevel", "F");
        if (foodExhaustionField) {
            env->SetFloatField(player, foodExhaustionField, 0.0f);
        }

        env->DeleteLocalRef(player);
    }
};
