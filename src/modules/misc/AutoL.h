#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoL : public Module {
public:
    AutoL() : Module("AutoL", "Auto L", Category::Misc, 0) {
        AddSetting(Setting::IntSetting("Delay", "Delay (ticks)", 60, 20, 200));
    }

    void OnTick(JNIEnv* env) override {
        timer_++;
        auto* delay = GetSetting("Delay");
        int d = delay ? delay->iVal : 60;
        if (timer_ < d) return;
        timer_ = 0;

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        jstring msg = env->NewStringUTF("/l");
        env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, msg);
        env->DeleteLocalRef(msg);

        env->DeleteLocalRef(player);
    }

private:
    int timer_ = 0;
};
