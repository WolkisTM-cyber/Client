#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../ModuleManager.h"
#include "../config/ConfigManager.h"
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

        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) return;

        std::string dir = std::string(appData, appData + wcslen(appData)) + "\\Client\\profiles\\";
        if (!std::filesystem::exists(dir)) return;

        for (auto& entry : std::filesystem::directory_iterator(dir)) {
            std::string name = entry.path().filename().string();
            size_t ext = name.find_last_of('.');
            if (ext != std::string::npos) name = name.substr(0, ext);
            if (!name.empty()) profiles_.push_back(name);
        }

        // Update setting
        auto* setting = GetSetting("Profile");
        if (setting) {
            setting->modeVals.clear();
            for (auto& p : profiles_) setting->modeVals.push_back(p);
            setting->modeVal = 0;
        }
    }

    void LoadProfile(int index) {
        if (index < 0 || index >= (int)profiles_.size()) return;
        std::string name = profiles_[index];

        auto* config = ConfigManager::GetInstance();
        if (config) {
            config->SetProfile(name);
            config->Load(ConfigManager::GetConfigPath());
        }
    }

    void SaveProfile(int index) {
        if (index < 0 || index >= (int)profiles_.size()) return;
        std::string name = profiles_[index];

        auto* config = ConfigManager::GetInstance();
        if (config) {
            config->SetProfile(name);
            config->Save(ConfigManager::GetConfigPath());
        }
    }

    std::vector<std::string>& GetProfiles() { return profiles_; }

private:
    std::vector<std::string> profiles_;
};
