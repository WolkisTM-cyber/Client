#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class TriggerBot : public Module {
public:
    TriggerBot() : Module("TriggerBot", "Trigger Bot", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Range", "Range", 4.5f, 1.0f, 6.0f));
        AddSetting(Setting::IntSetting("Delay", "Delay (ms)", 0, 0, 1000));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;

        if (delay_ > 0) { delay_ -= 50; return; }

        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jfieldID objectMouseOverField = env->GetFieldID(c.minecraft, "objectMouseOver", "Lnet/minecraft/util/MovingObjectPosition;");
        if (!objectMouseOverField) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }
        jobject obj = env->GetObjectField(mc, objectMouseOverField);
        if (!obj || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jclass movingObjectClass = env->FindClass("net/minecraft/util/MovingObjectPosition");
        if (!movingObjectClass) { env->DeleteLocalRef(obj); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jfieldID typeField = env->GetFieldID(movingObjectClass, "typeOfHit", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
        jfieldID entityHitField = env->GetFieldID(movingObjectClass, "entityHit", "Lnet/minecraft/entity/Entity;");
        if (!typeField || !entityHitField) {
            env->DeleteLocalRef(movingObjectClass); env->DeleteLocalRef(obj); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return;
        }

        jobject type = env->GetObjectField(obj, typeField);
        jobject entityHit = env->GetObjectField(obj, entityHitField);

        if (type && entityHit) {
            jclass entityEnum = env->FindClass("net/minecraft/util/MovingObjectPosition$MovingObjectType");
            if (entityEnum) {
                jfieldID entityField = env->GetStaticFieldID(entityEnum, "ENTITY", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
                if (entityField) {
                    jobject entityType = env->GetStaticObjectField(entityEnum, entityField);
                    if (entityType && env->IsSameObject(type, entityType)) {
                        if (!env->IsSameObject(entityHit, player)) {
                            jobject controller = JNIHelper::GetPlayerController(env);
                            if (controller) {
                                jmethodID attackMethod = env->GetMethodID(
                                    env->GetObjectClass(controller), "attackEntity",
                                    "(Lnet/minecraft/entity/player/EntityPlayer;Lnet/minecraft/entity/Entity;)V");
                                if (attackMethod) {
                                    env->CallVoidMethod(controller, attackMethod, player, entityHit);
                                    if (env->ExceptionCheck()) env->ExceptionClear();
                                    auto* delay = GetSetting("Delay");
                                    delay_ = delay ? delay->iVal : 0;
                                }
                            }
                            env->DeleteLocalRef(controller);
                        }
                    }
                    if (entityType) env->DeleteLocalRef(entityType);
                }
                env->DeleteLocalRef(entityEnum);
            }
        }

        if (type) env->DeleteLocalRef(type);
        if (entityHit) env->DeleteLocalRef(entityHit);
        env->DeleteLocalRef(movingObjectClass);
        env->DeleteLocalRef(obj);
        env->DeleteLocalRef(mc);
        env->DeleteLocalRef(player);
    }

private:
    int delay_ = 0;
};
