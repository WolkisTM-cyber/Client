#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <vector>

class Zoot : public Module {
public:
    Zoot() : Module("Zoot", "Zoot", Category::Combat, 0) {
        AddSetting(Setting::BoolSetting("Blindness", "Blindness", true));
        AddSetting(Setting::BoolSetting("Slowness", "Slowness", true));
        AddSetting(Setting::BoolSetting("Poison", "Poison", true));
        AddSetting(Setting::BoolSetting("Wither", "Wither", true));
        AddSetting(Setting::BoolSetting("Milk", "Auto Milk", false));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID getActiveEffects = env->GetMethodID(c.entityLivingBase,
            "getActivePotionEffects", "()Ljava/util/Collection;");
        if (!getActiveEffects) { env->DeleteLocalRef(player); return; }

        jobject effects = env->CallObjectMethod(player, getActiveEffects);
        if (!effects || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(player); return; }

        jmethodID collSize = env->GetMethodID(env->GetObjectClass(effects), "size", "()I");
        jmethodID toArray = env->GetMethodID(env->GetObjectClass(effects), "toArray", "()[Ljava/lang/Object;");
        if (!collSize || !toArray) { env->DeleteLocalRef(effects); env->DeleteLocalRef(player); return; }

        jint size = env->CallIntMethod(effects, collSize);
        if (size > 0) {
            jobjectArray arr = (jobjectArray)env->CallObjectMethod(effects, toArray);
            if (arr && !env->ExceptionCheck()) {
                for (int i = 0; i < size; i++) {
                    jobject effect = env->GetObjectArrayElement(arr, i);
                    if (!effect) continue;

                    jclass peClass = env->GetObjectClass(effect);
                    jmethodID getPotionID = env->GetMethodID(peClass, "getPotionID", "()I");
                    if (!getPotionID) { env->DeleteLocalRef(effect); env->DeleteLocalRef(peClass); continue; }

                    int id = env->CallIntMethod(effect, getPotionID);
                    bool shouldRemove = false;

                    if (id == 15 && GetSetting("Blindness")->bVal) shouldRemove = true;
                    if (id == 2 && GetSetting("Slowness")->bVal) shouldRemove = true;
                    if (id == 19 && GetSetting("Poison")->bVal) shouldRemove = true;
                    if (id == 20 && GetSetting("Wither")->bVal) shouldRemove = true;

                    if (shouldRemove) {
                        jmethodID removeEffect = env->GetMethodID(c.entityLivingBase,
                            "removePotionEffect", "(I)V");
                        if (removeEffect) {
                            env->CallVoidMethod(player, removeEffect, id);
                            if (env->ExceptionCheck()) env->ExceptionClear();
                        }
                    }

                    env->DeleteLocalRef(peClass);
                    env->DeleteLocalRef(effect);
                }
            }
            if (arr) env->DeleteLocalRef(arr);
        }

        env->DeleteLocalRef(effects);
        env->DeleteLocalRef(player);
    }
};
