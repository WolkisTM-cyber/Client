#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>
#include <ctime>

class ServerInfo : public Module {
public:
    ServerInfo() : Module("ServerInfo", "Server Info", Category::Misc, 0) {}

    void Render(JNIEnv* env, jobject fr, jmethodID drawStr) {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        // Get server IP
        jmethodID getCurrentServer = env->GetMethodID(
            c.minecraft, "getCurrentServerData",
            "()Lnet/minecraft/client/multiplayer/ServerData;");
        std::string ip = "Singleplayer";
        std::string brand = "Vanilla";
        int ping = 0;

        if (getCurrentServer) {
            jobject serverData = env->CallObjectMethod(mc, getCurrentServer);
            if (serverData && !env->ExceptionCheck()) {
                jclass sdClass = env->GetObjectClass(serverData);
                jfieldID ipField = env->GetFieldID(sdClass, "serverIP", "Ljava/lang/String;");
                jfieldID pingField = env->GetFieldID(sdClass, "pingToServer", "I");
                if (ipField) {
                    jstring ipObj = (jstring)env->GetObjectField(serverData, ipField);
                    if (ipObj) {
                        const char* ipStr = env->GetStringUTFChars(ipObj, nullptr);
                        if (ipStr) ip = ipStr;
                        env->ReleaseStringUTFChars(ipObj, ipStr);
                        env->DeleteLocalRef(ipObj);
                    }
                }
                if (pingField) ping = env->GetIntField(serverData, pingField);
                env->DeleteLocalRef(sdClass);
                env->DeleteLocalRef(serverData);
            } else if (env->ExceptionCheck()) env->ExceptionClear();
        }

        // Get server brand
        jobject world = env->GetObjectField(mc, c.theWorld);
        if (world) {
            jmethodID getWorldInfo = env->GetMethodID(
                env->GetObjectClass(world), "getWorldInfo",
                "()Lnet/minecraft/world/WorldInfo;");
            if (getWorldInfo) {
                jobject winfo = env->CallObjectMethod(world, getWorldInfo);
                if (winfo) env->DeleteLocalRef(winfo);
            }
            env->DeleteLocalRef(world);
        }

        // Draw server info
        char buf[128];
        int y = 4;
        RECT r; GetClientRect(GetDesktopWindow(), &r);
        int right = r.right - 150;

        snprintf(buf, sizeof(buf), "IP: %s", ip.c_str());
        jstring text = env->NewStringUTF(buf);
        if (text && drawStr) {
            env->CallIntMethod(fr, drawStr, text, right, y, 0x55FFFF);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(text);
        }
        y += 12;

        snprintf(buf, sizeof(buf), "Ping: %dms", ping);
        text = env->NewStringUTF(buf);
        if (text && drawStr) {
            env->CallIntMethod(fr, drawStr, text, right, y, ping < 100 ? 0x55FF55 : 0xFF5555);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(text);
        }

        env->DeleteLocalRef(mc);
    }
};
