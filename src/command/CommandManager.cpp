#include "CommandManager.h"
#include "../modules/ModuleManager.h"
#include "../modules/JNIHelper.h"
#include "../modules/misc/Friends.h"
#include "../modules/world/Waypoints.h"
#include "../modules/quality/Profiles.h"
#include "../config/ConfigManager.h"
#include <sstream>
#include <algorithm>

extern ModuleManager* g_moduleManager;

static void SendChat(JNIEnv* env, const std::string& text) {
    auto player = JNIHelper::GetPlayer(env);
    if (!player) return;
    auto& c = JNIHelper::Get();
    if (c.sendChatMessage) {
        jstring msg = env->NewStringUTF(text.c_str());
        if (msg) {
            env->CallVoidMethod(player, c.sendChatMessage, msg);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(msg);
        }
    }
    env->DeleteLocalRef(player);
}

CommandManager::CommandManager() {
    RegisterDefaults();
}

void CommandManager::RegisterDefaults() {
    Register("help", "Show all commands", ".help", [](JNIEnv* env, const std::vector<std::string>& args) {
        SendChat(env, "§6Client Commands:");
        SendChat(env, "§e.toggle <name> §7- Toggle module");
        SendChat(env, "§e.friend add/remove/list <name> §7- Manage friends");
        SendChat(env, "§e.waypoint set/remove/list <name> §7- Manage waypoints");
        SendChat(env, "§e.profile load/save/list <name> §7- Manage profiles");
        SendChat(env, "§e.bind <module> <key> §7- Set keybind");
        SendChat(env, "§e.set <module> <setting> <value> §7- Change setting");
        SendChat(env, "§e.nuker <radius> §7- Set nuker radius");
        SendChat(env, "§e.toggleall on/off §7- Toggle all modules");
        SendChat(env, "§e.config load/save §7- Load/save config");
        SendChat(env, "§e.panic §7- Disable all modules");
    });

    Register("toggle", "Toggle a module", ".toggle <name>", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return;
        auto* mod = g_moduleManager->Find(args[1]);
        if (mod) {
            mod->Toggle(env);
            SendChat(env, std::string("§aToggled ") + args[1]);
        }
    });

    Register("friend", "Manage friends", ".friend add/remove/list <name>", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return;
        auto& friends = Friends::Get();

        if (args[1] == "add" && args.size() >= 3) {
            friends.AddFriend(args[2]);
            SendChat(env, std::string("§aAdded friend: ") + args[2]);
        } else if (args[1] == "remove" && args.size() >= 3) {
            friends.RemoveFriend(args[2]);
            SendChat(env, std::string("§cRemoved friend: ") + args[2]);
        } else if (args[1] == "list") {
            SendChat(env, "§6Friends list");
        }
    });

    Register("waypoint", "Manage waypoints", ".waypoint set/remove/list <name>", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return;
        auto* wpMod = g_moduleManager->Find("Waypoints");
        if (!wpMod) return;
        auto* waypoints = (Waypoints*)wpMod;

        if (args[1] == "set" && args.size() >= 3) {
            auto player = JNIHelper::GetPlayer(env);
            if (player) {
                auto& c = JNIHelper::Get();
                double x = env->GetDoubleField(player, c.posX);
                double y = env->GetDoubleField(player, c.posY);
                double z = env->GetDoubleField(player, c.posZ);
                waypoints->AddWaypoint(args[2], x, y, z);
                env->DeleteLocalRef(player);
                SendChat(env, std::string("§aWaypoint set: ") + args[2]);
            }
        } else if (args[1] == "remove" && args.size() >= 3) {
            waypoints->RemoveWaypoint(args[2]);
            SendChat(env, std::string("§cRemoved waypoint: ") + args[2]);
        } else if (args[1] == "list") {
            SendChat(env, "§6Waypoints:");
            for (auto& wp : waypoints->GetWaypoints()) {
                SendChat(env, std::string(" §e") + wp.name + " §7at " +
                         std::to_string((int)wp.x) + " " +
                         std::to_string((int)wp.y) + " " +
                         std::to_string((int)wp.z));
            }
        }
    });

    Register("profile", "Manage profiles", ".profile load/save/list <name>", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return;
        auto* profMod = g_moduleManager->Find("Profiles");
        if (!profMod) return;
        auto* profiles = (Profiles*)profMod;

        if (args[1] == "load" && args.size() >= 3) {
            auto& plist = profiles->GetProfiles();
            for (size_t i = 0; i < plist.size(); i++) {
                if (plist[i] == args[2]) {
                    profiles->LoadProfile((int)i);
                    SendChat(env, std::string("§aLoaded profile: ") + args[2]);
                    return;
                }
            }
            SendChat(env, "§cProfile not found!");
        } else if (args[1] == "save" && args.size() >= 3) {
            auto& plist = profiles->GetProfiles();
            for (size_t i = 0; i < plist.size(); i++) {
                if (plist[i] == args[2]) {
                    profiles->SaveProfile((int)i);
                    SendChat(env, std::string("§aSaved profile: ") + args[2]);
                    return;
                }
            }
            SendChat(env, "§cProfile not found!");
        } else if (args[1] == "list") {
            SendChat(env, "§6Profiles:");
            for (auto& p : profiles->GetProfiles()) {
                SendChat(env, std::string(" §e") + p);
            }
        }
    });

    Register("nuker", "Set nuker radius", ".nuker <radius>", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return;
        auto* nuker = g_moduleManager->Find("Nuker");
        if (!nuker) return;
        auto* setting = nuker->GetSetting("Radius");
        if (setting) {
            int radius = std::stoi(args[1]);
            if (radius >= 1 && radius <= 10) {
                setting->iVal = radius;
                SendChat(env, std::string("§aNuker radius set to ") + args[1]);
            }
        }
    });

    Register("toggleall", "Toggle all modules", ".toggleall on/off", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return;
        bool enable = args[1] == "on";

        for (auto* mod : g_moduleManager->GetAll()) {
            if (enable && !mod->IsEnabled()) mod->Toggle(env);
            else if (!enable && mod->IsEnabled()) mod->Toggle(env);
        }
        SendChat(env, enable ? "§aEnabled all modules" : "§cDisabled all modules");
    });

    Register("panic", "Disable all modules", ".panic", [](JNIEnv* env, const std::vector<std::string>& args) {
        for (auto* mod : g_moduleManager->GetAll()) {
            if (mod->GetName() != "ClickGUI" && mod->IsEnabled()) mod->Toggle(env);
        }
        SendChat(env, "§cPanic! All modules disabled.");
    });

    Register("config", "Load/save config", ".config load/save", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return;
        if (args[1] == "save") {
            ConfigManager::Get().Save(g_moduleManager);
            SendChat(env, "§aConfig saved!");
        } else if (args[1] == "load") {
            ConfigManager::Get().Load(g_moduleManager);
            SendChat(env, "§aConfig loaded!");
        }
    });

    Register("bind", "Set keybind", ".bind <module> <key>", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 3) return;
        auto* mod = g_moduleManager->Find(args[1]);
        if (!mod) return;

        int key = 0;
        if (args[2] == "lcontrol" || args[2] == "lctrl") key = VK_LCONTROL;
        else if (args[2] == "rcontrol" || args[2] == "rctrl") key = VK_RCONTROL;
        else if (args[2] == "lshift") key = VK_LSHIFT;
        else if (args[2] == "rshift") key = VK_RSHIFT;
        else if (args[2] == "tab") key = VK_TAB;
        else if (args[2] == "none") key = 0;
        else if (args[2].length() == 1) key = toupper(args[2][0]);
        else key = std::stoi(args[2]);

        mod->SetKey(key);
        SendChat(env, std::string("§aBound ") + args[1] + " to key " + args[2]);
    });

    Register("set", "Change module setting", ".set <module> <setting> <value>", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 4) return;
        auto* mod = g_moduleManager->Find(args[1]);
        if (!mod) return;
        auto* setting = mod->GetSetting(args[2]);
        if (!setting) return;

        switch (setting->type) {
        case SettingType::BOOLEAN:
            setting->bVal = args[3] == "true" || args[3] == "1";
            break;
        case SettingType::INTEGER:
            setting->iVal = std::stoi(args[3]);
            break;
        case SettingType::FLOAT:
            setting->fVal = std::stof(args[3]);
            break;
        case SettingType::MODE:
            setting->modeVal = std::stoi(args[3]);
            break;
        }

        SendChat(env, std::string("§aSet ") + args[1] + " " + args[2] + " = " + args[3]);
    });
}

void CommandManager::Register(const std::string& name, const std::string& desc,
                               const std::string& usage,
                               std::function<void(JNIEnv*, const std::vector<std::string>&)> handler) {
    commands_.push_back({name, desc, usage, handler});
}

bool CommandManager::Execute(JNIEnv* env, const std::string& input) {
    if (input.empty() || input[0] != '.') return false;

    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) args.push_back(token);

    if (args.empty()) return false;
    std::string cmdName = args[0].substr(1); // Remove '.'

    for (auto& cmd : commands_) {
        if (cmd.name == cmdName) {
            cmd.handler(env, args);
            return true;
        }
    }
    return false;
}

