#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoFire : public Module {
public:
    NoFire() : Module("NoFire", "No Fire", Category::Player, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jfieldID fireField = env->GetFieldID(c.entityPlayer, "fire", "I");
        if (fireField) {
            int fireTicks = env->GetIntField(player, fireField);
            if (fireTicks > 0) {
                env->SetIntField(player, fireField, 0);
            }
        }

        // Also extinguish by setting noClip
        jfieldID noClipField = env->GetFieldID(c.entityPlayer, "noClip", "Z");
        if (noClipField) {
            env->SetBooleanField(player, noClipField, false);
        }

        env->DeleteLocalRef(player);
    }
};
