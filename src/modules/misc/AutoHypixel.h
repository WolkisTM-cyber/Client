#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>

class AutoHypixel : public Module {
public:
    AutoHypixel() : Module("AutoHypixel", "Auto Hypixel", Category::Misc, 0) {
        AddSetting(Setting::ModeSetting("Game", "Game", {"BedWars", "SkyWars", "Duels", "UHC"}, 0));
        AddSetting(Setting::BoolSetting("AutoPlay", "Auto /play", true));
        AddSetting(Setting::BoolSetting("AutoLobby", "Auto /l", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID sendChat = c.sendChatMessage;
        if (!sendChat) { env->DeleteLocalRef(player); return; }

        // Check if in lobby (no respawn screen)
        jmethodID isDead = env->GetMethodID(c.entityLivingBase, "isEntityAlive", "()Z");
        if (!isDead) { env->DeleteLocalRef(player); return; }

        bool alive = env->CallBooleanMethod(player, isDead);
        if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(player); return; }

        clock_t now = clock();
        if (!alive && GetSetting("AutoLobby")->bVal && now - lastSend_ > 3000) {
            // Player is dead, send /l
            jstring cmd = env->NewStringUTF("/l");
            env->CallVoidMethod(player, sendChat, cmd);
            if (!env->ExceptionCheck()) lastSend_ = now;
            else env->ExceptionClear();
            env->DeleteLocalRef(cmd);
            env->DeleteLocalRef(player);
            return;
        }

        // Auto /play
        if (GetSetting("AutoPlay")->bVal && now - lastSend_ > 2000) {
            const char* cmd = "";
            switch (GetSetting("Game")->modeVal) {
            case 0: cmd = "/play bedwars"; break;
            case 1: cmd = "/play skywars"; break;
            case 2: cmd = "/play duels"; break;
            case 3: cmd = "/play uhc"; break;
            }

            jstring cmdObj = env->NewStringUTF(cmd);
            env->CallVoidMethod(player, sendChat, cmdObj);
            if (!env->ExceptionCheck()) lastSend_ = now;
            else env->ExceptionClear();
            env->DeleteLocalRef(cmdObj);
        }

        env->DeleteLocalRef(player);
    }

private:
    clock_t lastSend_ = 0;
};
