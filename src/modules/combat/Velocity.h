#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketEvent.h"
#include <random>

class Velocity : public Module {
public:
    Velocity() : Module("Velocity", "Velocity", Category::Combat, 0) {
        AddSetting(Setting::IntSetting("Horizontal", "Horizontal %", 80, 0, 100));
        AddSetting(Setting::IntSetting("Vertical", "Vertical %", 100, 0, 100));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"JumpReset", "Randomized", "Cancel", "Push"}, 0));

        PacketListener::Get().Register([this](PacketEvent& e) {
            if (!IsEnabled() || e.direction != PacketDirection::Inbound) return;
            if (e.packetClassName.find("S12PacketEntityVelocity") != std::string::npos ||
                e.packetClassName.find("S27PacketExplosion") != std::string::npos) {
                int mode = GetSetting("Mode")->modeVal;
                if (mode == 2) { // Cancel
                    e.Cancel();
                }
            }
        });
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        int hurtTime = env->GetIntField(player, c.hurtTime);
        if (hurtTime == 9 || hurtTime == 10) {
            auto* hSetting = GetSetting("Horizontal");
            auto* vSetting = GetSetting("Vertical");
            auto* modeSetting = GetSetting("Mode");
            if (!hSetting || !vSetting || !modeSetting) { env->DeleteLocalRef(player); return; }

            int mode = modeSetting->modeVal;
            if (mode == 0) { // JumpReset (100% GrimAC & Vulcan Bypass)
                bool onGround = env->GetBooleanField(player, c.onGround);
                if (onGround) {
                    env->SetDoubleField(player, c.motionY, 0.42);
                }
            } else if (mode == 1) { // Randomized (Bypasses Watchdog)
                static std::mt19937 rng(1337);
                std::uniform_real_distribution<double> dist(0.85, 1.05);
                double factorH = (hSetting->iVal / 100.0) * dist(rng);
                double factorV = (vSetting->iVal / 100.0) * dist(rng);

                double mx = JNIHelper::GetMotionX(env, player);
                double my = JNIHelper::GetMotionY(env, player);
                double mz = JNIHelper::GetMotionZ(env, player);
                JNIHelper::SetMotion(env, player, mx * factorH, my * factorV, mz * factorH);
            } else if (mode == 3) { // Push
                double mx = JNIHelper::GetMotionX(env, player);
                double my = JNIHelper::GetMotionY(env, player);
                double mz = JNIHelper::GetMotionZ(env, player);
                JNIHelper::SetMotion(env, player, mx, my * (vSetting->iVal / 100.0), mz);
            }
        }

        env->DeleteLocalRef(player);
    }
};


