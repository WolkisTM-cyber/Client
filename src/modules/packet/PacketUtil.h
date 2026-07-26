#pragma once
#include <jni.h>
#include "../JNIHelper.h"
#include <string>

class PacketUtil {
public:
    static jobject GetNetHandler(JNIEnv* env);
    static bool SendPacket(JNIEnv* env, jobject packet);
    static jobject CreatePacket(JNIEnv* env, const char* className, const char* sig, ...);

    // Common packet constructors
    static jobject PacketPlayer(JNIEnv* env, bool onGround);
    static jobject PacketPosition(JNIEnv* env, double x, double y, double z, bool onGround);
    static jobject PacketPositionLook(JNIEnv* env, double x, double y, double z,
                                       float yaw, float pitch, bool onGround);

    static bool IsHypixel(JNIEnv* env);

private:
    static jclass GetPacketClass(JNIEnv* env, const char* name);
    static jmethodID GetSendMethod(JNIEnv* env);
};
