#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class BedAura : public Module {
public:
    BedAura() : Module("BedAura", "Bed Aura", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Range", "Range", 5.0f, 2.0f, 10.0f));
    }
};
