#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"

class Criticals : public Module {
public:
    Criticals() : Module("Criticals", "Criticals", Category::Combat, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Packet", "Hypixel", "MiniJump"}, 0));
    }

    void DoCritical(JNIEnv* env, jobject player) {
        if (!IsEnabled() || !env || !player) return;
        auto& c = JNIHelper::Get();
        bool onGround = env->GetBooleanField(player, c.onGround);
        if (!onGround) return;

        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY);
        double pz = env->GetDoubleField(player, c.posZ);

        int mode = GetSetting("Mode")->modeVal;
        if (mode == 0) { // Packet
            static const double offsets[] = { 0.0625101, 0.0, 0.0112511, 0.0 };
            for (double offset : offsets) {
                jobject p = PacketUtil::PacketPosition(env, px, py + offset, pz, false);
                if (p) { PacketUtil::SendPacket(env, p); env->DeleteLocalRef(p); }
            }
        } else if (mode == 1) { // Hypixel
            static const double offsets[] = { 0.056, 0.016, 0.003 };
            for (double offset : offsets) {
                jobject p = PacketUtil::PacketPosition(env, px, py + offset, pz, false);
                if (p) { PacketUtil::SendPacket(env, p); env->DeleteLocalRef(p); }
            }
        } else if (mode == 2) { // MiniJump
            env->SetDoubleField(player, c.motionY, 0.1);
        }
    }
};

