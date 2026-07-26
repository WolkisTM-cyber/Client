#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <GL/gl.h>
#include <vector>
#include <cmath>

class CaveFinder : public Module {
public:
    CaveFinder() : Module("CaveFinder", "Cave Finder", Category::World, 0) {
        AddSetting(Setting::IntSetting("Range", "Range", 32, 16, 64));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY);
        double pz = env->GetDoubleField(player, c.posZ);
        int range = GetSetting("Range")->iVal;

        caves_.clear();

        jmethodID getBlock = env->GetMethodID(
            env->GetObjectClass(world), "getBlock",
            "(III)Lnet/minecraft/block/Block;");
        jmethodID isAir = env->GetMethodID(
            env->GetObjectClass(world), "isAirBlock",
            "(III)Z");

        if (!getBlock || !isAir) { env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        for (int x = (int)px - range; x <= (int)px + range; x++) {
            for (int y = (int)py - range; y <= (int)py + range && y < 64; y++) {
                for (int z = (int)pz - range; z <= (int)pz + range; z++) {
                    double dx = x - px, dy = y - py, dz = z - pz;
                    if (sqrt(dx*dx + dy*dy + dz*dz) > range) continue;

                    // Check if it's an air pocket surrounded by stone
                    jboolean isAirBlock = env->CallBooleanMethod(world, isAir, x, y, z);
                    if (isAirBlock) {
                        // Check if surrounded by stone/dirt
                        jboolean above = env->CallBooleanMethod(world, isAir, x, y+1, z);
                        jboolean below = env->CallBooleanMethod(world, isAir, x, y-1, z);
                        int stoneCount = 0;
                        for (int nx = -1; nx <= 1; nx++) {
                            for (int nz = -1; nz <= 1; nz++) {
                                if (nx == 0 && nz == 0) continue;
                                if (!env->CallBooleanMethod(world, isAir, x+nx, y, z+nz)) stoneCount++;
                            }
                        }
                        if (above && below && stoneCount >= 4) {
                            caves_.push_back({(double)x + 0.5, (double)y, (double)z + 0.5});
                        }
                    }
                }
            }
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }

    void Render3D() {
        if (!IsEnabled() || caves_.empty()) return;

        auto* renderer = Renderer::GetInstance();
        renderer->Setup3DProjection();

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        for (auto& c : caves_) {
            glColor4f(0.0f, 1.0f, 0.0f, 0.3f);
            glBegin(GL_QUADS);
            glVertex3d(c.x - 0.5, c.y, c.z - 0.5);
            glVertex3d(c.x + 0.5, c.y, c.z - 0.5);
            glVertex3d(c.x + 0.5, c.y + 1, c.z - 0.5);
            glVertex3d(c.x - 0.5, c.y + 1, c.z - 0.5);
            glVertex3d(c.x - 0.5, c.y, c.z + 0.5);
            glVertex3d(c.x + 0.5, c.y, c.z + 0.5);
            glVertex3d(c.x + 0.5, c.y + 1, c.z + 0.5);
            glVertex3d(c.x - 0.5, c.y + 1, c.z + 0.5);
            glEnd();
        }

        glPopAttrib();
        renderer->RestoreProjection();
    }

private:
    struct Cave { double x, y, z; };
    std::vector<Cave> caves_;
};
