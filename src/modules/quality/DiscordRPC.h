#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

// Discord Rich Presence placeholder
// Requires discord-rpc library (discord_rpc.dll + discord_GameSDK)
// This module provides the hook - linking requires the actual SDK

class DiscordRPC : public Module {
public:
    DiscordRPC() : Module("DiscordRPC", "Discord RPC", Category::Quality, 0) {
        AddSetting(Setting::ModeSetting("Details", "Details", {"Username", "Server", "FPS"}, 0));
        AddSetting(Setting::BoolSetting("ShowServer", "Show Server", true));
    }

    void OnEnable(JNIEnv* env) override {
        initialized_ = InitDiscord();
        UpdatePresence(env);
    }

    void OnDisable(JNIEnv* env) override {
        if (initialized_) {
            typedef void (*ShutdownFunc)();
            auto shutdown = (ShutdownFunc)GetProcAddress(GetModuleHandleA("discord_rpc.dll"), "Discord_Shutdown");
            if (shutdown) shutdown();
            initialized_ = false;
        }
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled() || !initialized_) return;
        tick_++;
        if (tick_ % 100 == 0) { // Update every 5 seconds
            UpdatePresence(env);
        }

        typedef void (*RunFunc)();
        auto run = (RunFunc)GetProcAddress(GetModuleHandleA("discord_rpc.dll"), "Discord_RunCallbacks");
        if (run) run();
    }

private:
    bool InitDiscord() {
        HMODULE discord = LoadLibraryA("discord_rpc.dll");
        if (!discord) return false;

        typedef void (*InitFunc)(const char*, void*, int, void*);
        auto init = (InitFunc)GetProcAddress(discord, "Discord_Initialize");
        if (!init) { FreeLibrary(discord); return false; }

        init("133742069", nullptr, 0, nullptr);
        return true;
    }

    void UpdatePresence(JNIEnv* env) {
        HMODULE discord = GetModuleHandleA("discord_rpc.dll");
        if (!discord) return;

        typedef void (*PresenceFunc)(void*);
        auto presence = (PresenceFunc)GetProcAddress(discord, "Discord_UpdatePresence");
        if (!presence) return;

        // Construct presence struct
        struct DiscordRichPresence {
            const char* state;
            const char* details;
            int64_t startTimestamp;
            int64_t endTimestamp;
            int largeImageKey;
            const char* largeImageText;
            const char* smallImageKey;
            const char* smallImageText;
            const char* partyId;
            int partySize;
            int partyMax;
            const char* matchSecret;
            const char* joinSecret;
            const char* spectateSecret;
            int instance;
        };

        DiscordRichPresence rp = {};
        rp.state = "Playing Minecraft";
        rp.details = "Client -";
        rp.startTimestamp = time(nullptr);
        rp.largeImageKey = 0;
        rp.largeImageText = "Minecraft 1.8.x";
        rp.instance = 0;

        presence(&rp);
    }

    bool initialized_ = false;
    int tick_ = 0;
};
