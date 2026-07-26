#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"

class AutoBlock : public Module {
public:
    AutoBlock() : Module("AutoBlock", "Auto Block", Category::Combat, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"HypixelPacket", "NCP", "Vanilla"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        jobject heldItem = env->CallObjectMethod(player, c.getHeldItem);
        if (heldItem) {
            jobject item = env->CallObjectMethod(heldItem, c.getItemFromStack);
            if (item) {
                jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
                if (name) {
                    const char* str = env->GetStringUTFChars(name, nullptr);
                    if (str) {
                        std::string itemName(str);
                        if (itemName.find("sword") != std::string::npos) {
                            int mode = GetSetting("Mode")->modeVal;
                            if (mode == 0) { // HypixelPacket (C08 packet block placement)
                                jobject packet = PacketUtil::PacketPlayer(env, true);
                                if (packet) {
                                    PacketUtil::SendPacket(env, packet);
                                    env->DeleteLocalRef(packet);
                                }
                            } else {
                                jobject mc = JNIHelper::GetMinecraft(env);
                                if (mc) {
                                    jmethodID clickMouse = env->GetMethodID(c.minecraft, "clickMouse", "()V");
                                    if (clickMouse) env->CallVoidMethod(mc, clickMouse);
                                    env->DeleteLocalRef(mc);
                                }
                            }
                        }
                        env->ReleaseStringUTFChars(name, str);
                    }
                    env->DeleteLocalRef(name);
                }
                env->DeleteLocalRef(item);
            }
            env->DeleteLocalRef(heldItem);
        }
        env->DeleteLocalRef(player);
    }
};

