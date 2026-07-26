#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class HitBox : public Module {
public:
    HitBox() : Module("HitBox", "HitBox", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Width", "Width", 0.6f, 0.1f, 3.0f));
        AddSetting(Setting::FloatSetting("Height", "Height", 1.8f, 0.1f, 5.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jobject entities = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entities || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        jint size = env->CallIntMethod(entities, c.listSize);
        for (int i = 0; i < size && i < 50; i++) {
            jobject ent = env->CallObjectMethod(entities, c.listGet, i);
            if (!ent || env->ExceptionCheck()) { if (ent) env->DeleteLocalRef(ent); env->ExceptionClear(); continue; }
            if (!env->IsSameObject(ent, player) && env->IsInstanceOf(ent, c.entityLivingBase)) {
                float w = GetSetting("Width")->fVal;
                float h = GetSetting("Height")->fVal;
                jclass eClass = env->GetObjectClass(ent);
                if (eClass) {
                    jmethodID setSize = env->GetMethodID(eClass, "setSize", "(FF)V");
                    if (setSize) {
                        env->CallVoidMethod(ent, setSize, w, h);
                        if (env->ExceptionCheck()) env->ExceptionClear();
                    }
                    env->DeleteLocalRef(eClass);
                }
            }
            env->DeleteLocalRef(ent);
        }

        env->DeleteLocalRef(entities);
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }
};
