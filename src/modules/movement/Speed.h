#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class Speed : public Module {
public:
    Speed() : Module("Speed", "Speed", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.32f, 0.1f, 1.0f));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"YPort", "NCP", "BHop", "Strafe"}, 0));
        AddSetting(Setting::BoolSetting("Jump", "Jump", true));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        auto* speedSetting = GetSetting("Speed");
        auto* modeSetting = GetSetting("Mode");
        auto* jumpSetting = GetSetting("Jump");
        if (!speedSetting || !modeSetting || !jumpSetting) { env->DeleteLocalRef(player); return; }

        double speed = speedSetting->fVal;
        bool onGround = env->GetBooleanField(player, c.onGround);
        float yaw = env->GetFloatField(player, c.rotationYaw);

        jobject mi = env->GetObjectField(player, c.movementInputF);
        if (!mi) { env->DeleteLocalRef(player); return; }

        float forward = env->GetFloatField(mi, c.moveForward);
        float strafe = env->GetFloatField(mi, c.moveStrafe);
        env->DeleteLocalRef(mi);

        if (forward == 0 && strafe == 0) { env->DeleteLocalRef(player); return; }

        if (forward != 0 && strafe != 0) speed *= 0.707;

        double rad = yaw * 3.141592653589793 / 180.0;
        double sine = -std::sin(rad);
        double cose = std::cos(rad);

        double mx = forward * sine * speed + strafe * cose * speed;
        double mz = forward * cose * speed - strafe * sine * speed;

        switch (modeSetting->modeVal) {
        case 0: // YPort
            if (onGround && jumpSetting->bVal) {
                env->SetDoubleField(player, c.motionY, 0.42);
                env->SetDoubleField(player, c.motionX, mx * 1.5);
                env->SetDoubleField(player, c.motionZ, mz * 1.5);
            } else {
                env->SetDoubleField(player, c.motionX, mx);
                env->SetDoubleField(player, c.motionZ, mz);
            }
            break;
        case 1: // NCP
            if (onGround) {
                if (jumpSetting->bVal) env->SetDoubleField(player, c.motionY, 0.4);
                env->SetDoubleField(player, c.motionX, mx * 1.2);
                env->SetDoubleField(player, c.motionZ, mz * 1.2);
            }
            break;
        case 2: // BHop
            if (onGround) {
                if (jumpSetting->bVal) env->SetDoubleField(player, c.motionY, 0.42);
                env->SetDoubleField(player, c.motionX, mx);
                env->SetDoubleField(player, c.motionZ, mz);
            } else {
                double currentMx = env->GetDoubleField(player, c.motionX);
                double currentMz = env->GetDoubleField(player, c.motionZ);
                double currentSpd = std::sqrt(currentMx*currentMx + currentMz*currentMz);
                if (currentSpd < speed) {
                    double factor = speed / currentSpd;
                    env->SetDoubleField(player, c.motionX, currentMx * factor);
                    env->SetDoubleField(player, c.motionZ, currentMz * factor);
                }
            }
            break;
        case 3: // Strafe
            env->SetDoubleField(player, c.motionX, mx);
            env->SetDoubleField(player, c.motionZ, mz);
            break;
        }

        env->DeleteLocalRef(player);
    }
};
