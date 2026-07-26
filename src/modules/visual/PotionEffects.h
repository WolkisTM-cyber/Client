#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>
#include <vector>

class PotionEffects : public Module {
public:
    PotionEffects() : Module("PotionEffects", "Potion Effects", Category::Visual, 0) {}

    void Render(JNIEnv* env, jobject fr, jmethodID drawStr) {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID getActiveEffects = env->GetMethodID(c.entityLivingBase,
            "getActivePotionEffects", "()Ljava/util/Collection;");
        if (!getActiveEffects) { env->DeleteLocalRef(player); return; }

        jobject effects = env->CallObjectMethod(player, getActiveEffects);
        if (!effects || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(player); return; }

        jmethodID toArray = env->GetMethodID(env->GetObjectClass(effects), "toArray", "()[Ljava/lang/Object;");
        if (!toArray) { env->DeleteLocalRef(effects); env->DeleteLocalRef(player); return; }

        jobjectArray arr = (jobjectArray)env->CallObjectMethod(effects, toArray);
        if (!arr || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(effects); env->DeleteLocalRef(player); return; }

        jint size = env->GetArrayLength(arr);
        std::vector<std::string> lines;
        std::vector<int> colors;

        for (int i = 0; i < size; i++) {
            jobject effect = env->GetObjectArrayElement(arr, i);
            if (!effect) continue;

            jclass peClass = env->GetObjectClass(effect);
            jmethodID getID = env->GetMethodID(peClass, "getPotionID", "()I");
            jmethodID getAmplifier = env->GetMethodID(peClass, "getAmplifier", "()I");
            jmethodID getDuration = env->GetMethodID(peClass, "getDuration", "()I");

            if (getID && getAmplifier && getDuration) {
                int id = env->CallIntMethod(effect, getID);
                int amp = env->CallIntMethod(effect, getAmplifier) + 1;
                int dur = env->CallIntMethod(effect, getDuration) / 20;

                const char* name = "Unknown";
                int color = 0xFFFFFF;
                switch (id) {
                case 1: name = "Speed"; color = 0x55FF55; break;
                case 2: name = "Slowness"; color = 0x888888; break;
                case 3: name = "Haste"; color = 0xFFAA55; break;
                case 4: name = "M.Fatigue"; color = 0x444444; break;
                case 5: name = "Strength"; color = 0xFF5555; break;
                case 6: name = "Heal"; color = 0xFF55FF; break;
                case 7: name = "Harm"; color = 0xAA0000; break;
                case 8: name = "Jump"; color = 0x55FF55; break;
                case 9: name = "Nausea"; color = 0x884422; break;
                case 10: name = "Regen"; color = 0xFF55FF; break;
                case 11: name = "Resist"; color = 0xAAAAAA; break;
                case 12: name = "FireRes"; color = 0xFF8800; break;
                case 13: name = "WaterBrth"; color = 0x4444FF; break;
                case 14: name = "Invis"; color = 0x888888; break;
                case 15: name = "Blindness"; color = 0x222222; break;
                case 16: name = "NightVis"; color = 0x4444FF; break;
                case 17: name = "Hunger"; color = 0x88AA44; break;
                case 18: name = "Weakness"; color = 0x884444; break;
                case 19: name = "Poison"; color = 0x44FF44; break;
                case 20: name = "Wither"; color = 0x222222; break;
                case 21: name = "HealthBoost"; color = 0xFF88AA; break;
                case 22: name = "Absorb"; color = 0xFFAA00; break;
                case 23: name = "Sat"; color = 0xFF8844; break;
                }

                char buf[64];
                int mins = dur / 60;
                int secs = dur % 60;
                snprintf(buf, sizeof(buf), "%s %d %d:%02d", name, amp, mins, secs);
                lines.push_back(buf);
                colors.push_back(color);
            }

            env->DeleteLocalRef(peClass);
            env->DeleteLocalRef(effect);
        }

        env->DeleteLocalRef(arr);
        env->DeleteLocalRef(effects);
        env->DeleteLocalRef(player);

        int y = 130;
        for (size_t i = 0; i < lines.size() && i < 5; i++) {
            jstring text = env->NewStringUTF(lines[i].c_str());
            if (text && drawStr) {
                env->CallIntMethod(fr, drawStr, text, 4, y, colors[i]);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }
            y += 12;
        }
    }
};
