#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoFov : public Module {
public:
    NoFov() : Module("NoFov", "No Fov", Category::Visual, 0) {
        AddSetting(Setting::FloatSetting("Fov", "FOV", 120.0f, 30.0f, 180.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jobject gs = env->GetObjectField(mc, c.gameSettingsF);
        if (!gs) { env->DeleteLocalRef(mc); return; }

        jfieldID fovField = env->GetFieldID(c.gameSettings, "fovSetting", "F");
        if (fovField) {
            float targetFov = GetSetting("Fov")->fVal;
            float currentFov = env->GetFloatField(gs, fovField);
            if (abs(currentFov - targetFov) > 0.1f) {
                env->SetFloatField(gs, fovField, targetFov);
            }
        }

        env->DeleteLocalRef(gs);
        env->DeleteLocalRef(mc);
    }
};
