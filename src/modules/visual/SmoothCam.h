#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class SmoothCam : public Module {
public:
    SmoothCam() : Module("SmoothCam", "Smooth Cam", Category::Visual, 0) {
        AddSetting(Setting::FloatSetting("Smoothness", "Smoothness", 0.5f, 0.1f, 1.0f));
    }
};
