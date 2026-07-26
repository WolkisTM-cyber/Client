#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <shlobj.h>
#include <unordered_set>
#include <string>
#include <fstream>
#include <sstream>

class Friends : public Module {
public:
    Friends() : Module("Friends", "Friends", Category::Misc, 0) {
        AddSetting(Setting::BoolSetting("ESP Color", "ESP Color", true));
    }

    static Friends& Get() {
        static Friends inst;
        return inst;
    }

    bool IsFriend(const std::string& name) {
        return friends_.count(name) > 0 || friends_.count(ToLower(name)) > 0;
    }

    void AddFriend(const std::string& name) {
        friends_.insert(name);
        Save();
    }

    void RemoveFriend(const std::string& name) {
        friends_.erase(name);
        friends_.erase(ToLower(name));
        Save();
    }

    bool Load() {
        friends_.clear();

        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData)))
            return false;

        std::string path = std::string(appData, appData + wcslen(appData)) + "\\Client\\friends.txt";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) friends_.insert(line);
        }
        return true;
    }

    bool Save() {
        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData)))
            return false;

        std::string path = std::string(appData, appData + wcslen(appData)) + "\\Client\\friends.txt";
        SHCreateDirectoryExW(nullptr, std::wstring(
            path.begin(), path.end()).c_str(), nullptr);

        std::ofstream file(path);
        if (!file.is_open()) return false;

        for (auto& name : friends_) {
            file << name << std::endl;
        }
        return true;
    }

    static std::string ToLower(const std::string& s) {
        std::string r = s;
        for (char& c : r) c = (char)tolower(c);
        return r;
    }

private:
    std::unordered_set<std::string> friends_;
};
