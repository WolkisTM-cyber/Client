#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <ctime>
#include <string>

class SessionInfo : public Module {
public:
    SessionInfo() : Module("SessionInfo", "Session Info", Category::Visual, 0) {
        startTime_ = clock();
    }

    void Render(JNIEnv* env, jobject fr, jmethodID drawStr) {
        if (!IsEnabled() || !env || !fr || !drawStr) return;

        clock_t elapsed = (clock() - startTime_) / CLOCKS_PER_SEC;
        int mins = (int)elapsed / 60;
        int secs = (int)elapsed % 60;

        char buf[128];
        snprintf(buf, sizeof(buf), "[Session] Time: %02dm %02ds | Kills: %d | Deaths: %d", mins, secs, kills_, deaths_);

        jstring text = env->NewStringUTF(buf);
        if (text) {
            env->CallIntMethod(fr, drawStr, text, 10, 120, 0x00CEC9);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(text);
        }
    }

    void AddKill() { kills_++; }
    void AddDeath() { deaths_++; }

private:
    clock_t startTime_ = 0;
    int kills_ = 0;
    int deaths_ = 0;
};
