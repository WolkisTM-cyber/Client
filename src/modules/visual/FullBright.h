#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class FullBright : public Module {
public:
    FullBright() : Module("FullBright", "Full Bright", Category::Visual, 0) {
        AddSetting(Setting::FloatSetting("Gamma", "Gamma", 1000.0f, 1.0f, 10000.0f));
    }

    void OnEnable(JNIEnv* env) override {
        auto gs = JNIHelper::GetGameSettings(env);
        if (!gs) return;
        originalGamma_ = env->GetFloatField(gs, JNIHelper::Get().gammaSetting);
        auto* gamma = GetSetting("Gamma");
        env->SetFloatField(gs, JNIHelper::Get().gammaSetting, gamma ? gamma->fVal : 1000.0f);
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
