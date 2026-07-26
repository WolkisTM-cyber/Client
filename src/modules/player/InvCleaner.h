#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class InvCleaner : public Module {
public:
    InvCleaner() : Module("InvCleaner", "Inv Cleaner", Category::Player, 0) {
        AddSetting(Setting::BoolSetting("Tools", "Tools", true));
        AddSetting(Setting::BoolSetting("Blocks", "Blocks", true));
        AddSetting(Setting::BoolSetting("Weapons", "Weapons", true));
    }
    // Inventory management - drops junk items
};
