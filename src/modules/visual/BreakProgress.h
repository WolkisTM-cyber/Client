#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <GL/gl.h>

class BreakProgress : public Module {
public:
    BreakProgress() : Module("BreakProgress", "Break Progress", Category::Visual, 0) {}

    void OnSwapBuffers(JNIEnv* env) {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        auto& c = JNIHelper::Get();
        jobject obj = env->CallObjectMethod(mc, c.objectMouseOver);
        if (!obj || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(mc); return; }

        jclass mopClass = env->GetObjectClass(obj);
        jfieldID blockX = env->GetFieldID(mopClass, "blockX", "I");
        jfieldID blockY = env->GetFieldID(mopClass, "blockY", "I");
        jfieldID blockZ = env->GetFieldID(mopClass, "blockZ", "I");
        if (!blockX || !blockY || !blockZ) { env->DeleteLocalRef(mopClass); env->DeleteLocalRef(obj); env->DeleteLocalRef(mc); return; }

        int bx = env->GetIntField(obj, blockX);
        int by = env->GetIntField(obj, blockY);
        int bz = env->GetIntField(obj, blockZ);

        // Get block damage from controller
        auto controller = JNIHelper::GetPlayerController(env);
        float damage = 0.0f;
        if (controller) {
            jmethodID getCurDamage = env->GetMethodID(
                env->GetObjectClass(controller), "curBlockDamageMP", "()F");
            if (getCurDamage) {
                damage = env->CallFloatMethod(controller, getCurDamage);
                if (env->ExceptionCheck()) env->ExceptionClear();
            }
            env->DeleteLocalRef(controller);
        }

        if (damage > 0) {
            auto* renderer = Renderer::GetInstance();
            renderer->Setup3DProjection();

            glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);

            glColor4f(1.0f, 0.0f, 0.0f, damage * 0.5f);
            glBegin(GL_QUADS);
            glVertex3i(bx, by, bz);
            glVertex3i(bx+1, by, bz);
            glVertex3i(bx+1, by+1, bz);
            glVertex3i(bx, by+1, bz);

            glVertex3i(bx, by, bz+1);
            glVertex3i(bx+1, by, bz+1);
            glVertex3i(bx+1, by+1, bz+1);
            glVertex3i(bx, by+1, bz+1);

            glVertex3i(bx, by, bz);
            glVertex3i(bx, by, bz+1);
            glVertex3i(bx, by+1, bz+1);
            glVertex3i(bx, by+1, bz);

            glVertex3i(bx+1, by, bz);
            glVertex3i(bx+1, by, bz+1);
            glVertex3i(bx+1, by+1, bz+1);
            glVertex3i(bx+1, by+1, bz);

            glVertex3i(bx, by, bz);
            glVertex3i(bx+1, by, bz);
            glVertex3i(bx+1, by, bz+1);
            glVertex3i(bx, by, bz+1);

            glVertex3i(bx, by+1, bz);
            glVertex3i(bx+1, by+1, bz);
            glVertex3i(bx+1, by+1, bz+1);
            glVertex3i(bx, by+1, bz+1);
            glEnd();

            glPopAttrib();
            renderer->RestoreProjection();
        }

        env->DeleteLocalRef(mopClass);
        env->DeleteLocalRef(obj);
        env->DeleteLocalRef(mc);
    }
};
