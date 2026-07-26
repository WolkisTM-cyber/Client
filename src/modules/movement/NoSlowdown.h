#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"

class NoSlowdown : public Module {
public:
    NoSlowdown() : Module("NoSlowdown", "No Slowdown", Category::Movement, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Hypixel", "GrimAC", "NCP"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID isUsingItem = env->GetMethodID(c.entityLivingBase, "isUsingItem", "()Z");
        if (isUsingItem) {
            jboolean usingItem = env->CallBooleanMethod(player, isUsingItem);
            if (usingItem) {
                int mode = GetSetting("Mode")->modeVal;
                if (mode == 0) { // Hypixel (C07 release before motion tick, C08 re-use after)
                    jobject c07 = PacketUtil::PacketPlayer(env, true);
                    if (c07) { PacketUtil::SendPacket(env, c07); env->DeleteLocalRef(c07); }
                } else if (mode == 1) { // GrimAC
                    jobject c08 = PacketUtil::PacketPlayer(env, false);
                    if (c08) { PacketUtil::SendPacket(env, c08); env->DeleteLocalRef(c08); }
                }
            }
        }

        env->DeleteLocalRef(player);
    }
};

