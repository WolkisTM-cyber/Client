#pragma once
#include "../Module.h"

class HUD : public Module {
public:
    HUD() : Module("HUD", "HUD", Category::Visual, 0) {
        AddSetting(Setting::BoolSetting("Coords", "Show Coords", true));
        AddSetting(Setting::BoolSetting("Modules", "Show Modules", true));
        AddSetting(Setting::BoolSetting("FPS", "Show FPS", false));
    }
};
