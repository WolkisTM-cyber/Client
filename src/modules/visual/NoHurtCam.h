#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoHurtCam : public Module {
public:
    NoHurtCam() : Module("NoHurtCam", "No Hurt Cam", Category::Visual, 0) {}

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        env->SetIntField(player, JNIHelper::Get().hurtTime, 0);

        env->DeleteLocalRef(player);
    }
};
