#pragma once
#include <windows.h>
#include <jni.h>
#include "../modules/JNIHelper.h"

class Renderer {
public:
    static Renderer& Get();

    bool Init();
    void Shutdown();

    void OnSwapBuffers(HDC hdc);

    bool IsInitialized() const { return initialized_; }

    void AddEntityLine(double x1, double y1, double z1,
                       double x2, double y2, double z2,
                       float r, float g, float b);
    void AddEntityBox(double x, double y, double z,
                      double w, double h,
                      float r, float g, float b);

private:
    void RenderESP(JNIEnv* env);
    void RenderTracers(JNIEnv* env);
    void RenderHUD(JNIEnv* env);

    struct Line3D {
        double x1, y1, z1, x2, y2, z2;
        float r, g, b;
    };

    struct Box3D {
        double x, y, z, w, h;
        float r, g, b;
    };

    std::vector<Line3D> lines_;
    std::vector<Box3D> boxes_;
    bool initialized_;
};
