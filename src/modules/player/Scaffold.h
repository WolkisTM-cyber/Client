#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../combat/RotationUtil.h"

class Scaffold : public Module {
public:
    Scaffold() : Module("Scaffold", "Scaffold", Category::Player, 0) {
        AddSetting(Setting::BoolSetting("SilentRotation", "Silent Rotation", true));
        AddSetting(Setting::BoolSetting("Swing", "Swing", true));
        AddSetting(Setting::IntSetting("Delay", "Placement Delay", 2, 1, 5));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        tick_++;
        int delay = GetSetting("Delay")->iVal;
        if (tick_ < delay) { env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }
        tick_ = 0;

        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY) - 1.0;
        double pz = env->GetDoubleField(player, c.posZ);

        if (GetSetting("SilentRotation")->bVal) {
            float yaw = env->GetFloatField(player, c.rotationYaw);
            RotationUtil::SetSilentRotation(env, player, yaw + 180.0f, 82.5f);
        }

        jmethodID blockPosCtor = env->GetMethodID(c.blockPos, "<init>", "(DDD)V");
        if (blockPosCtor) {
            jobject blockPos = env->NewObject(c.blockPos, blockPosCtor, px, py, pz);
            if (blockPos) {
                auto pc = JNIHelper::GetPlayerController(env);
                if (pc) {
                    jclass enumClass = env->FindClass("net/minecraft/util/EnumFacing");
                    if (enumClass) {
                        jfieldID upField = env->GetStaticFieldID(enumClass, "UP", "Lnet/minecraft/util/EnumFacing;");
                        if (upField) {
                            jobject up = env->GetStaticObjectField(enumClass, upField);
                            if (up) {
                                env->CallBooleanMethod(pc, c.clickBlock, blockPos, up);
                                env->DeleteLocalRef(up);
                            }
                        }
                        env->DeleteLocalRef(enumClass);
                    }
                    env->DeleteLocalRef(pc);
                }
                env->DeleteLocalRef(blockPos);
            }
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }

private:
    int tick_ = 0;
};

