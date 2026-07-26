#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class NoCameraClip : public Module {
public:
    NoCameraClip() : Module("NoCameraClip", "No Camera Clip", Category::Visual, 0) {}

    void OnTick(JNIEnv* env) override {
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        jfieldID renderViewEntity = env->GetFieldID(
            JNIHelper::Get().minecraft, "renderViewEntity",
            "Lnet/minecraft/entity/Entity;");
        jclass entityRendererClass = env->FindClass(
            "net/minecraft/client/renderer/EntityRenderer");
        jmethodID orientCamera = env->GetMethodID(
            entityRendererClass, "orientCamera", "(F)V");
        // Camera clipping is handled in EntityRenderer - hook would need detour

        env->DeleteLocalRef(mc);
    }
};
