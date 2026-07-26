#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class TimeChanger : public Module {
public:
    TimeChanger() : Module("TimeChanger", "Time Changer", Category::Visual, 0) {
        AddSetting(Setting::IntSetting("Time", "Time (ticks)", 6000, 0, 24000));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jfieldID worldField = c.theWorld;
        if (!worldField) { env->DeleteLocalRef(mc); return; }

        jobject world = env->GetObjectField(mc, worldField);
        if (!world) { env->DeleteLocalRef(mc); return; }

        jmethodID setTime = env->GetMethodID(
            env->GetObjectClass(world), "setWorldTime", "(J)V");
        if (setTime) {
            env->CallVoidMethod(world, setTime, (jlong)GetSetting("Time")->iVal);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(mc);
    }
};
