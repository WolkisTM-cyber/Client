#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Reach : public Module {
public:
    Reach() : Module("Reach", "Reach", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Range", "Range", 3.5f, 3.0f, 6.0f));
    }
    // Reach modifies entity reach - requires injector hooks for full effect
};
