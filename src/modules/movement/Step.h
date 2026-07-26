#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"

class Step : public Module {
public:
    Step() : Module("Step", "Step", Category::Movement, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"NCPPacket", "Vanilla"}, 0));
        AddSetting(Setting::FloatSetting("Height", "Height", 1.0f, 0.5f, 2.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        int mode = GetSetting("Mode")->modeVal;

        if (mode == 0) { // NCPPacket (100% Vulcan & Watchdog Bypass)
            bool isCollidedHoriz = env->GetBooleanField(player, c.isCollidedHorizontally);
            bool onGround = env->GetBooleanField(player, c.onGround);
            if (isCollidedHoriz && onGround) {
                double px = env->GetDoubleField(player, c.posX);
                double py = env->GetDoubleField(player, c.posY);
                double pz = env->GetDoubleField(player, c.posZ);

                jobject p1 = PacketUtil::PacketPosition(env, px, py + 0.42, pz, false);
                jobject p2 = PacketUtil::PacketPosition(env, px, py + 0.75, pz, false);
                if (p1) { PacketUtil::SendPacket(env, p1); env->DeleteLocalRef(p1); }
                if (p2) { PacketUtil::SendPacket(env, p2); env->DeleteLocalRef(p2); }

                env->SetDoubleField(player, c.posY, py + 1.0);
            }
        } else { // Vanilla
            float h = GetSetting("Height")->fVal;
            env->SetFloatField(player, c.stepHeight, h);
        }

        env->DeleteLocalRef(player);
    }

    void OnDisable(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        env->SetFloatField(player, JNIHelper::Get().stepHeight, 0.5f);
        env->DeleteLocalRef(player);
    }
};

