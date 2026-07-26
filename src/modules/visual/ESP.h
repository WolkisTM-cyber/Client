#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <GL/gl.h>

class ESP : public Module {
public:
    ESP() : Module("ESP", "ESP", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Box", "2D", "Tracer"}, 0));
    }

    void Render3D(JNIEnv* env) {
        if (!IsEnabled() || !env) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) { env->DeleteLocalRef(world); return; }

        auto& c = JNIHelper::Get();
        jobject entityList = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (entityList) {
            jint size = env->CallIntMethod(entityList, c.listSize);
            glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glLineWidth(2.0f);
            glColor4f(0.0f, 0.8f, 0.78f, 0.85f); // Cyan accent

            for (int i = 0; i < size && i < 100; i++) {
                jobject entity = env->CallObjectMethod(entityList, c.listGet, i);
                if (entity && !env->IsSameObject(entity, player)) {
                    double ex = env->GetDoubleField(entity, c.posX);
                    double ey = env->GetDoubleField(entity, c.posY);
                    double ez = env->GetDoubleField(entity, c.posZ);

                    glBegin(GL_LINE_LOOP);
                    glVertex3d(ex - 0.4, ey, ez - 0.4);
                    glVertex3d(ex + 0.4, ey, ez - 0.4);
                    glVertex3d(ex + 0.4, ey, ez + 0.4);
                    glVertex3d(ex - 0.4, ey, ez + 0.4);
                    glEnd();

                    glBegin(GL_LINE_LOOP);
                    glVertex3d(ex - 0.4, ey + 1.8, ez - 0.4);
                    glVertex3d(ex + 0.4, ey + 1.8, ez - 0.4);
                    glVertex3d(ex + 0.4, ey + 1.8, ez + 0.4);
                    glVertex3d(ex - 0.4, ey + 1.8, ez + 0.4);
                    glEnd();
                }
                if (entity) env->DeleteLocalRef(entity);
            }
            glPopAttrib();
            env->DeleteLocalRef(entityList);
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }
};

