#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class BossBar : public Module {
public:
    BossBar() : Module("BossBar", "Boss Bar", Category::Visual, 0) {
        AddSetting(Setting::BoolSetting("Hide", "Hide Boss Bar", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jfieldID worldField = c.theWorld;
        if (!worldField) { env->DeleteLocalRef(mc); return; }

        jobject world = env->GetObjectField(mc, worldField);
        if (!world) { env->DeleteLocalRef(mc); return; }

        if (GetSetting("Hide")->bVal) {
            // Clear boss bar entities by removing them from world
            jmethodID getEntityList = env->GetMethodID(
                env->GetObjectClass(world), "getLoadedEntityList",
                "()Ljava/util/List;");
            if (getEntityList) {
                jobject entities = env->CallObjectMethod(world, getEntityList);
                if (entities && !env->ExceptionCheck()) {
                    jmethodID listSize = env->GetMethodID(
                        env->GetObjectClass(entities), "size", "()I");
                    if (listSize) {
                        jint size = env->CallIntMethod(entities, listSize);
                        if (size > 0) {
                            jmethodID clear = env->GetMethodID(
                                env->GetObjectClass(entities), "clear", "()V");
                            if (clear) {
                                env->CallVoidMethod(entities, clear);
                            }
                        }
                    }
                    env->DeleteLocalRef(entities);
                } else if (env->ExceptionCheck()) env->ExceptionClear();
            }
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(mc);
    }
};
