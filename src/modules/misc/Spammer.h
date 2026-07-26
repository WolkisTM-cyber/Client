#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>

class Spammer : public Module {
public:
    Spammer() : Module("Spammer", "Spammer", Category::Misc, 0) {
        AddSetting(Setting::IntSetting("Delay", "Delay (ticks)", 100, 10, 1000));
        AddSetting(Setting::BoolSetting("Random", "Randomize", false));
    }

    void OnTick(JNIEnv* env) override {
        tickCount_++;
        auto* delay = GetSetting("Delay");
        int d = delay ? delay->iVal : 100;
        if (tickCount_ < d) return;
        tickCount_ = 0;

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        jstring msg = env->NewStringUTF(message_.c_str());
        env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, msg);
        env->DeleteLocalRef(msg);

        env->DeleteLocalRef(player);
    }

    void SetMessage(const std::string& msg) { message_ = msg; }

private:
    std::string message_ = "/l";
    int tickCount_ = 0;
};
