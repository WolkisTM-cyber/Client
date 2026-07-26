#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class CustomSky : public Module {
public:
    CustomSky() : Module("CustomSky", "Custom Sky", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Color", "Sky Theme", {"Sunset", "Midnight", "Purple", "Cyan"}, 0));
        AddSetting(Setting::BoolSetting("DisableFog", "Disable Fog", true));
    }
};
