#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <GL/gl.h>
#include <cmath>

class TargetESP : public Module {
public:
    TargetESP() : Module("TargetESP", "Target ESP", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Style", "Style", {"Ring", "Box", "Tracer"}, 0));
        AddSetting(Setting::FloatSetting("Radius", "Radius", 0.8f, 0.4f, 2.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        angle_ += 3.0f;
        if (angle_ >= 360.0f) angle_ -= 360.0f;
    }

    void Render3D(JNIEnv* env, jobject target) {
        if (!IsEnabled() || !env || !target) return;
        auto& c = JNIHelper::Get();

        double tx = env->GetDoubleField(target, c.posX);
        double ty = env->GetDoubleField(target, c.posY);
        double tz = env->GetDoubleField(target, c.posZ);

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(2.5f);

        int style = GetSetting("Style")->modeVal;
        float radius = GetSetting("Radius")->fVal;

        if (style == 0) { // 3D Pulsing Ring around target feet
            glColor4f(0.0f, 0.8f, 0.78f, 0.85f);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < 360; i += 10) {
                double rad = i * 3.14159265 / 180.0;
                glVertex3d(tx + cos(rad) * radius, ty + 0.1 + sin(angle_ * 3.14159 / 180.0) * 0.1, tz + sin(rad) * radius);
            }
            glEnd();
        } else if (style == 1) { // 3D Box
            glColor4f(0.42f, 0.36f, 0.90f, 0.85f);
            glBegin(GL_LINE_LOOP);
            glVertex3d(tx - radius, ty, tz - radius);
            glVertex3d(tx + radius, ty, tz - radius);
            glVertex3d(tx + radius, ty, tz + radius);
            glVertex3d(tx - radius, ty, tz + radius);
            glEnd();

            glBegin(GL_LINE_LOOP);
            glVertex3d(tx - radius, ty + 1.8, tz - radius);
            glVertex3d(tx + radius, ty + 1.8, tz - radius);
            glVertex3d(tx + radius, ty + 1.8, tz + radius);
            glVertex3d(tx - radius, ty + 1.8, tz + radius);
            glEnd();
        }

        glPopAttrib();
    }

private:
    float angle_ = 0.0f;
};
