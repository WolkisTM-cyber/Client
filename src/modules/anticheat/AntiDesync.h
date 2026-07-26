#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AntiDesync : public Module {
public:
    AntiDesync() : Module("AntiDesync", "Anti Desync", Category::Player, 0) {
        AddSetting(Setting::IntSetting("Interval", "Interval (ticks)", 40, 10, 200));
        AddSetting(Setting::BoolSetting("Ground", "Spoof Ground", false));
    }

    void OnTick(JNIEnv* env) override {
        timer_++;
        auto* interval = GetSetting("Interval");
        if (timer_ < (interval ? interval->iVal : 40)) return;
        timer_ = 0;

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // Check position desync
        double posX = env->GetDoubleField(player, c.posX);
        double posY = env->GetDoubleField(player, c.posY);
        double posZ = env->GetDoubleField(player, c.posZ);

        if (prevSet_) {
            double dx = posX - prevX_;
            double dy = posY - prevY_;
            double dz = posZ - prevZ_;
            double dist = dx * dx + dy * dy + dz * dz;

            // If teleported back > 3 blocks, server desynced us
            if (dist > 9.0 && lastTeleport_ > 100) {
                env->SetDoubleField(player, c.posX, prevTeleportX_);
                env->SetDoubleField(player, c.posY, prevTeleportY_);
                env->SetDoubleField(player, c.posZ, prevTeleportZ_);
                lastTeleport_ = 0;
            }

            auto* ground = GetSetting("Ground");
            if (ground && ground->bVal) {
                env->SetBooleanField(player, c.onGround, JNI_TRUE);
            }
        } else {
            prevSet_ = true;
        }

        prevX_ = posX;
        prevY_ = posY;
        prevZ_ = posZ;

        lastTeleport_++;

        env->DeleteLocalRef(player);
    }

private:
    int timer_ = 0;
    bool prevSet_ = false;
    double prevX_ = 0, prevY_ = 0, prevZ_ = 0;
    double prevTeleportX_ = 0, prevTeleportY_ = 0, prevTeleportZ_ = 0;
    int lastTeleport_ = 0;
};
