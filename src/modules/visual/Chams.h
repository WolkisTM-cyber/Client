#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Chams : public Module {
public:
    Chams() : Module("Chams", "Chams", Category::Visual, 0) {}
    // Requires OpenGL depth override - placeholder
};
