#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Step : public Module {
public:
    Step() : Module("Step", "Step", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Height", "Height", 1.0f, 0.5f, 2.0f));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto* height = GetSetting("Height");
        float h = height ? height->fVal : 1.0f;
        env->SetFloatField(player, JNIHelper::Get().stepHeight, h);
        env->DeleteLocalRef(player);
    }

    void OnDisable(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        env->SetFloatField(player, JNIHelper::Get().stepHeight, 0.5f);
        env->DeleteLocalRef(player);
    }
};
