#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class ItemPhysics3D : public Module {
public:
    ItemPhysics3D() : Module("ItemPhysics3D", "3D Item Physics", Category::Visual, 0) {
        AddSetting(Setting::FloatSetting("RotateSpeed", "Rotation Speed", 1.5f, 0.5f, 5.0f));
    }
};
