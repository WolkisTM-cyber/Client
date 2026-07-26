#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../misc/Notifications.h"
#include <string>
#include <ctime>

class BanNotifier : public Module {
public:
    BanNotifier() : Module("BanNotifier", "Ban Notifier", Category::Misc, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        clock_t now = clock();
        if (now - lastCheck_ < 1000) return;
        lastCheck_ = now;

        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        // Check if disconnected with ban message
        jfieldID worldField = c.theWorld;
        if (!worldField) { env->DeleteLocalRef(mc); return; }

        jobject world = env->GetObjectField(mc, worldField);
        if (!world) {
            // Not in a world - check for ban screen
            jobject screen = env->GetObjectField(mc, env->GetFieldID(
                c.minecraft, "currentScreen", "Lnet/minecraft/client/gui/GuiScreen;"));
            if (screen) {
                jclass screenClass = env->GetObjectClass(screen);
                jmethodID getText = env->GetMethodID(screenClass, "getText",
                    "()Ljava/lang/String;");
                if (getText) {
                    jstring text = (jstring)env->CallObjectMethod(screen, getText);
                    if (text) {
                        const char* str = env->GetStringUTFChars(text, nullptr);
                        if (str && (strstr(str, "ban") || strstr(str, "Ban") || strstr(str, "kicked"))) {
                            Notifications::Add(std::string("Ban detected: ") + str, 0xFF0000);
                        }
                        env->ReleaseStringUTFChars(text, str);
                        env->DeleteLocalRef(text);
                    }
                }
                env->DeleteLocalRef(screenClass);
                env->DeleteLocalRef(screen);
            }
        } else {
            env->DeleteLocalRef(world);
        }

        env->DeleteLocalRef(mc);
    }

private:
    clock_t lastCheck_ = 0;
};
