#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class HitColor : public Module {
public:
    HitColor() : Module("HitColor", "Hit Color", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Color", "Damage Color", {"Purple", "Cyan", "Rainbow", "Red"}, 0));
        AddSetting(Setting::FloatSetting("Alpha", "Opacity", 0.6f, 0.1f, 1.0f));
    }
};
