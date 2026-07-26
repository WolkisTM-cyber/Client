#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <GL/gl.h>

class Crosshair : public Module {
public:
    Crosshair() : Module("Crosshair", "Crosshair", Category::Visual, 0) {
        AddSetting(Setting::IntSetting("Size", "Size", 8, 2, 20));
        AddSetting(Setting::FloatSetting("Thickness", "Thickness", 1.5f, 0.5f, 4.0f));
        AddSetting(Setting::BoolSetting("Dot", "Center Dot", true));
    }

    void Render2D() {
        if (!IsEnabled()) return;
        auto* renderer = Renderer::GetInstance();

        RECT r; GetClientRect(GetDesktopWindow(), &r);
        int cx = r.right / 2;
        int cy = r.bottom / 2;
        int size = GetSetting("Size")->iVal;
        float thick = GetSetting("Thickness")->fVal;

        renderer->Setup2DProjection();

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(thick);

        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        glBegin(GL_LINES);
        // Top
        glVertex2i(cx, cy - size); glVertex2i(cx, cy - size/3);
        // Bottom
        glVertex2i(cx, cy + size); glVertex2i(cx, cy + size/3);
        // Left
        glVertex2i(cx - size, cy); glVertex2i(cx - size/3, cy);
        // Right
        glVertex2i(cx + size, cy); glVertex2i(cx + size/3, cy);
        glEnd();

        if (GetSetting("Dot")->bVal) {
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            glVertex2i(cx, cy);
            glEnd();
        }

        glPopAttrib();
        renderer->RestoreProjection();
    }
};
