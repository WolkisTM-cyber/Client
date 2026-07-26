#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoBlock : public Module {
public:
    AutoBlock() : Module("AutoBlock", "Auto Block", Category::Combat, 0) {}

    void OnTick(JNIEnv* env) override {
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
                            jmethodID isBlocking = env->GetMethodID(
                                JNIHelper::Get().entityPlayer, "isBlocking", "()Z");
                            if (isBlocking && !env->CallBooleanMethod(player, isBlocking)) {
                                // Simulate right-click via Minecraft
                                jobject mc = JNIHelper::GetMinecraft(env);
                                if (mc) {
                                    jmethodID clickMouse = env->GetMethodID(
                                        JNIHelper::Get().minecraft, "clickMouse",
                                        "()V");
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
