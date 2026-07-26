#include "CommandManager.h"
#include "../modules/ModuleManager.h"
#include "../modules/JNIHelper.h"
#include "../modules/misc/Friends.h"
#include "../modules/world/Waypoints.h"
#include "../modules/quality/Profiles.h"
#include "../config/ConfigManager.h"
#include "../Obfuscation.h"
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
    Register(XOR_STR("help"), XOR_STR("Show all commands"), XOR_STR(".help"), [](JNIEnv* env, const std::vector<std::string>& args) {
        SendChat(env, XOR_STR("§6Client Commands:"));
        SendChat(env, XOR_STR("§e.toggle <name> §7- Toggle module"));
        SendChat(env, XOR_STR("§e.friend add/remove/list <name> §7- Manage friends"));
        SendChat(env, XOR_STR("§e.waypoint set/remove/list <name> §7- Manage waypoints"));
        SendChat(env, XOR_STR("§e.profile load/save/list <name> §7- Manage profiles"));
        SendChat(env, XOR_STR("§e.bind <module> <key> §7- Set keybind"));
        SendChat(env, XOR_STR("§e.set <module> <setting> <value> §7- Change setting"));
        SendChat(env, XOR_STR("§e.panic §7- Disable all modules"));
    });

    Register(XOR_STR("toggle"), XOR_STR("Toggle a module"), XOR_STR(".toggle <name>"), [](JNIEnv* env, const std::vector<std::string>& args) {
        if (args.size() < 2) return;
        auto* mod = g_moduleManager->Find(args[1]);
        if (mod) {
            mod->Toggle(env);
            SendChat(env, std::string(XOR_STR("§aToggled ")) + args[1]);
        }
    });

    Register(XOR_STR("panic"), XOR_STR("Disable all modules"), XOR_STR(".panic"), [](JNIEnv* env, const std::vector<std::string>& args) {
        for (auto* mod : g_moduleManager->GetAll()) {
            if (mod->GetName() != XOR_STR("ClickGUI") && mod->IsEnabled()) mod->Toggle(env);
        }
        SendChat(env, XOR_STR("§cPanic! All modules disabled."));
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
    std::string cmdName = args[0].substr(1);

    for (auto& cmd : commands_) {
        if (cmd.name == cmdName) {
            cmd.handler(env, args);
            return true;
        }
    }
    return false;
}


