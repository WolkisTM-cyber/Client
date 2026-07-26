#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <GL/gl.h>
#include <unordered_map>
#include <ctime>

class NewChunks : public Module {
public:
    NewChunks() : Module("NewChunks", "New Chunks", Category::World, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        int cx = ((int)env->GetDoubleField(player, c.posX)) >> 4;
        int cz = ((int)env->GetDoubleField(player, c.posZ)) >> 4;

        // Mark chunks near player as loaded
        for (int dx = -2; dx <= 2; dx++) {
            for (int dz = -2; dz <= 2; dz++) {
                int64_t key = (int64_t)(cx + dx) << 32 | (cz + dz);
                if (chunks_.find(key) == chunks_.end()) {
                    // New chunk detected!
                    chunks_[key] = clock();
                }
            }
        }

        // Remove old chunks (10s timeout)
        clock_t now = clock();
        for (auto it = chunks_.begin(); it != chunks_.end(); ) {
            if (now - it->second > 10000) it = chunks_.erase(it);
            else ++it;
        }

        env->DeleteLocalRef(player);
    }

    void Render3D() {
        if (!IsEnabled() || chunks_.empty()) return;

        auto* renderer = Renderer::GetInstance();
        renderer->Setup3DProjection();

        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glLineWidth(1.0f);

        clock_t now = clock();
        for (auto& [key, time] : chunks_) {
            int cx = (int)(key >> 32);
            int cz = (int)(key & 0xFFFFFFFF);
            int bx = cx << 4;
            int bz = cz << 4;

            float alpha = 1.0f - (float)(now - time) / 10000.0f;
            if (alpha < 0.1f) continue;

            glColor4f(0.0f, 1.0f, 0.0f, alpha);
            glBegin(GL_LINE_LOOP);
            glVertex3i(bx, 0, bz);
            glVertex3i(bx + 16, 0, bz);
            glVertex3i(bx + 16, 0, bz + 16);
            glVertex3i(bx, 0, bz + 16);
            glEnd();

            // Vertical pillars at corners
            glBegin(GL_LINES);
            glVertex3i(bx, 0, bz); glVertex3i(bx, 255, bz);
            glVertex3i(bx + 16, 0, bz); glVertex3i(bx + 16, 255, bz);
            glVertex3i(bx, 0, bz + 16); glVertex3i(bx, 255, bz + 16);
            glVertex3i(bx + 16, 0, bz + 16); glVertex3i(bx + 16, 255, bz + 16);
            glEnd();
        }

        glPopAttrib();
        renderer->RestoreProjection();
    }

private:
    std::unordered_map<int64_t, clock_t> chunks_;
};
