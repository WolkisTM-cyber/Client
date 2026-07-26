#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Sneak : public Module {
public:
    Sneak() : Module("Sneak", "Sneak", Category::Player, 0) {
        AddSetting(Setting::BoolSetting("Toggle", "Toggle", false));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        env->CallVoidMethod(player, JNIHelper::Get().setSneaking, JNI_TRUE);
        wasSneaking_ = true;

        env->DeleteLocalRef(player);
    }

    void OnDisable(JNIEnv* env) override {
        if (wasSneaking_) {
            auto player = JNIHelper::GetPlayer(env);
            if (player) {
                env->CallVoidMethod(player, JNIHelper::Get().setSneaking, JNI_FALSE);
                env->DeleteLocalRef(player);
            }
            wasSneaking_ = false;
        }
    }

private:
    bool wasSneaking_ = false;
};
