#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"
#include <vector>
#include <queue>

class Blink : public Module {
public:
    Blink() : Module("Blink", "Blink", Category::Player, 0) {}

    void OnEnable(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        posX_ = env->GetDoubleField(player, c.posX);
        posY_ = env->GetDoubleField(player, c.posY);
        posZ_ = env->GetDoubleField(player, c.posZ);

        // Cancel all motion
        env->SetDoubleField(player, c.motionX, 0);
        env->SetDoubleField(player, c.motionY, 0);
        env->SetDoubleField(player, c.motionZ, 0);
        env->DeleteLocalRef(player);
    }

    void OnDisable(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // Teleport back to original position
        env->SetDoubleField(player, c.posX, posX_);
        env->SetDoubleField(player, c.posY, posY_);
        env->SetDoubleField(player, c.posZ, posZ_);
        env->DeleteLocalRef(player);
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // Freeze in place
        env->SetDoubleField(player, c.motionX, 0);
        env->SetDoubleField(player, c.motionY, 0);
        env->SetDoubleField(player, c.motionZ, 0);
        env->SetBooleanField(player, c.onGround, true);

        // Prevent position from updating
        env->SetDoubleField(player, c.posX, posX_);
        env->SetDoubleField(player, c.posY, posY_);
        env->SetDoubleField(player, c.posZ, posZ_);

        env->DeleteLocalRef(player);
    }

private:
    double posX_ = 0, posY_ = 0, posZ_ = 0;
};
