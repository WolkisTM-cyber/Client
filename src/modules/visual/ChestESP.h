#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class ChestESP : public Module {
public:
    ChestESP() : Module("ChestESP", "Chest ESP", Category::Visual, 0) {}
    // Requires tile entity render hook - placeholder
};
