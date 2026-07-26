#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class Strafe : public Module {
public:
    Strafe() : Module("Strafe", "Strafe", Category::Movement, 0) {
        AddSetting(Setting::BoolSetting("Silent", "Silent", true));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jobject mi = env->GetObjectField(player, c.movementInputF);
        if (!mi) { env->DeleteLocalRef(player); return; }

        float forward = env->GetFloatField(mi, c.moveForward);
        float strafe = env->GetFloatField(mi, c.moveStrafe);
        env->DeleteLocalRef(mi);

        if (forward == 0 && strafe == 0) { env->DeleteLocalRef(player); return; }

        double mx = env->GetDoubleField(player, c.motionX);
        double mz = env->GetDoubleField(player, c.motionZ);
        double speed = std::sqrt(mx*mx + mz*mz);

        float yaw = env->GetFloatField(player, c.rotationYaw);
        double rad = yaw * 3.141592653589793 / 180.0;

        if (forward != 0 || strafe != 0) {
            mx = forward * -std::sin(rad) * speed + strafe * std::cos(rad) * speed;
            mz = forward * std::cos(rad) * speed - strafe * -std::sin(rad) * speed;
            env->SetDoubleField(player, c.motionX, mx);
            env->SetDoubleField(player, c.motionZ, mz);
        }

        env->DeleteLocalRef(player);
    }
};
