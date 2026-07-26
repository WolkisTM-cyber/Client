#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AntiThrow : public Module {
public:
    AntiThrow() : Module("AntiThrow", "Anti Throw", Category::Player, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        int hurtTime = env->GetIntField(player, c.hurtTime);
        if (hurtTime > 0 && hurtTime < 3) {
            env->SetDoubleField(player, c.motionX, 0);
            env->SetDoubleField(player, c.motionY, 0);
            env->SetDoubleField(player, c.motionZ, 0);
            env->SetBooleanField(player, c.onGround, true);
        }

        env->DeleteLocalRef(player);
    }
};
