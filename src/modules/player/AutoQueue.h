#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../packet/PacketUtil.h"
#include <string>

class AutoQueue : public Module {
public:
    AutoQueue() : Module("AutoQueue", "Auto Queue", Category::Misc, 0) {
        AddSetting(Setting::BoolSetting("Hypixel", "Hypixel Mode", true));
        AddSetting(Setting::IntSetting("Delay", "Delay (ticks)", 60, 20, 200));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        timer_++;
        auto* delay = GetSetting("Delay");
        if (timer_ < (delay ? delay->iVal : 60)) { env->DeleteLocalRef(player); return; }
        timer_ = 0;

        // Check for game end indicators
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) { env->DeleteLocalRef(player); return; }

        jfieldID currentScreen = env->GetFieldID(
            JNIHelper::Get().minecraft, "currentScreen",
            "Lnet/minecraft/client/gui/GuiScreen;");
        if (!currentScreen) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jobject screen = env->GetObjectField(mc, currentScreen);
        env->DeleteLocalRef(mc);

        if (!screen) { env->DeleteLocalRef(player); return; }

        // Check for play-again button
        jclass guiGameOver = env->FindClass(
            "net/minecraft/client/gui/GuiGameOver");
        if (guiGameOver && env->IsInstanceOf(screen, guiGameOver)) {
            // Automatically click respawn / play again
            jmethodID actionPerformed = env->GetMethodID(
                env->GetObjectClass(screen), "actionPerformed",
                "(Lnet/minecraft/client/gui/GuiButton;)V");

            if (actionPerformed) {
                // Find "Play Again" button
                jfieldID buttonListField = env->GetFieldID(
                    env->GetObjectClass(screen), "buttonList",
                    "Ljava/util/List;");
                if (buttonListField) {
                    jobject buttonList = env->GetObjectField(screen, buttonListField);
                    if (buttonList) {
                        jint bSize = env->CallIntMethod(buttonList, JNIHelper::Get().listSize);
                        for (int i = 0; i < bSize; i++) {
                            jobject btn = env->CallObjectMethod(buttonList,
                                JNIHelper::Get().listGet, i);
                            if (btn) {
                                jmethodID getDisplay = env->GetMethodID(
                                    env->GetObjectClass(btn), "getDisplayString",
                                    "()Ljava/lang/String;");
                                jstring display = (jstring)env->CallObjectMethod(btn, getDisplay);
                                if (display) {
                                    const char* str = env->GetStringUTFChars(display, nullptr);
                                    if (str) {
                                        std::string s(str);
                                        if (s.find("Play Again") != std::string::npos ||
                                            s.find("play again") != std::string::npos) {
                                            env->CallVoidMethod(screen, actionPerformed, btn);
                                        }
                                        env->ReleaseStringUTFChars(display, str);
                                    }
                                    env->DeleteLocalRef(display);
                                }
                                env->DeleteLocalRef(btn);
                            }
                        }
                        env->DeleteLocalRef(buttonList);
                    }
                }
            }
        }

        env->DeleteLocalRef(guiGameOver);
        env->DeleteLocalRef(screen);
        env->DeleteLocalRef(player);
    }

private:
    int timer_ = 0;
};
