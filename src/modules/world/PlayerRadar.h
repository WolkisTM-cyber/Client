#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <GL/gl.h>

class PlayerRadar : public Module {
public:
    PlayerRadar() : Module("PlayerRadar", "Player Radar", Category::World, 0) {}

    void Render2D(JNIEnv* env) {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(player, c.posX);
        double pz = env->GetDoubleField(player, c.posZ);
        float yaw = env->GetFloatField(player, c.rotationYaw);

        jobject entities = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entities || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        jint size = env->CallIntMethod(entities, c.listSize);

        // Radar circle at top-right
        RECT r; GetClientRect(GetDesktopWindow(), &r);
        int radarSize = 64;
        int radarX = r.right - radarSize - 10;
        int radarY = 10;

        auto* renderer = Renderer::GetInstance();
        renderer->Setup2DProjection();

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);

        // Background circle
        glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(radarX + radarSize/2, radarY + radarSize/2);
        for (int a = 0; a <= 360; a += 10) {
            float rad = a * 3.14159f / 180.0f;
            glVertex2i(radarX + radarSize/2 + (int)(cos(rad) * radarSize/2),
                       radarY + radarSize/2 + (int)(sin(rad) * radarSize/2));
        }
        glEnd();

        // Border
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        glBegin(GL_LINE_LOOP);
        for (int a = 0; a <= 360; a += 10) {
            float rad = a * 3.14159f / 180.0f;
            glVertex2i(radarX + radarSize/2 + (int)(cos(rad) * radarSize/2),
                       radarY + radarSize/2 + (int)(sin(rad) * radarSize/2));
        }
        glEnd();

        // Player dots
        for (int i = 0; i < size && i < 50; i++) {
            jobject ent = env->CallObjectMethod(entities, c.listGet, i);
            if (!ent || env->ExceptionCheck()) { if (ent) env->DeleteLocalRef(ent); env->ExceptionClear(); continue; }
            if (env->IsSameObject(ent, player)) { env->DeleteLocalRef(ent); continue; }

            double ex = env->GetDoubleField(ent, c.posX);
            double ez = env->GetDoubleField(ent, c.posZ);
            double dx = ex - px;
            double dz = ez - pz;
            double dist = sqrt(dx*dx + dz*dz);

            if (dist < 50 && dist > 0.5) {
                float angleTo = (float)(atan2(dz, dx) * 180.0 / 3.14159);
                float relativeAngle = angleTo - yaw;
                float rad = relativeAngle * 3.14159f / 180.0f;

                int dotX = radarX + radarSize/2 + (int)(cos(rad) * dist / 50 * radarSize/2);
                int dotY = radarY + radarSize/2 + (int)(sin(rad) * dist / 50 * radarSize/2);

                glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
                glBegin(GL_QUADS);
                glVertex2i(dotX - 1, dotY - 1);
                glVertex2i(dotX + 1, dotY - 1);
                glVertex2i(dotX + 1, dotY + 1);
                glVertex2i(dotX - 1, dotY + 1);
                glEnd();
            }

            env->DeleteLocalRef(ent);
        }

        // Center dot (self)
        glColor4f(0.0f, 1.0f, 0.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2i(radarX + radarSize/2 - 2, radarY + radarSize/2 - 2);
        glVertex2i(radarX + radarSize/2 + 2, radarY + radarSize/2 - 2);
        glVertex2i(radarX + radarSize/2 + 2, radarY + radarSize/2 + 2);
        glVertex2i(radarX + radarSize/2 - 2, radarY + radarSize/2 + 2);
        glEnd();

        glPopAttrib();
        renderer->RestoreProjection();

        env->DeleteLocalRef(entities);
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }
};
