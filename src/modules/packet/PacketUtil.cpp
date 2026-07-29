#include "PacketUtil.h"
#include <cstdarg>
#include <cstring>

jclass PacketUtil::GetPacketClass(JNIEnv* env, const char* name) {
    std::string full = "net/minecraft/network/play/client/";
    full += name;
    jclass clazz = env->FindClass(full.c_str());
    if (!clazz) {
        env->ExceptionClear();
        return nullptr;
    }
    return clazz;
}

jmethodID PacketUtil::GetSendMethod(JNIEnv* env) {
    auto& c = JNIHelper::Get();
    jclass nc = env->FindClass("net/minecraft/client/network/NetHandlerPlayClient");
    if (!nc) { env->ExceptionClear(); return nullptr; }
    jmethodID send = env->GetMethodID(nc,
        "sendPacket", "(Lnet/minecraft/network/Packet;)V");
    if (!send) {
        // 1.8.x: addToSendQueue
        send = env->GetMethodID(nc,
            "addToSendQueue", "(Lnet/minecraft/network/Packet;)V");
    }
    env->DeleteLocalRef(nc);
    if (!send) env->ExceptionClear();
    return send;
}

jobject PacketUtil::GetNetHandler(JNIEnv* env) {
    auto mc = JNIHelper::GetMinecraft(env);
    if (!mc) return nullptr;

    jfieldID netHandlerField = env->GetFieldID(
        JNIHelper::Get().minecraft, "getNetHandler",
        "Lnet/minecraft/client/network/NetHandlerPlayClient;");
    if (!netHandlerField) {
        netHandlerField = env->GetFieldID(
            JNIHelper::Get().minecraft, "thePlayer",
            "Lnet/minecraft/client/entity/EntityPlayerSP;");
        if (netHandlerField) {
            jobject player = env->GetObjectField(mc, netHandlerField);
            if (player) {
                jmethodID gnh = env->GetMethodID(
                    JNIHelper::Get().entityPlayer, "sendQueue",
                    "Lnet/minecraft/client/network/NetHandlerPlayClient;");
                if (!gnh) {
                    gnh = env->GetMethodID(
                        JNIHelper::Get().entityPlayer, "getNetHandler",
                        "()Lnet/minecraft/client/network/NetHandlerPlayClient;");
                }
                env->DeleteLocalRef(mc);
                if (!gnh) { env->DeleteLocalRef(player); return nullptr; }
                jobject nh = env->CallObjectMethod(player, gnh);
                env->DeleteLocalRef(player);
                return nh;
            }
        }
        env->DeleteLocalRef(mc);
        return nullptr;
    }

    jobject nh = env->GetObjectField(mc, netHandlerField);
    env->DeleteLocalRef(mc);
    return nh;
}

bool PacketUtil::SendPacket(JNIEnv* env, jobject packet) {
    if (!packet) return false;
    jobject nh = GetNetHandler(env);
    if (!nh) return false;

    jmethodID send = GetSendMethod(env);
    if (!send) { env->DeleteLocalRef(nh); return false; }

    env->CallVoidMethod(nh, send, packet);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(nh); return false; }

    env->DeleteLocalRef(nh);
    return true;
}

jobject PacketUtil::CreatePacket(JNIEnv* env, const char* className, const char* sig, ...) {
    jclass clazz = GetPacketClass(env, className);
    if (!clazz) return nullptr;

    jmethodID ctor = env->GetMethodID(clazz, "<init>", sig);
    if (!ctor) { env->ExceptionClear(); env->DeleteLocalRef(clazz); return nullptr; }

    va_list args;
    va_start(args, sig);
    // Can't directly use va_list with JNI - use explicit overloads instead
    va_end(args);

    env->DeleteLocalRef(clazz);
    return nullptr;
}

jobject PacketUtil::PacketPlayer(JNIEnv* env, bool onGround) {
    jclass clazz = GetPacketClass(env, "C03PacketPlayer");
    if (!clazz) return nullptr;

    jmethodID ctor = env->GetMethodID(clazz, "<init>", "(Z)V");
    if (!ctor) { env->ExceptionClear(); env->DeleteLocalRef(clazz); return nullptr; }

    jobject packet = env->NewObject(clazz, ctor, onGround ? JNI_TRUE : JNI_FALSE);
    env->DeleteLocalRef(clazz);
    return packet;
}

