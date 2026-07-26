#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <fstream>
#include <sstream>

class Theme : public Module {
public:
    Theme() : Module("Theme", "Theme", Category::Quality, 0) {
        AddSetting(Setting::IntSetting("R", "Red", 85, 0, 255));
        AddSetting(Setting::IntSetting("G", "Green", 255, 0, 255));
        AddSetting(Setting::IntSetting("B", "Blue", 255, 0, 255));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Static", "Rainbow", "Wave"}, 0));
        AddSetting(Setting::BoolSetting("Save", "Save on Change", true));
    }

    int GetColor(float offset = 0) {
        auto* mode = GetSetting("Mode");
        if (!mode) return 0x55FFFF;

        switch (mode->modeVal) {
        case 0: { // Static
            int r = GetSetting("R")->iVal;
            int g = GetSetting("G")->iVal;
            int b = GetSetting("B")->iVal;
            return (r << 16) | (g << 8) | b;
        }
        case 1: { // Rainbow
            float hue = (float)(clock() % 2000) / 2000.0f + offset;
            int r = (int)(sin(hue * 6.28318f) * 127 + 128);
            int g = (int)(sin((hue + 0.33f) * 6.28318f) * 127 + 128);
            int b = (int)(sin((hue + 0.66f) * 6.28318f) * 127 + 128);
            return (r << 16) | (g << 8) | b;
        }
        case 2: { // Wave
            float val = (float)(clock() % 1000) / 1000.0f + offset;
            int c = (int)(sin(val * 3.14159f) * 127 + 128);
            return (c << 16) | (c << 8) | c;
        }
        }
        return 0x55FFFF;
    }
};
