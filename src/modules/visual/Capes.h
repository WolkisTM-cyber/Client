#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <GL/gl.h>
#include <cmath>

class Capes : public Module {
public:
    Capes() : Module("Capes", "Custom Capes", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Style", "Cape Style", {"Red", "Rainbow", "Dark"}, 1));
        AddSetting(Setting::FloatSetting("WaveSpeed", "Wave Speed", 1.0f, 0.1f, 3.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        wave_ += 0.05f * GetSetting("WaveSpeed")->fVal;
        if (wave_ > 6.283f) wave_ -= 6.283f;
    }

    void Render3D(JNIEnv* env) {
        if (!IsEnabled() || !env) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY) + 1.2;
        double pz = env->GetDoubleField(player, c.posZ);
        float yaw = env->GetFloatField(player, c.rotationYaw);

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        int style = GetSetting("Style")->modeVal;
        if (style == 0) glColor4f(1.0f, 0.2f, 0.2f, 0.9f);
        else if (style == 1) glColor4f(sin(wave_) * 0.5f + 0.5f, sin(wave_ + 2.0f) * 0.5f + 0.5f, sin(wave_ + 4.0f) * 0.5f + 0.5f, 0.9f);
        else glColor4f(0.1f, 0.1f, 0.1f, 0.95f);

        double rad = yaw * 3.14159 / 180.0;
        double backX = px - sin(rad) * 0.25;
        double backZ = pz + cos(rad) * 0.25;
        double swing = sin(wave_) * 0.1;

        glBegin(GL_QUADS);
        glVertex3d(backX - 0.2, py, backZ);
        glVertex3d(backX + 0.2, py, backZ);
        glVertex3d(backX + 0.2 + swing, py - 0.9, backZ + swing);
        glVertex3d(backX - 0.2 + swing, py - 0.9, backZ + swing);
        glEnd();

        glPopAttrib();
        env->DeleteLocalRef(player);
    }

private:
    float wave_ = 0.0f;
};
