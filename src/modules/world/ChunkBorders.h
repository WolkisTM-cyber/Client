#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <GL/gl.h>

class ChunkBorders : public Module {
public:
    ChunkBorders() : Module("ChunkBorders", "Chunk Borders", Category::World, 0) {}

    void Render3D(JNIEnv* env) {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(player, c.posX);
        double pz = env->GetDoubleField(player, c.posZ);

        int chunkX = ((int)px) >> 4;
        int chunkZ = ((int)pz) >> 4;

        auto* renderer = Renderer::GetInstance();
        renderer->Setup3DProjection();

        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glLineWidth(1.0f);

        for (int dx = -4; dx <= 4; dx++) {
            for (int dz = -4; dz <= 4; dz++) {
                int cx = (chunkX + dx) << 4;
                int cz = (chunkZ + dz) << 4;

                glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
                glBegin(GL_LINE_LOOP);
                glVertex3i(cx, 0, cz);
                glVertex3i(cx + 16, 0, cz);
                glVertex3i(cx + 16, 0, cz + 16);
                glVertex3i(cx, 0, cz + 16);
                glEnd();
            }
        }

        glPopAttrib();
        renderer->RestoreProjection();
        env->DeleteLocalRef(player);
    }
};
