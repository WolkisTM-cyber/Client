#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Timer : public Module {
public:
    Timer() : Module("Timer", "Timer", Category::Misc, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 2.0f, 0.1f, 10.0f));
        AddSetting(Setting::BoolSetting("Reverse", "Reverse", false));
    }

    void OnTick(JNIEnv* env) override {
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jobject timerObj = env->GetObjectField(mc, c.timer);
        if (timerObj) {
            auto* speed = GetSetting("Speed");
            float s = speed ? speed->fVal : 2.0f;
            env->SetFloatField(timerObj, c.timerSpeed, s);
            env->DeleteLocalRef(timerObj);
        }

        env->DeleteLocalRef(mc);
    }

    void OnDisable(JNIEnv* env) override {
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;
        jobject timerObj = env->GetObjectField(mc, JNIHelper::Get().timer);
        if (timerObj) {
            env->SetFloatField(timerObj, JNIHelper::Get().timerSpeed, 1.0f);
            env->DeleteLocalRef(timerObj);
        }
        env->DeleteLocalRef(mc);
    }
};
