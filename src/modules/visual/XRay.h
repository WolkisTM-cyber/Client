#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class XRay : public Module {
public:
    XRay() : Module("XRay", "XRay", Category::Visual, 0) {
        AddSetting(Setting::FloatSetting("Gamma", "Gamma", 10.0f, 1.0f, 100.0f));
    }

    void OnEnable(JNIEnv* env) override {
        auto gs = JNIHelper::GetGameSettings(env);
        if (!gs) return;
        originalGamma_ = env->GetFloatField(gs, JNIHelper::Get().gammaSetting);
        env->SetFloatField(gs, JNIHelper::Get().gammaSetting, 10.0f);
        env->DeleteLocalRef(gs);
    }

    void OnDisable(JNIEnv* env) override {
        auto gs = JNIHelper::GetGameSettings(env);
        if (!gs) return;
        env->SetFloatField(gs, JNIHelper::Get().gammaSetting, originalGamma_);
        env->DeleteLocalRef(gs);
    }

private:
    float originalGamma_ = 1.0f;
};
