#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Criticals : public Module {
public:
    Criticals() : Module("Criticals", "Criticals", Category::Combat, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Packet", "MiniJump", "NCP"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        if (env->GetBooleanField(player, JNIHelper::Get().onGround)) {
            auto* mode = GetSetting("Mode");
            if (mode) {
                switch (mode->modeVal) {
                case 0: // Packet
                    env->SetDoubleField(player, JNIHelper::Get().motionY, 0.1);
                    break;
                case 1: // MiniJump
                    env->SetDoubleField(player, JNIHelper::Get().motionY, 0.2);
                    break;
                case 2: // NCP
                    env->SetDoubleField(player, JNIHelper::Get().motionY, 0.0625);
                    break;
                }
            }
        }

        env->DeleteLocalRef(player);
    }
};
