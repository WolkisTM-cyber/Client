#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <shlobj.h>
#include <algorithm>

ConfigManager& ConfigManager::Get() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::SetPath(const std::string& path) {
    path_ = path;
}

std::string ConfigManager::GetPath() {
    if (!path_.empty()) return path_;

    wchar_t appData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) {
        std::string path = std::string(appData, appData + wcslen(appData)) + "\\Client\\config.txt";
        path_ = path;
    }
    return path_;
}

bool ConfigManager::Load(ModuleManager* mgr) {
    std::string path = GetPath();
    if (path.empty()) return false;

    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    std::string currentModule;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[') {
            currentModule = line.substr(1, line.find(']') - 1);
            continue;
        }

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        auto* mod = mgr->Find(currentModule);
        if (!mod) continue;

        auto* setting = mod->GetSetting(key);
        if (!setting) continue;

        switch (setting->type) {
        case Setting::Bool:
            setting->bVal = val == "true";
            break;
        case Setting::Int:
            setting->iVal = std::stoi(val);
            break;
        case Setting::Float:
            setting->fVal = std::stof(val);
            break;
        case Setting::Mode:
            setting->modeVal = std::stoi(val);
            break;
        }

        // Module enabled state
        if (key == "enabled") {
            setting->bVal = val == "true";
        }
        if (key == "key") {
            mod->SetKey(std::stoi(val));
        }
    }

    return true;
}

bool ConfigManager::Save(ModuleManager* mgr) {
    std::string path = GetPath();
    if (path.empty()) return false;

    // Create directory
    auto dir = path.substr(0, path.find_last_of("\\/"));
    SHCreateDirectoryExW(nullptr, std::wstring(dir.begin(), dir.end()).c_str(), nullptr);

    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "# Client Config" << std::endl;
    file << "# Auto-generated, do not edit manually" << std::endl;

    for (auto* mod : mgr->GetAll()) {
        file << "[" << mod->GetName() << "]" << std::endl;
        file << "enabled=" << (mod->IsEnabled() ? "true" : "false") << std::endl;
        file << "key=" << mod->GetKey() << std::endl;
        for (auto& s : mod->GetSettings()) {
            switch (s.type) {
            case Setting::Bool:
                file << s.name << "=" << (s.bVal ? "true" : "false") << std::endl;
                break;
            case Setting::Int:
                file << s.name << "=" << s.iVal << std::endl;
                break;
            case Setting::Float:
                file << s.name << "=" << s.fVal << std::endl;
                break;
            case Setting::Mode:
                file << s.name << "=" << s.modeVal << std::endl;
                break;
            }
        }
        file << std::endl;
    }

    return true;
}
