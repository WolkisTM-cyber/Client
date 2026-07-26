#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <GL/gl.h>

class ItemESP : public Module {
public:
    ItemESP() : Module("ItemESP", "Item ESP", Category::Visual, 0) {}

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jobject entities = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entities || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        jint size = env->CallIntMethod(entities, c.listSize);
        items_.clear();

        jclass itemClass = env->FindClass("net/minecraft/entity/item/EntityItem");
        if (!itemClass) { env->DeleteLocalRef(entities); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        for (int i = 0; i < size && i < 200; i++) {
            jobject ent = env->CallObjectMethod(entities, c.listGet, i);
            if (!ent || env->ExceptionCheck()) { if (ent) env->DeleteLocalRef(ent); env->ExceptionClear(); continue; }

            if (env->IsInstanceOf(ent, itemClass)) {
                ItemEnt item;
                item.x = env->GetDoubleField(ent, c.posX);
                item.y = env->GetDoubleField(ent, c.posY);
                item.z = env->GetDoubleField(ent, c.posZ);
                items_.push_back(item);
            }
            env->DeleteLocalRef(ent);
        }

        env->DeleteLocalRef(itemClass);
        env->DeleteLocalRef(entities);
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }

    void Render3D() {
        if (items_.empty()) return;

        auto* renderer = Renderer::GetInstance();
        renderer->Setup3DProjection();

        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(2.0f);

        for (auto& item : items_) {
            glBegin(GL_LINE_STRIP);
            glColor4f(1.0f, 0.8f, 0.0f, 0.8f);
            glVertex3d(item.x - 0.25, item.y, item.z - 0.25);
            glVertex3d(item.x - 0.25, item.y + 0.5, item.z - 0.25);
            glVertex3d(item.x + 0.25, item.y + 0.5, item.z - 0.25);
            glVertex3d(item.x + 0.25, item.y, item.z - 0.25);
            glVertex3d(item.x - 0.25, item.y, item.z - 0.25);
            glEnd();

            glBegin(GL_LINE_STRIP);
            glColor4f(1.0f, 0.8f, 0.0f, 0.8f);
            glVertex3d(item.x - 0.25, item.y, item.z + 0.25);
            glVertex3d(item.x - 0.25, item.y + 0.5, item.z + 0.25);
            glVertex3d(item.x + 0.25, item.y + 0.5, item.z + 0.25);
            glVertex3d(item.x + 0.25, item.y, item.z + 0.25);
            glVertex3d(item.x - 0.25, item.y, item.z + 0.25);
            glEnd();

            glBegin(GL_LINES);
            glColor4f(1.0f, 0.8f, 0.0f, 0.8f);
            glVertex3d(item.x - 0.25, item.y, item.z - 0.25);
            glVertex3d(item.x - 0.25, item.y, item.z + 0.25);
            glVertex3d(item.x + 0.25, item.y, item.z - 0.25);
            glVertex3d(item.x + 0.25, item.y, item.z + 0.25);
            glVertex3d(item.x - 0.25, item.y + 0.5, item.z - 0.25);
            glVertex3d(item.x - 0.25, item.y + 0.5, item.z + 0.25);
            glVertex3d(item.x + 0.25, item.y + 0.5, item.z - 0.25);
            glVertex3d(item.x + 0.25, item.y + 0.5, item.z + 0.25);
            glEnd();
        }

        glPopAttrib();
        renderer->RestoreProjection();
    }

private:
    struct ItemEnt { double x, y, z; };
    std::vector<ItemEnt> items_;
};
