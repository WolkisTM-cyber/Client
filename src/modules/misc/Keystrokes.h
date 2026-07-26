#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Keystrokes : public Module {
public:
    Keystrokes() : Module("Keystrokes", "Keystrokes", Category::Visual, 0) {
        AddSetting(Setting::BoolSetting("Mouse", "Show Mouse", true));
        AddSetting(Setting::FloatSetting("Opacity", "Opacity", 0.7f, 0.1f, 1.0f));
    }

    void Render(JNIEnv* env, jobject fr, jmethodID drawStr) {
        auto* mouse = GetSetting("Mouse");
        auto* opacity = GetSetting("Opacity");

        int cx = 100, cy = 50, keysize = 20, gap = 2;
        float op = opacity ? opacity->fVal : 0.7f;

        struct Key {
            int vk; const char* name; int x, y;
        };

        Key keys[] = {
            {0x57, "W", cx, cy - keysize - gap},        // W
            {0x41, "A", cx - keysize - gap, cy},        // A
            {0x53, "S", cx, cy},                         // S
            {0x44, "D", cx + keysize + gap, cy},         // D
        };

        // Space bar
        int spaceY = cy + keysize + gap;
        int spaceW = keysize * 2 + gap;

        for (auto& k : keys) {
            bool pressed = GetAsyncKeyState(k.vk) & 0x8000;
            int color = pressed ? 0x55FF55 : 0xFFFFFF;
            jstring text = env->NewStringUTF(k.name);
            if (text && drawStr) {
                env->CallIntMethod(fr, drawStr, text, k.x + 6, k.y + 4, color);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }
        }

        // Space bar indicator
        bool spacePressed = GetAsyncKeyState(VK_SPACE) & 0x8000;
        if (spacePressed && mouse && mouse->bVal) {
            jstring text = env->NewStringUTF("_");
            if (text && drawStr) {
                env->CallIntMethod(fr, drawStr, text, cx - 4, spaceY, 0x55FF55);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }
        }
    }
};
