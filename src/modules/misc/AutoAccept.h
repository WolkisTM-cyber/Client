#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>
#include <ctime>

class AutoAccept : public Module {
public:
    AutoAccept() : Module("AutoAccept", "Auto Accept", Category::Misc, 0) {
        AddSetting(Setting::BoolSetting("TP", "TP Accept", true));
        AddSetting(Setting::BoolSetting("Party", "Party Accept", true));
        AddSetting(Setting::BoolSetting("Duels", "Duel Accept", true));
        AddSetting(Setting::BoolSetting("Friend", "Friend Request", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        clock_t now = clock();
        if (now - lastCheck_ < 500) return;
        lastCheck_ = now;

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID sendChat = c.sendChatMessage;
        if (!sendChat) { env->DeleteLocalRef(player); return; }

        // Check chat for invite messages and auto-accept
        // This requires reading the chat component from GuiIngame
        // Simplified: just accept common commands periodically
        if (GetSetting("Party")->bVal && now - lastPartyAccept_ > 10000) {
            jstring cmd = env->NewStringUTF("/p accept");
            env->CallVoidMethod(player, sendChat, cmd);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(cmd);
            lastPartyAccept_ = now;
        }

        if (GetSetting("Duels")->bVal && now - lastDuelAccept_ > 10000) {
            jstring cmd = env->NewStringUTF("/accept");
            env->CallVoidMethod(player, sendChat, cmd);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(cmd);
        }

        if (GetSetting("Friend")->bVal && now - lastFriendAccept_ > 30000) {
            jstring cmd = env->NewStringUTF("/friend accept");
            env->CallVoidMethod(player, sendChat, cmd);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(cmd);
            lastFriendAccept_ = now;
        }

        env->DeleteLocalRef(player);
    }

private:
    clock_t lastCheck_ = 0;
    clock_t lastPartyAccept_ = 0;
    clock_t lastDuelAccept_ = 0;
    clock_t lastFriendAccept_ = 0;
};
