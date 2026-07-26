#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class ClickTP : public Module {
public:
    ClickTP() : Module("ClickTP", "Click TP", Category::Player, 0) {
        AddSetting(Setting::BoolSetting("MiddleClick", "Middle Click", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        if (!(GetSetting("MiddleClick")->bVal && (GetAsyncKeyState(VK_MBUTTON) & 1))) return;

        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jfieldID objectMouseOverField = env->GetFieldID(c.minecraft, "objectMouseOver", "Lnet/minecraft/util/MovingObjectPosition;");
        if (!objectMouseOverField) { env->DeleteLocalRef(mc); return; }
        jobject obj = env->GetObjectField(mc, objectMouseOverField);
        if (!obj || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(mc); return; }

        jclass mopClass = env->GetObjectClass(obj);
        jfieldID typeField = env->GetFieldID(mopClass, "typeOfHit",
            "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
        jfieldID blockX = env->GetFieldID(mopClass, "blockX", "I");
        jfieldID blockY = env->GetFieldID(mopClass, "blockY", "I");
        jfieldID blockZ = env->GetFieldID(mopClass, "blockZ", "I");

        if (!typeField || !blockX || !blockY || !blockZ) {
            env->DeleteLocalRef(mopClass); env->DeleteLocalRef(obj); env->DeleteLocalRef(mc); return;
        }

        jobject type = env->GetObjectField(obj, typeField);
        jclass blockEnum = env->FindClass("net/minecraft/util/MovingObjectPosition$MovingObjectType");
        jfieldID blockField = env->GetStaticFieldID(blockEnum, "BLOCK",
            "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
        if (!blockField) { env->DeleteLocalRef(blockEnum); env->DeleteLocalRef(type); env->DeleteLocalRef(mopClass); env->DeleteLocalRef(obj); env->DeleteLocalRef(mc); return; }

        jobject blockType = env->GetStaticObjectField(blockEnum, blockField);
        bool isBlock = type && blockType && env->IsSameObject(type, blockType);

        if (isBlock) {
            int bx = env->GetIntField(obj, blockX);
            int by = env->GetIntField(obj, blockY);
            int bz = env->GetIntField(obj, blockZ);

            auto player = JNIHelper::GetPlayer(env);
            if (player) {
                env->SetDoubleField(player, c.posX, bx + 0.5);
                env->SetDoubleField(player, c.posY, by + 1.0);
                env->SetDoubleField(player, c.posZ, bz + 0.5);
                env->SetBooleanField(player, c.onGround, true);
                env->DeleteLocalRef(player);
            }
        }

        if (blockType) env->DeleteLocalRef(blockType);
        if (blockEnum) env->DeleteLocalRef(blockEnum);
        if (type) env->DeleteLocalRef(type);
        env->DeleteLocalRef(mopClass);
        env->DeleteLocalRef(obj);
        env->DeleteLocalRef(mc);
    }
};
