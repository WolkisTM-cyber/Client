#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoTool : public Module {
public:
    AutoTool() : Module("AutoTool", "Auto Tool", Category::Player, 0) {}
    // Requires block breaking hook - placeholder
};
