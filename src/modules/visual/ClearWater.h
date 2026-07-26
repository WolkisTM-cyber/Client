#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class ClearWater : public Module {
public:
    ClearWater() : Module("ClearWater", "Clear Water", Category::Visual, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();

        // Set fog density to 0
        jmethodID setWaterFog = env->GetMethodID(
            env->GetObjectClass(mc), "entityRenderer",
            "()Lnet/minecraft/client/renderer/EntityRenderer;");
        if (!setWaterFog) { env->DeleteLocalRef(mc); return; }

        jobject er = env->CallObjectMethod(mc, setWaterFog);
        if (!er || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(mc); return; }

        jclass erClass = env->GetObjectClass(er);
        jfieldID fogColorRed = env->GetFieldID(erClass, "fogColorRed", "F");
        jfieldID fogColorGreen = env->GetFieldID(erClass, "fogColorGreen", "F");
        jfieldID fogColorBlue = env->GetFieldID(erClass, "fogColorBlue", "F");

        if (fogColorRed && fogColorGreen && fogColorBlue) {
            env->SetFloatField(er, fogColorRed, 0.0f);
            env->SetFloatField(er, fogColorGreen, 0.3f);
            env->SetFloatField(er, fogColorBlue, 0.5f);
        }

        env->DeleteLocalRef(er);
        env->DeleteLocalRef(mc);
    }
};
