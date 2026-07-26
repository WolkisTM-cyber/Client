#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class LongJump : public Module {
public:
    LongJump() : Module("LongJump", "Long Jump", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Boost", "Boost", 2.0f, 1.0f, 5.0f));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        if (env->GetBooleanField(player, c.onGround)) {
            jumping_ = false;
        } else if (!jumping_) {
            jumping_ = true;
            auto* boost = GetSetting("Boost");
            double b = boost ? boost->fVal : 2.0;
            float yaw = env->GetFloatField(player, c.rotationYaw);
            double rad = yaw * 3.141592653589793 / 180.0;
            env->SetDoubleField(player, c.motionX, -std::sin(rad) * b);
            env->SetDoubleField(player, c.motionZ, std::cos(rad) * b);
        }

        env->DeleteLocalRef(player);
    }

private:
    bool jumping_ = false;
};
