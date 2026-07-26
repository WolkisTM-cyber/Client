#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoRespawn : public Module {
public:
    AutoRespawn() : Module("AutoRespawn", "Auto Respawn", Category::Player, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID isDead = env->GetMethodID(c.entityLivingBase, "isEntityAlive", "()Z");
        if (!isDead) { env->DeleteLocalRef(player); return; }

        bool alive = env->CallBooleanMethod(player, isDead);
        if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(player); return; }

        if (!alive) {
            // Send respawn packet
            auto mc = JNIHelper::GetMinecraft(env);
            if (mc) {
                jmethodID clickMouse = env->GetMethodID(
                    env->GetObjectClass(mc), "clickMouse", "()V");
                if (clickMouse) {
                    env->CallVoidMethod(mc, clickMouse);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                }
                env->DeleteLocalRef(mc);
            }
        }

        env->DeleteLocalRef(player);
    }
};
