#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <GL/gl.h>

class BlockOutline : public Module {
public:
    BlockOutline() : Module("BlockOutline", "Block Outline", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Color", "Color", {"Red", "Green", "Blue", "White"}, 0));
        AddSetting(Setting::FloatSetting("Width", "Width", 2.0f, 0.5f, 5.0f));
    }

    void OnSwapBuffers(JNIEnv* env) {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jobject obj = env->CallObjectMethod(mc, c.objectMouseOver);
        if (!obj || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(mc); return; }

        jclass mopClass = env->GetObjectClass(obj);
        jfieldID typeField = env->GetFieldID(mopClass, "typeOfHit", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
        jfieldID blockX = env->GetFieldID(mopClass, "blockX", "I");
        jfieldID blockY = env->GetFieldID(mopClass, "blockY", "I");
        jfieldID blockZ = env->GetFieldID(mopClass, "blockZ", "I");

        if (!typeField || !blockX || !blockY || !blockZ) {
            env->DeleteLocalRef(mopClass); env->DeleteLocalRef(obj); env->DeleteLocalRef(mc); return;
        }

        jobject type = env->GetObjectField(obj, typeField);
        jclass blockEnum = env->FindClass("net/minecraft/util/MovingObjectPosition$MovingObjectType");
        jfieldID blockField = env->GetStaticFieldID(blockEnum, "BLOCK", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
        if (!blockField) { env->DeleteLocalRef(blockEnum); env->DeleteLocalRef(type); env->DeleteLocalRef(mopClass); env->DeleteLocalRef(obj); env->DeleteLocalRef(mc); return; }

        jobject blockType = env->GetStaticObjectField(blockEnum, blockField);
        bool isBlock = type && blockType && env->IsSameObject(type, blockType);

        if (isBlock) {
            int bx = env->GetIntField(obj, blockX);
            int by = env->GetIntField(obj, blockY);
            int bz = env->GetIntField(obj, blockZ);

            auto* renderer = Renderer::GetInstance();
            renderer->Setup3DProjection();

            glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glLineWidth(GetSetting("Width")->fVal);

            float r=1,g=0,b=0;
            switch (GetSetting("Color")->modeVal) {
                case 0: r=1;g=0;b=0; break;
                case 1: r=0;g=1;b=0; break;
                case 2: r=0;g=0;b=1; break;
                case 3: r=1;g=1;b=1; break;
            }

            glBegin(GL_LINE_STRIP);
            glColor4f(r,g,b,0.8f);
            glVertex3i(bx, by, bz);
            glVertex3i(bx+1, by, bz);
            glVertex3i(bx+1, by, bz+1);
            glVertex3i(bx, by, bz+1);
            glVertex3i(bx, by, bz);
            glVertex3i(bx, by+1, bz);
            glVertex3i(bx+1, by+1, bz);
            glVertex3i(bx+1, by+1, bz+1);
            glVertex3i(bx, by+1, bz+1);
            glVertex3i(bx, by+1, bz);
            glEnd();

            glBegin(GL_LINES);
            glVertex3i(bx+1, by, bz);
            glVertex3i(bx+1, by+1, bz);
            glVertex3i(bx+1, by, bz+1);
            glVertex3i(bx+1, by+1, bz+1);
            glVertex3i(bx, by, bz+1);
            glVertex3i(bx, by+1, bz+1);
            glEnd();

            glPopAttrib();
            renderer->RestoreProjection();
        }

        if (blockType) env->DeleteLocalRef(blockType);
        if (blockEnum) env->DeleteLocalRef(blockEnum);
        if (type) env->DeleteLocalRef(type);
        env->DeleteLocalRef(mopClass);
        env->DeleteLocalRef(obj);
        env->DeleteLocalRef(mc);
    }
};
