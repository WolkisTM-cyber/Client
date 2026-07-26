#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Flight : public Module {
public:
    Flight() : Module("Flight", "Flight", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.5f, 0.1f, 5.0f));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Creative", "Motion", "NCP"}, 0));
    }

    void OnEnable(JNIEnv* env) override {
        auto caps = JNIHelper::GetPlayerCapabilities(env);
        if (!caps) return;
        env->SetBooleanField(caps, JNIHelper::Get().isFlyingF, JNI_TRUE);
        env->DeleteLocalRef(caps);
    }

    void OnDisable(JNIEnv* env) override {
        auto caps = JNIHelper::GetPlayerCapabilities(env);
        if (!caps) return;
        env->SetBooleanField(caps, JNIHelper::Get().isFlyingF, JNI_FALSE);
        env->DeleteLocalRef(caps);
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        auto* speedSetting = GetSetting("Speed");
        auto* modeSetting = GetSetting("Mode");
        if (!speedSetting || !modeSetting) { env->DeleteLocalRef(player); return; }

        float speed = speedSetting->fVal;
        double motionY = 0.0;

        if (GetAsyncKeyState('W') & 0x8000) {
            float yaw = env->GetFloatField(player, c.rotationYaw);
            double rad = yaw * 3.141592653589793 / 180.0;
            env->SetDoubleField(player, c.motionX, -std::sin(rad) * speed);
            env->SetDoubleField(player, c.motionZ, std::cos(rad) * speed);
        }
        if (GetAsyncKeyState(' ') & 0x8000) motionY = speed;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) motionY = -speed;

        env->SetDoubleField(player, c.motionY, motionY);

        if (modeSetting->modeVal == 0) {
            auto caps = JNIHelper::GetPlayerCapabilities(env);
            if (caps) {
                env->SetBooleanField(caps, c.isFlyingF, JNI_TRUE);
                env->DeleteLocalRef(caps);
            }
        }

        env->DeleteLocalRef(player);
    }
};
