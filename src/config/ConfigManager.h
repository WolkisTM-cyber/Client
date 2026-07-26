#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../modules/ModuleManager.h"

class ConfigManager {
public:
    static ConfigManager& Get();

    bool Load(ModuleManager* mgr);
    bool Save(ModuleManager* mgr);

    bool ExportToClipboard(ModuleManager* mgr);
    bool ImportFromClipboard(ModuleManager* mgr);

    void SetPath(const std::string& path);

private:
    std::string GetPath();
    std::string path_;
};
