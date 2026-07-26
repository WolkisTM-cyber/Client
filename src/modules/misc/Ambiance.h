#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Ambiance : public Module {
public:
    Ambiance() : Module("Ambiance", "Ambiance", Category::Visual, 0) {
        AddSetting(Setting::IntSetting("Time", "Time", 18000, 0, 24000));
    }
};
