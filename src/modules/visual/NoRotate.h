#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoRotate : public Module {
public:
    NoRotate() : Module("NoRotate", "No Rotate", Category::Visual, 0) {}
    // Prevents server-side rotation - requires packet interception
};
