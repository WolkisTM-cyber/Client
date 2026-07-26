#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <GL/gl.h>

class MotionBlur : public Module {
public:
    MotionBlur() : Module("MotionBlur", "Motion Blur", Category::Visual, 0) {
        AddSetting(Setting::FloatSetting("Blur", "Blur", 0.7f, 0.1f, 1.0f));
    }

    void OnSwapBuffers() {
        if (!IsEnabled()) return;

        float blur = GetSetting("Blur")->fVal;
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendColor(0.0f, 0.0f, 0.0f, 1.0f - blur);

        // Accumulation buffer blur
        glAccum(GL_MULT, blur);
        glAccum(GL_ACCUM, 1.0f - blur);
        glAccum(GL_RETURN, 1.0f);

        glPopAttrib();
    }
};
