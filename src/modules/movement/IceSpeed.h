#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class IceSpeed : public Module {
public:
    IceSpeed() : Module("IceSpeed", "Ice Speed", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Slipperiness", "Slipperiness", 0.4f, 0.1f, 1.0f));
    }
    // Ice slipperiness is a block property - requires Block field modification
    // Placeholder for full implementation
};
