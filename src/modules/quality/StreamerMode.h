#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <windows.h>

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

class StreamerMode : public Module {
public:
    StreamerMode() : Module("StreamerMode", "Streamer Mode", Category::Quality, 0) {
        AddSetting(Setting::BoolSetting("HideIP", "Hide IP", true));
        AddSetting(Setting::BoolSetting("HideName", "Hide Username", true));
        AddSetting(Setting::BoolSetting("HideCoords", "Hide Coords", false));
        AddSetting(Setting::BoolSetting("HideOBS", "Hide From OBS", true));
    }

    void OnEnable(JNIEnv* env) override {
        if (GetSetting("HideOBS")->bVal) {
            HWND hwnd = GetActiveWindow();
            if (!hwnd) hwnd = FindWindowA("LWJGL", nullptr);
            if (hwnd) {
                SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
            }
        }
    }

    void OnDisable(JNIEnv* env) override {
        HWND hwnd = GetActiveWindow();
        if (!hwnd) hwnd = FindWindowA("LWJGL", nullptr);
        if (hwnd) {
            SetWindowDisplayAffinity(hwnd, 0x00000000);
        }
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jobject session = env->GetObjectField(mc, env->GetFieldID(
            c.minecraft, "session", "Lnet/minecraft/util/Session;"));
        if (!session) { env->DeleteLocalRef(mc); return; }

        if (GetSetting("HideName")->bVal) {
            jfieldID usernameField = env->GetFieldID(
                env->GetObjectClass(session), "username",
                "Ljava/lang/String;");
            if (usernameField) {
                jstring hidden = env->NewStringUTF("Player");
                env->SetObjectField(session, usernameField, hidden);
                env->DeleteLocalRef(hidden);
            }
        }

        env->DeleteLocalRef(session);
        env->DeleteLocalRef(mc);
    }

    bool ShouldHideCoord() { return IsEnabled() && GetSetting("HideCoords")->bVal; }
    bool ShouldHideIP() { return IsEnabled() && GetSetting("HideIP")->bVal; }
};

