#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"

class SuperKnockback : public Module {
public:
    SuperKnockback() : Module("SuperKnockback", "Super Knockback", Category::Combat, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Packet", "WTap"}, 0));
    }

    void DoExtraKnockback(JNIEnv* env, jobject player) {
        if (!IsEnabled() || !env || !player) return;
        auto& c = JNIHelper::Get();

        // Send C0B stop & start sprint packet to trigger extra vanilla knockback vector
        jobject stopSprint = PacketUtil::PacketPlayer(env, false);
        jobject startSprint = PacketUtil::PacketPlayer(env, true);
        if (stopSprint) { PacketUtil::SendPacket(env, stopSprint); env->DeleteLocalRef(stopSprint); }
        if (startSprint) { PacketUtil::SendPacket(env, startSprint); env->DeleteLocalRef(startSprint); }
    }
};

