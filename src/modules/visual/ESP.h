#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class ESP : public Module {
public:
    ESP() : Module("ESP", "ESP", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Box", "Outline", "2D"}, 0));
    }
    // Requires render hook - placeholder
};
