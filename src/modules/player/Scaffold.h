#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Scaffold : public Module {
public:
    Scaffold() : Module("Scaffold", "Scaffold", Category::Player, 0) {
        AddSetting(Setting::BoolSetting("Swing", "Swing", true));
        AddSetting(Setting::BoolSetting("Expand", "Expand", false));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY) - 1.0;
        double pz = env->GetDoubleField(player, c.posZ);

        jmethodID blockPosCtor = env->GetMethodID(c.blockPos, "<init>", "(DDD)V");
        if (!blockPosCtor) { env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        jobject blockPos = env->NewObject(c.blockPos, blockPosCtor, px, py, pz);
        if (blockPos) {
            auto pc = JNIHelper::GetPlayerController(env);
            if (pc) {
                // Use EnumFacing.UP field instead of getUp()
                jfieldID upField = env->GetStaticFieldID(
                    env->FindClass("net/minecraft/util/EnumFacing"), "UP",
                    "Lnet/minecraft/util/EnumFacing;");
                if (upField) {
                    jobject up = env->GetStaticObjectField(
                        env->FindClass("net/minecraft/util/EnumFacing"), upField);
                    if (up) {
                        env->CallBooleanMethod(pc, c.clickBlock, blockPos, up);
                        env->DeleteLocalRef(up);
                    }
                }
                env->DeleteLocalRef(pc);
            }
            env->DeleteLocalRef(blockPos);
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }
};
