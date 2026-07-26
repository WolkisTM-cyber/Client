#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <queue>
#include <ctime>

class CPSCounter : public Module {
public:
    CPSCounter() : Module("CPSCounter", "CPS Counter", Category::Visual, 0) {
        AddSetting(Setting::BoolSetting("LClick", "Left Click", true));
        AddSetting(Setting::BoolSetting("RClick", "Right Click", true));
    }

    void OnTick(JNIEnv* env) override {
        clock_t now = clock();

        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            leftClicks_.push(now);
        }
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
            rightClicks_.push(now);
        }

        // Remove clicks older than 1 second
        while (!leftClicks_.empty() && now - leftClicks_.front() > CLOCKS_PER_SEC)
            leftClicks_.pop();
        while (!rightClicks_.empty() && now - rightClicks_.front() > CLOCKS_PER_SEC)
            rightClicks_.pop();
    }

    void Render(JNIEnv* env, jobject fr, jmethodID drawStr) {
        char buf[32];
        int lCps = (int)leftClicks_.size();
        int rCps = (int)rightClicks_.size();

        auto* lClick = GetSetting("LClick");
        auto* rClick = GetSetting("RClick");

        int y = 60;
        if (lClick && lClick->bVal) {
            snprintf(buf, sizeof(buf), "LCPS: %d", lCps);
            jstring text = env->NewStringUTF(buf);
            if (text && drawStr) {
                env->CallIntMethod(fr, drawStr, text, 4, y, 0x55FF55);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }
            y += 12;
        }
        if (rClick && rClick->bVal) {
            snprintf(buf, sizeof(buf), "RCPS: %d", rCps);
            jstring text = env->NewStringUTF(buf);
            if (text && drawStr) {
                env->CallIntMethod(fr, drawStr, text, 4, y, 0xFF5555);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }
        }
    }

private:
    std::queue<clock_t> leftClicks_;
    std::queue<clock_t> rightClicks_;
};
