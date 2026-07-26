#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoWeb : public Module {
public:
    NoWeb() : Module("NoWeb", "No Web", Category::Movement, 0) {
        AddSetting(Setting::BoolSetting("Motion", "Cancel Motion", true));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        if (env->GetBooleanField(player, c.isInWeb)) {
            env->SetBooleanField(player, c.isInWeb, JNI_FALSE);
            auto* motion = GetSetting("Motion");
            if (motion && motion->bVal) {
                env->SetDoubleField(player, c.motionX, 0.0);
                env->SetDoubleField(player, c.motionZ, 0.0);
            }
        }

        env->DeleteLocalRef(player);
    }
};
