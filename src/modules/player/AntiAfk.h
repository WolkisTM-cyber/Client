#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <ctime>

class AntiAfk : public Module {
public:
    AntiAfk() : Module("AntiAfk", "Anti AFK", Category::Player, 0) {
        AddSetting(Setting::IntSetting("Delay", "Delay (s)", 30, 5, 120));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        clock_t now = clock();
        int delay = GetSetting("Delay")->iVal * CLOCKS_PER_SEC;
        if (now - lastMove_ < delay) return;

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        float yaw = env->GetFloatField(player, c.rotationYaw);
        env->SetFloatField(player, c.rotationYaw, yaw + 90.0f);
        env->SetFloatField(player, c.rotationPitch, 0.0f);

        // Jump
        if (env->GetBooleanField(player, c.onGround)) {
            env->SetDoubleField(player, c.motionY, 0.42);
        }

        lastMove_ = clock();
        env->DeleteLocalRef(player);
    }

private:
    clock_t lastMove_ = 0;
};
