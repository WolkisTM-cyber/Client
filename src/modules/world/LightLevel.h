#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class LightLevel : public Module {
public:
    LightLevel() : Module("LightLevel", "Light Level", Category::World, 0) {
        AddSetting(Setting::BoolSetting("NightVision", "Night Vision", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();

        if (GetSetting("NightVision")->bVal) {
            // Set gamma to max
            jfieldID gammaField = c.gammaSetting;
            if (gammaField) {
                jobject gs = env->GetObjectField(mc, c.gameSettingsF);
                if (gs) {
                    float curGamma = env->GetFloatField(gs, gammaField);
                    if (curGamma < 100.0f) {
                        env->SetFloatField(gs, gammaField, 100.0f);
                    }
                    env->DeleteLocalRef(gs);
                }
            }
        }

        env->DeleteLocalRef(mc);
    }
};
