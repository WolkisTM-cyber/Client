#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class InventoryHUD : public Module {
public:
    InventoryHUD() : Module("InventoryHUD", "Inventory HUD", Category::Visual, 0) {}
};
