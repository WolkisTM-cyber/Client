#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class SpeedMine : public Module {
public:
    SpeedMine() : Module("SpeedMine", "Speed Mine", Category::Player, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 1.5f, 1.0f, 5.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jfieldID blockDamage = env->GetFieldID(c.minecraft, "blockDamage", "F");
        if (blockDamage) {
            float cur = env->GetFloatField(mc, blockDamage);
            float speed = GetSetting("Speed")->fVal;
            env->SetFloatField(mc, blockDamage, cur + (speed - 1.0f) * 0.05f);
        }

        // Remove delay between block hits
        jfieldID leftClickCounter = c.leftClickCounter;
        if (leftClickCounter) {
            int counter = env->GetIntField(mc, leftClickCounter);
            if (counter > 0) {
                env->SetIntField(mc, leftClickCounter, 0);
            }
        }

        env->DeleteLocalRef(mc);
    }
};
