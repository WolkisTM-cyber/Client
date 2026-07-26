#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>

class AutoGG : public Module {
public:
    AutoGG() : Module("AutoGG", "Auto GG", Category::Misc, 0) {
        AddSetting(Setting::ModeSetting("Message", "Message",
            {"gg", "GG", "gg wp", "Good Game!", "gf"}, 0));
        AddSetting(Setting::IntSetting("Delay", "Delay (ticks)", 40, 10, 100));
    }

    void OnTick(JNIEnv* env) override {
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        // Check for game-ending scoreboard / title
        // In Hypixel, "YOU DIED!" or "VICTORY!" title appears
        jclass guiIngame = env->FindClass("net/minecraft/client/gui/GuiIngame");
        if (!guiIngame) { env->ExceptionClear(); env->DeleteLocalRef(mc); return; }

        jfieldID ingameField = env->GetFieldID(
            JNIHelper::Get().minecraft, "ingameGUI",
            "Lnet/minecraft/client/gui/GuiIngame;");
        if (!ingameField) { env->DeleteLocalRef(mc); return; }

        jobject ig = env->GetObjectField(mc, ingameField);
        if (!ig) { env->DeleteLocalRef(mc); return; }

        // Check for title/subtitle
        jfieldID titleField = env->GetFieldID(guiIngame, "title", "Ljava/lang/String;");
        jfieldID subTitleField = env->GetFieldID(guiIngame, "subtitle", "Ljava/lang/String;");

        if (titleField && subTitleField) {
            jstring title = (jstring)env->GetObjectField(ig, titleField);
            jstring subtitle = (jstring)env->GetObjectField(ig, subTitleField);

            const char* titleStr = title ? env->GetStringUTFChars(title, nullptr) : nullptr;
            const char* subStr = subtitle ? env->GetStringUTFChars(subtitle, nullptr) : nullptr;

            bool gameEnded = false;
            if (titleStr) {
                std::string t(titleStr);
                if (t.find("VICTORY") != std::string::npos ||
                    t.find("DEFEAT") != std::string::npos ||
                    t.find("YOU DIED") != std::string::npos ||
                    t.find("GAME OVER") != std::string::npos ||
                    t.find("WINS!") != std::string::npos) {
                    gameEnded = true;
                }
                env->ReleaseStringUTFChars(title, titleStr);
            }

            if (gameEnded && !ggSent_) {
                delay_++;
                auto* delaySetting = GetSetting("Delay");
                if (delay_ >= (delaySetting ? delaySetting->iVal : 40)) {
                    auto player = JNIHelper::GetPlayer(env);
                    if (player) {
                        auto* msgSetting = GetSetting("Message");
                        const char* messages[] = {"gg", "GG", "gg wp", "Good Game!", "gf"};
                        int idx = msgSetting ? msgSetting->modeVal : 0;
                        if (idx >= 0 && idx < 5) {
                            jstring msg = env->NewStringUTF(messages[idx]);
                            if (msg) {
                                env->CallVoidMethod(player, JNIHelper::Get().sendChatMessage, msg);
                                if (env->ExceptionCheck()) env->ExceptionClear();
                                env->DeleteLocalRef(msg);
                            }
                        }
                        env->DeleteLocalRef(player);
                    }
                    ggSent_ = true;
                }
            }

            if (titleStr) env->ReleaseStringUTFChars(title, titleStr);
            if (subStr) env->ReleaseStringUTFChars(subtitle, subStr);
            if (title) env->DeleteLocalRef(title);
            if (subtitle) env->DeleteLocalRef(subtitle);
        }

        env->DeleteLocalRef(ig);
        env->DeleteLocalRef(mc);
    }

    void OnDisable(JNIEnv* env) override {
        ggSent_ = false;
        delay_ = 0;
    }

private:
    bool ggSent_ = false;
    int delay_ = 0;
};