jobject PacketUtil::PacketPosition(JNIEnv* env, double x, double y, double z, bool onGround) {
    jclass clazz = GetPacketClass(env, "C04PacketPlayerPosition");
    if (!clazz) return nullptr;

    jmethodID ctor = env->GetMethodID(clazz, "<init>", "(DDDZ)V");
    if (!ctor) { env->ExceptionClear(); env->DeleteLocalRef(clazz); return nullptr; }

    jobject packet = env->NewObject(clazz, ctor, x, y, z, onGround ? JNI_TRUE : JNI_FALSE);
    env->DeleteLocalRef(clazz);
    return packet;
}

jobject PacketUtil::PacketPositionLook(JNIEnv* env, double x, double y, double z,
                                       float yaw, float pitch, bool onGround) {
    jclass clazz = GetPacketClass(env, "C05PacketPlayerPositionLook");
    if (!clazz) return nullptr;

    jmethodID ctor = env->GetMethodID(clazz, "<init>", "(DDDFFZ)V");
    if (!ctor) { env->ExceptionClear(); env->DeleteLocalRef(clazz); return nullptr; }

    jobject packet = env->NewObject(clazz, ctor, x, y, z, yaw, pitch, onGround ? JNI_TRUE : JNI_FALSE);
    env->DeleteLocalRef(clazz);
    return packet;
}

bool PacketUtil::IsHypixel(JNIEnv* env) {
    auto mc = JNIHelper::GetMinecraft(env);
    if (!mc) return false;

    // Check current server IP
    jmethodID getCurrentServerData = env->GetMethodID(
        JNIHelper::Get().minecraft, "getCurrentServerData",
        "()Lnet/minecraft/client/multiplayer/ServerData;");
    if (!getCurrentServerData) {
        jfieldID currentServerData = env->GetFieldID(
            JNIHelper::Get().minecraft, "currentServerData",
            "Lnet/minecraft/client/multiplayer/ServerData;");
        if (!currentServerData) { env->DeleteLocalRef(mc); return false; }

        jobject sd = env->GetObjectField(mc, currentServerData);
        if (!sd) { env->DeleteLocalRef(mc); return false; }

        jmethodID getIP = env->GetMethodID(
            env->GetObjectClass(sd), "serverIP", "()Ljava/lang/String;");
        if (!getIP) {
            jfieldID ipField = env->GetFieldID(
                env->GetObjectClass(sd), "serverIP", "Ljava/lang/String;");
            if (!ipField) { env->DeleteLocalRef(sd); env->DeleteLocalRef(mc); return false; }
            jstring ip = (jstring)env->GetObjectField(sd, ipField);
            if (!ip) { env->DeleteLocalRef(sd); env->DeleteLocalRef(mc); return false; }
            const char* str = env->GetStringUTFChars(ip, nullptr);
            bool result = str && (strstr(str, "hypixel") != nullptr);
            env->ReleaseStringUTFChars(ip, str);
            env->DeleteLocalRef(ip);
            env->DeleteLocalRef(sd);
            env->DeleteLocalRef(mc);
            return result;
        }

        jstring ip = (jstring)env->CallObjectMethod(sd, getIP);
        if (!ip) { env->DeleteLocalRef(sd); env->DeleteLocalRef(mc); return false; }
        const char* str = env->GetStringUTFChars(ip, nullptr);
        bool result = str && (strstr(str, "hypixel") != nullptr);
        env->ReleaseStringUTFChars(ip, str);
        env->DeleteLocalRef(ip);
        env->DeleteLocalRef(sd);
        env->DeleteLocalRef(mc);
        return result;
    }

    jobject sd = env->CallObjectMethod(mc, getCurrentServerData);
    if (!sd) { env->DeleteLocalRef(mc); return false; }

    jmethodID getIP = env->GetMethodID(
        env->GetObjectClass(sd), "getIP", "()Ljava/lang/String;");
    if (!getIP) { env->DeleteLocalRef(sd); env->DeleteLocalRef(mc); return false; }

    jstring ip = (jstring)env->CallObjectMethod(sd, getIP);
    if (!ip) { env->DeleteLocalRef(sd); env->DeleteLocalRef(mc); return false; }
    const char* str = env->GetStringUTFChars(ip, nullptr);
    bool result = str && (strstr(str, "hypixel") != nullptr);
    env->ReleaseStringUTFChars(ip, str);
    env->DeleteLocalRef(ip);
    env->DeleteLocalRef(sd);
    env->DeleteLocalRef(mc);
    return result;
}
