#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <GL/gl.h>
#include <string>
#include <vector>

class NameTags : public Module {
public:
    NameTags() : Module("NameTags", "Name Tags", Category::Visual, 0) {
        AddSetting(Setting::BoolSetting("Self", "Show Self", false));
        AddSetting(Setting::BoolSetting("Health", "Show Health", true));
    }

    void Render3D(JNIEnv* env) {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jobject entities = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entities || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        jint size = env->CallIntMethod(entities, c.listSize);

        for (int i = 0; i < size && i < 100; i++) {
            jobject ent = env->CallObjectMethod(entities, c.listGet, i);
            if (!ent || env->ExceptionCheck()) { if (ent) env->DeleteLocalRef(ent); env->ExceptionClear(); continue; }

            if (!GetSetting("Self")->bVal && env->IsSameObject(ent, player)) { env->DeleteLocalRef(ent); continue; }
            if (!env->IsInstanceOf(ent, c.entityLivingBase)) { env->DeleteLocalRef(ent); continue; }

            jmethodID getName = env->GetMethodID(env->GetObjectClass(ent), "getName", "()Ljava/lang/String;");
            if (!getName) { env->DeleteLocalRef(ent); continue; }

            jstring nameObj = (jstring)env->CallObjectMethod(ent, getName);
            if (!nameObj || env->ExceptionCheck()) { if (nameObj) env->DeleteLocalRef(nameObj); env->ExceptionClear(); env->DeleteLocalRef(ent); continue; }

            const char* nameStr = env->GetStringUTFChars(nameObj, nullptr);
            if (!nameStr) { env->DeleteLocalRef(nameObj); env->DeleteLocalRef(ent); continue; }

            double ex = env->GetDoubleField(ent, c.posX);
            double ey = env->GetDoubleField(ent, c.posY) + 2.2;
            double ez = env->GetDoubleField(ent, c.posZ);

            // Render nametag using OpenGL billboard
            auto* renderer = Renderer::GetInstance();
            renderer->Setup3DProjection();

            glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);

            // Background
            glBegin(GL_QUADS);
            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            float width = strlen(nameStr) * 3.0f;
            glVertex3d(ex - width, ey - 0.1, ez);
            glVertex3d(ex + width, ey - 0.1, ez);
            glVertex3d(ex + width, ey + 0.3, ez);
            glVertex3d(ex - width, ey + 0.3, ez);
            glEnd();

            // Text outline using lines (simplified)
            glBegin(GL_LINE_LOOP);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glVertex3d(ex - width, ey - 0.1, ez);
            glVertex3d(ex + width, ey - 0.1, ez);
            glVertex3d(ex + width, ey + 0.3, ez);
            glVertex3d(ex - width, ey + 0.3, ez);
            glEnd();

            glPopAttrib();
            renderer->RestoreProjection();

            env->ReleaseStringUTFChars(nameObj, nameStr);
            env->DeleteLocalRef(nameObj);
            env->DeleteLocalRef(ent);
        }

        env->DeleteLocalRef(entities);
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }
};
