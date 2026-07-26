#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AntiCactus : public Module {
public:
    AntiCactus() : Module("AntiCactus", "Anti Cactus", Category::Combat, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // Prevent cactus damage by setting noClip
        jclass eClass = env->GetObjectClass(player);
        jfieldID noClipField = env->GetFieldID(eClass, "noClip", "Z");
        if (noClipField) {
            bool noClip = env->GetBooleanField(player, noClipField);
            if (!noClip) {
                double mx = env->GetDoubleField(player, c.motionX);
                double mz = env->GetDoubleField(player, c.motionZ);
                // If near cactus, enable noClip
                if (abs(mx) < 0.01 && abs(mz) < 0.01 && !env->GetBooleanField(player, c.onGround)) {
                    env->SetBooleanField(player, noClipField, true);
                }
            }
        }

        // Also set air friction to max
        jfieldID speedInAir = c.speedInAir;
        if (speedInAir) {
            env->SetFloatField(player, speedInAir, 0.02f);
        }

        env->DeleteLocalRef(eClass);
        env->DeleteLocalRef(player);
    }
};
