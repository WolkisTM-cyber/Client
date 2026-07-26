#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../ModuleManager.h"
#include "config/ConfigManager.h"
#include <shlobj.h>
#include <fstream>
#include <vector>
#include <filesystem>

class Profiles : public Module {
public:
    Profiles() : Module("Profiles", "Profiles", Category::Quality, 0) {
        AddSetting(Setting::ModeSetting("Profile", "Profile", {"Default"}, 0));
    }

    void OnEnable(JNIEnv* env) override {
        RefreshProfiles();
    }

    void RefreshProfiles() {
        profiles_.clear();
        profiles_.push_back("Default");
    }

    std::vector<std::string>& GetProfiles() { return profiles_; }

private:
    std::vector<std::string> profiles_;
};
