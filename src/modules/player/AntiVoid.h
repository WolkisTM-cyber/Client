#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AntiVoid : public Module {
public:
    AntiVoid() : Module("AntiVoid", "Anti Void", Category::Player, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Motion", "Jump", "Teleport"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        double posY = env->GetDoubleField(player, c.posY);

        if (posY < -10.0) {
            auto* mode = GetSetting("Mode");
            switch (mode ? mode->modeVal : 0) {
            case 0: // Motion
                env->SetDoubleField(player, c.motionY, 4.0);
                break;
            case 1: // Jump
                env->SetDoubleField(player, c.motionY, 1.0);
                break;
            case 2: // Teleport
                env->SetDoubleField(player, c.motionY, 0.0);
                env->SetDoubleField(player, c.posY, 0.0);
                break;
            }
        }

        env->DeleteLocalRef(player);
    }
};
