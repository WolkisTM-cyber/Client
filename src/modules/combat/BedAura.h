#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "RotationUtil.h"
#include "../packet/PacketUtil.h"
#include <cmath>

class BedAura : public Module {
public:
    BedAura() : Module("BedAura", "Bed Aura", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Range", "Range", 4.5f, 2.0f, 6.0f));
        AddSetting(Setting::BoolSetting("SilentRotation", "Silent Rotation", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY);
        double pz = env->GetDoubleField(player, c.posZ);

        float range = GetSetting("Range")->fVal;
        int r = (int)ceil(range);

        for (int x = -r; x <= r; x++) {
            for (int y = -r; y <= r; y++) {
                for (int z = -r; z <= r; z++) {
                    double bx = floor(px) + x;
                    double by = floor(py) + y;
                    double bz = floor(pz) + z;

                    double dist = sqrt((bx - px)*(bx - px) + (by - py)*(by - py) + (bz - pz)*(bz - pz));
                    if (dist > range) continue;

                    // Send dig packet for bed block
                    if (GetSetting("SilentRotation")->bVal) {
                        Rotations rot = RotationUtil::CalculateRotations({px, py + 1.62, pz}, {bx + 0.5, by + 0.5, bz + 0.5});
                        RotationUtil::SetSilentRotation(env, player, rot.yaw, rot.pitch);
                    }

                    jmethodID blockPosCtor = env->GetMethodID(c.blockPos, "<init>", "(DDD)V");
                    if (blockPosCtor) {
                        jobject pos = env->NewObject(c.blockPos, blockPosCtor, bx, by, bz);
                        if (pos) {
                            auto pc = JNIHelper::GetPlayerController(env);
                            if (pc) {
                                jmethodID onPlayerDamageBlock = env->GetMethodID(env->GetObjectClass(pc), "onPlayerDamageBlock", "(Lnet/minecraft/util/BlockPos;Lnet/minecraft/util/EnumFacing;)Z");
                                jclass enumClass = env->FindClass("net/minecraft/util/EnumFacing");
                                if (enumClass) {
                                    jfieldID upField = env->GetStaticFieldID(enumClass, "UP", "Lnet/minecraft/util/EnumFacing;");
                                    if (upField) {
                                        jobject up = env->GetStaticObjectField(enumClass, upField);
                                        if (up && onPlayerDamageBlock) {
                                            env->CallBooleanMethod(pc, onPlayerDamageBlock, pos, up);
                                            env->DeleteLocalRef(up);
                                        }
                                    }
                                    env->DeleteLocalRef(enumClass);
                                }
                                env->DeleteLocalRef(pc);
                            }
                            env->DeleteLocalRef(pos);
                        }
                    }
                    env->DeleteLocalRef(world);
                    env->DeleteLocalRef(player);
                    return;
                }
            }
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }
};

