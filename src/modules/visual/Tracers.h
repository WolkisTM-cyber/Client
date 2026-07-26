#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Tracers : public Module {
public:
    Tracers() : Module("Tracers", "Tracers", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Line", "Crosshair"}, 0));
    }
    // Requires world render hook - placeholder
};
