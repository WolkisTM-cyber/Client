#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>
#include <deque>
#include <ctime>

struct Notification {
    std::string text;
    int color;
    clock_t startTime;
    int duration;
};

class Notifications : public Module {
public:
    Notifications() : Module("Notifications", "Notifications", Category::Visual, 0) {
        AddSetting(Setting::IntSetting("Duration", "Duration (ms)", 2000, 500, 5000));
        AddSetting(Setting::ModeSetting("Position", "Position", {"TopRight", "TopLeft", "BottomRight"}, 0));
    }

    static void Add(const std::string& text, int color = 0x55FFFF) {
        Notification n;
        n.text = text;
        n.color = color;
        n.startTime = clock();
        n.duration = 2000;
        Get().notifications_.push_back(n);
        if (Get().notifications_.size() > 10) Get().notifications_.pop_front();
    }

    void Render(JNIEnv* env, jobject fontRenderer, jmethodID drawString) {
        auto* duration = GetSetting("Duration");
        int dur = duration ? duration->iVal : 2000;
        auto* pos = GetSetting("Position");
        int x = 0, y = 0;

        clock_t now = clock();
        int index = 0;

        std::deque<Notification> active;
        for (auto& n : notifications_) {
            if (now - n.startTime < n.duration) {
                active.push_back(n);
            }
        }
        notifications_ = active;

        for (auto& n : notifications_) {
            switch (pos ? pos->modeVal : 0) {
            case 0: x = saved_.viewport[2] - 200; y = 10 + index * 14; break;
            case 1: x = 4; y = 10 + index * 14; break;
            case 2: x = saved_.viewport[2] - 200; y = saved_.viewport[3] - 50 - index * 14; break;
            }

            jstring text = env->NewStringUTF(n.text.c_str());
            if (text && drawString) {
                env->CallIntMethod(fontRenderer, drawString, text, x, y, n.color);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }
            index++;
        }

        if (!notifications_.empty()) {
            notifications_.pop_front();
        }
    }

private:
    static Notifications& Get() {
        static Notifications inst;
        return inst;
    }

    std::deque<Notification> notifications_;
    struct { int viewport[4]; } saved_;
};
