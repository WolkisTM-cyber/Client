#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class Flight : public Module {
public:
    Flight() : Module("Flight", "Flight", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.5f, 0.1f, 5.0f));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"HypixelGlide", "Creative", "Motion"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        auto* speedSetting = GetSetting("Speed");
        auto* modeSetting = GetSetting("Mode");
        if (!speedSetting || !modeSetting) { env->DeleteLocalRef(player); return; }

        float speed = speedSetting->fVal;
        int mode = modeSetting->modeVal;
        tick_++;

        if (mode == 0) { // HypixelGlide (Pulse gravity to match physics)
            double motionY = (tick_ % 4 == 0) ? -0.03125 : 0.0;
            env->SetDoubleField(player, c.motionY, motionY);
        } else if (mode == 1) { // Creative
            auto caps = JNIHelper::GetPlayerCapabilities(env);
            if (caps) {
                env->SetBooleanField(caps, c.isFlyingF, JNI_TRUE);
                env->DeleteLocalRef(caps);
            }
        } else { // Motion
            double motionY = 0.0;
            if (GetAsyncKeyState(' ') & 0x8000) motionY = speed;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) motionY = -speed;
            env->SetDoubleField(player, c.motionY, motionY);
        }

        env->DeleteLocalRef(player);
    }

private:
    int tick_ = 0;
};

