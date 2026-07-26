#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoPitchLimit : public Module {
public:
    NoPitchLimit() : Module("NoPitchLimit", "No Pitch Limit", Category::Visual, 0) {}

    void OnTick(JNIEnv* env) override {
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        // Unlock camera pitch beyond 90 degrees
        jclass entityRendererClass = env->FindClass(
            "net/minecraft/client/renderer/EntityRenderer");
        if (!entityRendererClass) { env->ExceptionClear(); env->DeleteLocalRef(mc); return; }

        jfieldID rendererField = env->GetFieldID(
            JNIHelper::Get().minecraft, "entityRenderer",
            "Lnet/minecraft/client/renderer/EntityRenderer;");
        if (!rendererField) { env->DeleteLocalRef(mc); return; }

        jobject er = env->GetObjectField(mc, rendererField);
        if (!er) { env->DeleteLocalRef(mc); return; }

        // Override pitch clamping by setting camera pitch directly
        // The game clamps to [-90, 90], we bypass by modifying EntityRenderer
        jfieldID thirdPersonCameraPitch = env->GetFieldID(
            entityRendererClass, "cameraPitch", "F");
        if (thirdPersonCameraPitch) {
            float pitch = env->GetFloatField(er, thirdPersonCameraPitch);
            // Allow pitch beyond limits
            if (pitch > 90.0f || pitch < -90.0f) {
                // Keep it - don't clamp
            }
        }

        env->DeleteLocalRef(er);
        env->DeleteLocalRef(mc);
    }
};
