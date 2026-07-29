#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class KeepSprint : public Module {
public:
    KeepSprint() : Module("KeepSprint", "Keep Sprint", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.8f, 0.1f, 1.0f));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        auto* speed = GetSetting("Speed");
        if (!speed) { env->DeleteLocalRef(player); return; }

        // Keep sprinting when attacking by resetting sprint
        if (env->CallBooleanMethod(player, c.isSprinting)) {
            double motionX = env->GetDoubleField(player, c.motionX);
            double motionZ = env->GetDoubleField(player, c.motionZ);
            double currentSpeed = std::sqrt(motionX * motionX + motionZ * motionZ);
            double targetSpeed = 0.2873; // Default sprint speed

            if (currentSpeed < targetSpeed * speed->fVal) {
                // Re-apply sprint velocity
                float yaw = env->GetFloatField(player, c.rotationYaw);
                float forward = 1.0f;
                double rad = yaw * 3.141592653589793 / 180.0;
                double mx = -std::sin(rad) * targetSpeed * speed->fVal;
                double mz = std::cos(rad) * targetSpeed * speed->fVal;

                // Only boost if not exceeding desired speed
                if (currentSpeed < targetSpeed * speed->fVal || motionX * mx < 0) {
                    env->SetDoubleField(player, c.motionX, mx);
                    env->SetDoubleField(player, c.motionZ, mz);
                }
            }
        }

        env->DeleteLocalRef(player);
    }
};
