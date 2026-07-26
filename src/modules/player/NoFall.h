#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoFall : public Module {
public:
    NoFall() : Module("NoFall", "No Fall", Category::Player, 0) {
        AddSetting(Setting::FloatSetting("Distance", "Distance", 3.0f, 1.0f, 10.0f));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Packet", "Ground", "Edit"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        auto* dist = GetSetting("Distance");
        float maxDist = dist ? dist->fVal : 3.0f;
        float fallDist = env->GetFloatField(player, c.fallDistance);

        if (fallDist > maxDist) {
            auto* mode = GetSetting("Mode");
            if (mode) {
                switch (mode->modeVal) {
                case 0: // Packet - set ground to true
                    env->SetBooleanField(player, c.onGround, JNI_TRUE);
                    break;
                case 1: // Ground blink - set onGround
                    env->SetBooleanField(player, c.onGround, JNI_TRUE);
                    break;
                case 2: // Edit - set fall distance
                    env->SetFloatField(player, c.fallDistance, 0.0f);
                    break;
                }
            }
        }

        env->DeleteLocalRef(player);
    }
};
