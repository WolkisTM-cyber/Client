#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class GhostHand : public Module {
public:
    GhostHand() : Module("GhostHand", "Ghost Hand", Category::World, 0) {
        AddSetting(Setting::IntSetting("Range", "Range", 10, 5, 50));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        // Increase reach by modifying player controller range
        auto controller = JNIHelper::GetPlayerController(env);
        if (controller) {
            jclass pcClass = env->GetObjectClass(controller);
            jfieldID blockReach = env->GetFieldID(pcClass, "blockReach", "F");
            if (blockReach) {
                float range = (float)GetSetting("Range")->iVal;
                env->SetFloatField(controller, blockReach, range);
            }
            env->DeleteLocalRef(pcClass);
            env->DeleteLocalRef(controller);
        }

        env->DeleteLocalRef(mc);
    }

    void OnDisable(JNIEnv* env) override {
        // Reset reach
        auto controller = JNIHelper::GetPlayerController(env);
        if (controller) {
            jclass pcClass = env->GetObjectClass(controller);
            jfieldID blockReach = env->GetFieldID(pcClass, "blockReach", "F");
            if (blockReach) {
                env->SetFloatField(controller, blockReach, 4.5f);
            }
            env->DeleteLocalRef(pcClass);
            env->DeleteLocalRef(controller);
        }
    }
};
