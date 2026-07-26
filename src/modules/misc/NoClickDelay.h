#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoClickDelay : public Module {
public:
    NoClickDelay() : Module("NoClickDelay", "No Click Delay", Category::Misc, 0) {}

    void OnTick(JNIEnv* env) override {
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        env->SetIntField(mc, JNIHelper::Get().leftClickCounter, 0);

        env->DeleteLocalRef(mc);
    }
};
