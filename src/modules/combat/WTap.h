#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"
#include <cmath>

class WTap : public Module {
public:
    WTap() : Module("WTap", "W-Tap", Category::Combat, 0) {
        AddSetting(Setting::BoolSetting("KeepSprint", "Keep Sprint", true));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"WTap", "STap", "BlockHit"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // Check if we recently attacked
        if (lastAttack_ > 0) lastAttack_--;

        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            if (lastAttack_ == 0) {
                // Detect entity in crosshair
                jobject mc = JNIHelper::GetMinecraft(env);
                if (!mc) { env->DeleteLocalRef(player); return; }

                jmethodID getObjectMouseOver = env->GetMethodID(
                    c.minecraft, "getObjectMouseOver",
                    "()Lnet/minecraft/util/MovingObjectPosition;");
                if (!getObjectMouseOver) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

                jobject mop = env->CallObjectMethod(mc, getObjectMouseOver);
                if (mop) {
                    jfieldID entityHitField = env->GetFieldID(
                        env->GetObjectClass(mop), "entityHit",
                        "Lnet/minecraft/entity/Entity;");
                    if (entityHitField) {
                        jobject hitEntity = env->GetObjectField(mop, entityHitField);
                        if (hitEntity) {
                            // W-Tap: reset sprint to trigger sprint crit
                            auto* mode = GetSetting("Mode");
                            bool isSprinting = env->CallBooleanMethod(player, c.isSprinting);

                            if (mode && isSprinting) {
                                switch (mode->modeVal) {
                                case 0: // WTap
                                    env->CallVoidMethod(player, c.setSprinting, JNI_FALSE);
                                    break;
                                case 1: // STap - backward movement
                                    env->SetDoubleField(player, c.motionX, 0.0);
                                    env->SetDoubleField(player, c.motionZ, 0.0);
                                    break;
                                case 2: // BlockHit
                                    // Already handled by AutoBlock
                                    break;
                                }
                            }

                            auto* keepSprint = GetSetting("KeepSprint");
                            if (keepSprint && keepSprint->bVal && isSprinting) {
                                // Re-sprint quickly
                                env->CallVoidMethod(player, c.setSprinting, JNI_TRUE);
                            }

                            env->DeleteLocalRef(hitEntity);
                            lastAttack_ = 4; // ticks until next WTap
                        }
                    }
                    env->DeleteLocalRef(mop);
                }
                env->DeleteLocalRef(mc);
            }
        }

        env->DeleteLocalRef(player);
    }

private:
    int lastAttack_ = 0;
};
