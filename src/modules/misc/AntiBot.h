#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AntiBot : public Module {
public:
    AntiBot() : Module("AntiBot", "Anti Bot", Category::Misc, 0) {
        AddSetting(Setting::BoolSetting("Invisible", "Hide Invisible", true));
        AddSetting(Setting::BoolSetting("Ground", "Ground Check", false));
    }
};
