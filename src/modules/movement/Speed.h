#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class Speed : public Module {
public:
    Speed() : Module("Speed", "Speed", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.287f, 0.2f, 0.6f));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"HypixelBHop", "Vulcan", "NCP", "Strafe"}, 0));
        AddSetting(Setting::BoolSetting("Jump", "Auto Jump", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
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

        if (forward != 0 && strafe != 0) speed *= 0.7071;

        double rad = yaw * 3.141592653589793 / 180.0;
        double mx = forward * -std::sin(rad) * speed + strafe * std::cos(rad) * speed;
        double mz = forward * std::cos(rad) * speed - strafe * -std::sin(rad) * speed;

        int mode = modeSetting->modeVal;
        if (mode == 0) { // HypixelBHop (Friction & jump boost)
            if (onGround && jumpSetting->bVal) {
                env->SetDoubleField(player, c.motionY, 0.42);
                env->SetDoubleField(player, c.motionX, mx * 1.08);
                env->SetDoubleField(player, c.motionZ, mz * 1.08);
            } else {
                double curMx = env->GetDoubleField(player, c.motionX) * 0.98;
                double curMz = env->GetDoubleField(player, c.motionZ) * 0.98;
                env->SetDoubleField(player, c.motionX, curMx);
                env->SetDoubleField(player, c.motionZ, curMz);
            }
        } else if (mode == 1) { // Vulcan (Strict friction simulation)
            if (onGround && jumpSetting->bVal) {
                env->SetDoubleField(player, c.motionY, 0.42);
                env->SetDoubleField(player, c.motionX, mx * 1.02);
                env->SetDoubleField(player, c.motionZ, mz * 1.02);
            }
        } else if (mode == 2) { // NCP
            if (onGround) {
                if (jumpSetting->bVal) env->SetDoubleField(player, c.motionY, 0.4);
                env->SetDoubleField(player, c.motionX, mx * 1.15);
                env->SetDoubleField(player, c.motionZ, mz * 1.15);
            }
        } else if (mode == 3) { // Strafe
            env->SetDoubleField(player, c.motionX, mx);
            env->SetDoubleField(player, c.motionZ, mz);
        }

        env->DeleteLocalRef(player);
    }
};

