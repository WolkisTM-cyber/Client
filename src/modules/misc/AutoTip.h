#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <ctime>

class AutoTip : public Module {
public:
    AutoTip() : Module("AutoTip", "Auto Tip", Category::Misc, 0) {
        AddSetting(Setting::IntSetting("Interval", "Interval (s)", 120, 30, 600));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        clock_t now = clock();
        int interval = GetSetting("Interval")->iVal * CLOCKS_PER_SEC;
        if (now - lastTip_ < interval) return;

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID sendChat = c.sendChatMessage;
        if (sendChat) {
            jstring cmd = env->NewStringUTF("/tip all");
            env->CallVoidMethod(player, sendChat, cmd);
            if (!env->ExceptionCheck()) lastTip_ = now;
            else env->ExceptionClear();
            env->DeleteLocalRef(cmd);
        }

        env->DeleteLocalRef(player);
    }

private:
    clock_t lastTip_ = 0;
};
