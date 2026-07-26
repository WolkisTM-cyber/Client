#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Scoreboard : public Module {
public:
    Scoreboard() : Module("Scoreboard", "Scoreboard", Category::Visual, 0) {
        AddSetting(Setting::BoolSetting("Numbers", "Show Numbers", true));
        AddSetting(Setting::BoolSetting("Background", "Show Background", false));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) return;

        auto& c = JNIHelper::Get();
        jmethodID getScoreboard = env->GetMethodID(
            env->GetObjectClass(world), "getScoreboard",
            "()Lnet/minecraft/scoreboard/Scoreboard;");
        if (!getScoreboard) { env->DeleteLocalRef(world); return; }

        jobject sb = env->CallObjectMethod(world, getScoreboard);
        if (!sb || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(world); return; }

        jmethodID getObjInDisplaySlot = env->GetMethodID(
            env->GetObjectClass(sb), "getObjectiveInDisplaySlot",
            "(I)Lnet/minecraft/scoreboard/ScoreObjective;");
        if (getObjInDisplaySlot) {
            jobject obj = env->CallObjectMethod(sb, getObjInDisplaySlot, 1);
            if (obj && !env->ExceptionCheck()) {
                // Scoreboard is present - we can read it
                hasScoreboard_ = true;
                env->DeleteLocalRef(obj);
            } else {
                hasScoreboard_ = false;
                if (env->ExceptionCheck()) env->ExceptionClear();
            }
        }

        env->DeleteLocalRef(sb);
        env->DeleteLocalRef(world);
    }

    bool HasScoreboard() { return hasScoreboard_; }

private:
    bool hasScoreboard_ = false;
};
