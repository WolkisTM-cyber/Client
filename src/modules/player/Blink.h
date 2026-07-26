#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"
#include "../packet/PacketEvent.h"
#include <vector>

class Blink : public Module {
public:
    Blink() : Module("Blink", "Blink", Category::Player, 0) {
        PacketListener::Get().Register([this](PacketEvent& e) {
            if (!IsEnabled() || e.direction != PacketDirection::Outbound) return;
            if (e.packetClassName.find("C03PacketPlayer") != std::string::npos ||
                e.packetClassName.find("C04PacketPlayerPosition") != std::string::npos ||
                e.packetClassName.find("C05PacketPlayerLook") != std::string::npos ||
                e.packetClassName.find("C06PacketPlayerPosLook") != std::string::npos) {
                if (e.env && e.packet) {
                    packetQueue_.push_back(e.env->NewGlobalRef(e.packet));
                    e.Cancel();
                }
            }
        });
    }

    void OnEnable(JNIEnv* env) override {
        packetQueue_.clear();
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        posX_ = env->GetDoubleField(player, c.posX);
        posY_ = env->GetDoubleField(player, c.posY);
        posZ_ = env->GetDoubleField(player, c.posZ);

        env->SetDoubleField(player, c.motionX, 0);
        env->SetDoubleField(player, c.motionY, 0);
        env->SetDoubleField(player, c.motionZ, 0);
        env->DeleteLocalRef(player);
    }

    void OnDisable(JNIEnv* env) override {
        for (auto& packet : packetQueue_) {
            if (env && packet) {
                PacketUtil::SendPacket(env, packet);
                env->DeleteGlobalRef(packet);
            }
        }
        packetQueue_.clear();
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        env->SetDoubleField(player, c.motionX, 0);
        env->SetDoubleField(player, c.motionY, 0);
        env->SetDoubleField(player, c.motionZ, 0);

        env->DeleteLocalRef(player);
    }

private:
    double posX_ = 0, posY_ = 0, posZ_ = 0;
    std::vector<jobject> packetQueue_;
};

