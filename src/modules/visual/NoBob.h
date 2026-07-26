#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoBob : public Module {
public:
    NoBob() : Module("NoBob", "No Bob", Category::Visual, 0) {}

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        jfieldID distanceWalkedModified = env->GetFieldID(
            JNIHelper::Get().entityLivingBase, "distanceWalkedModified", "F");
        if (distanceWalkedModified) {
            env->SetFloatField(player, distanceWalkedModified, 0.0f);
        }

        env->DeleteLocalRef(player);
    }
};
