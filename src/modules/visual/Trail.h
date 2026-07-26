#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <GL/gl.h>
#include <deque>

class Trail : public Module {
public:
    Trail() : Module("Trail", "Trail", Category::Visual, 0) {
        AddSetting(Setting::IntSetting("Length", "Length", 50, 10, 200));
        AddSetting(Setting::BoolSetting("Rainbow", "Rainbow", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        double x = env->GetDoubleField(player, c.posX);
        double y = env->GetDoubleField(player, c.posY) + 1.0;
        double z = env->GetDoubleField(player, c.posZ);

        trail_.push_front({x, y, z});
        int maxLen = GetSetting("Length")->iVal;
        while ((int)trail_.size() > maxLen) trail_.pop_back();

        env->DeleteLocalRef(player);
    }

    void Render3D() {
        if (!IsEnabled() || trail_.size() < 2) return;

        auto* renderer = Renderer::GetInstance();
        renderer->Setup3DProjection();

        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(2.0f);

        bool rainbow = GetSetting("Rainbow")->bVal;
        float hue = 0.0f;

        glBegin(GL_LINE_STRIP);
        for (size_t i = 0; i < trail_.size(); i++) {
            if (rainbow) hue = (float)i / trail_.size();
            glColor4f(
                rainbow ? (sin(hue * 6.283f) * 0.5f + 0.5f) : 1.0f,
                rainbow ? (sin((hue + 0.33f) * 6.283f) * 0.5f + 0.5f) : 0.5f,
                rainbow ? (sin((hue + 0.66f) * 6.283f) * 0.5f + 0.5f) : 0.0f,
                0.3f + 0.7f * (1.0f - (float)i / trail_.size())
            );
            glVertex3d(trail_[i].x, trail_[i].y, trail_[i].z);
        }
        glEnd();

        glPopAttrib();
        renderer->RestoreProjection();
    }

private:
    struct Node { double x, y, z; };
    std::deque<Node> trail_;
};
