#include "CommandManager.h"
#include "../modules/ModuleManager.h"
#include "../modules/JNIHelper.h"
#include <sstream>
#include <algorithm>

extern ModuleManager* g_moduleManager;

CommandManager::CommandManager() {
    AddCommand(Command("help", "List commands", [](JNIEnv* env, const std::vector<std::string>& args) {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return false;
        jstring msg = env->NewStringUTF("Client: .toggle <module> .set <module> <setting> <value> .bind <module> <key>");
        env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, msg);
        env->DeleteLocalRef(msg);
        env->DeleteLocalRef(player);
        return true;
    }));

    AddCommand(Command("toggle", "Toggle a module", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return false;
        auto* mod = g_moduleManager->Find(args[1]);
        if (!mod) {
            auto player = JNIHelper::GetPlayer(env);
            if (player) {
                jstring msg = env->NewStringUTF(("Module not found: " + args[1]).c_str());
                env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, msg);
                env->DeleteLocalRef(msg);
                env->DeleteLocalRef(player);
            }
            return false;
        }
        mod->Toggle(env);
        auto player = JNIHelper::GetPlayer(env);
        if (player) {
            std::string msg = "Toggled " + mod->GetName() + ": " + (mod->IsEnabled() ? "ON" : "OFF");
            jstring jmsg = env->NewStringUTF(msg.c_str());
            env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, jmsg);
            env->DeleteLocalRef(jmsg);
            env->DeleteLocalRef(player);
        }
        return true;
    }));

    AddCommand(Command("set", "Set a module setting", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 4) return false;
        auto* mod = g_moduleManager->Find(args[1]);
        if (!mod) return false;
        auto* setting = mod->GetSetting(args[2]);
        if (!setting) return false;

        switch (setting->type) {
        case Setting::Bool: setting->bVal = args[3] == "true"; break;
        case Setting::Int: setting->iVal = std::stoi(args[3]); break;
        case Setting::Float: setting->fVal = std::stof(args[3]); break;
        case Setting::Mode: setting->modeVal = std::stoi(args[3]); break;
        }

        auto player = JNIHelper::GetPlayer(env);
        if (player) {
            jstring msg = env->NewStringUTF(("Set " + args[1] + "." + args[2] + " to " + args[3]).c_str());
            env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, msg);
            env->DeleteLocalRef(msg);
            env->DeleteLocalRef(player);
        }
        return true;
    }));

    AddCommand(Command("bind", "Set module keybind", [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 3) return false;
        auto* mod = g_moduleManager->Find(args[1]);
        if (!mod) return false;
        int key = std::stoi(args[2]);
        mod->SetKey(key);
        auto player = JNIHelper::GetPlayer(env);
        if (player) {
            jstring msg = env->NewStringUTF(("Bound " + args[1] + " to key " + std::to_string(key)).c_str());
            env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, msg);
            env->DeleteLocalRef(msg);
            env->DeleteLocalRef(player);
        }
        return true;
    }));

    AddCommand(Command("save", "Save config", [](JNIEnv* env, const std::vector<std::string>& args) {
        extern void SaveConfig();
        SaveConfig();
        auto player = JNIHelper::GetPlayer(env);
        if (player) {
            jstring msg = env->NewStringUTF("Config saved!");
            env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, msg);
            env->DeleteLocalRef(msg);
            env->DeleteLocalRef(player);
        }
        return true;
    }));
}

CommandManager::~CommandManager() = default;

bool CommandManager::Execute(JNIEnv* env, const std::string& input) {
    if (input.empty() || input[0] != '.') return false;

    std::string cmd = input.substr(1);
    std::vector<std::string> args;
    std::istringstream iss(cmd);
    std::string token;
    while (iss >> token) args.push_back(token);

    if (args.empty()) return false;

    for (auto& c : commands_) {
        if (c.GetName() == args[0]) {
            return c.Execute(env, args);
        }
    }

    return false;
}

void CommandManager::AddCommand(Command cmd) {
    commands_.push_back(cmd);
}
