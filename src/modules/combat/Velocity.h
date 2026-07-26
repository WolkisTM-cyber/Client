#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketEvent.h"

class Velocity : public Module {
public:
    Velocity() : Module("Velocity", "Velocity", Category::Combat, 0) {
        AddSetting(Setting::IntSetting("Horizontal", "Horizontal %", 0, 0, 100));
        AddSetting(Setting::IntSetting("Vertical", "Vertical %", 0, 0, 100));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Cancel", "Reduce", "Push"}, 0));

        PacketListener::Get().Register([this](PacketEvent& e) {
            if (!IsEnabled() || e.direction != PacketDirection::Inbound) return;
            if (e.packetClassName.find("S12PacketEntityVelocity") != std::string::npos ||
                e.packetClassName.find("S27PacketExplosion") != std::string::npos) {
                int mode = GetSetting("Mode")->modeVal;
                if (mode == 0) {
                    e.Cancel();
                }
            }
        });
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        int hurtTime = env->GetIntField(player, JNIHelper::Get().hurtTime);
        if (hurtTime > 0) {
            auto* hSetting = GetSetting("Horizontal");
            auto* vSetting = GetSetting("Vertical");
            auto* modeSetting = GetSetting("Mode");
            if (!hSetting || !vSetting || !modeSetting) { env->DeleteLocalRef(player); return; }

            double mx = JNIHelper::GetMotionX(env, player);
            double my = JNIHelper::GetMotionY(env, player);
            double mz = JNIHelper::GetMotionZ(env, player);

            switch (modeSetting->modeVal) {
            case 0: // Cancel
                JNIHelper::SetMotion(env, player, 0, 0, 0);
                break;
            case 1: // Reduce
                JNIHelper::SetMotion(env, player,
                    mx * hSetting->iVal / 100.0,
                    my * vSetting->iVal / 100.0,
                    mz * hSetting->iVal / 100.0);
                break;
            case 2: // Push (only vertical)
                JNIHelper::SetMotion(env, player, mx, my * vSetting->iVal / 100.0, mz);
                break;
            }
        }

        env->DeleteLocalRef(player);
    }
};

