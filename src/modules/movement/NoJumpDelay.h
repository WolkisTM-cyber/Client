#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoJumpDelay : public Module {
public:
    NoJumpDelay() : Module("NoJumpDelay", "No Jump Delay", Category::Movement, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();

        // Reset jumpTicks to 0
        jclass eClass = env->GetObjectClass(player);
        jfieldID jumpTicksField = env->GetFieldID(eClass, "jumpTicks", "I");
        if (jumpTicksField) {
            env->SetIntField(player, jumpTicksField, 0);
        }

        // Reset leftClickCounter
        if (c.leftClickCounter) {
            env->SetIntField(mc, c.leftClickCounter, 0);
        }

        env->DeleteLocalRef(eClass);
        env->DeleteLocalRef(mc);
        env->DeleteLocalRef(player);
    }
};
