#pragma once
#include <windows.h>
#include <vector>
#include <mutex>
#include <jni.h>

class Renderer {
public:
    static Renderer& Get();

    bool Init();
    void Shutdown();

    void OnSwapBuffers(HDC hdc);

    bool IsInitialized() const { return initialized_; }

private:
    void Setup3DProjection();
    void Setup2DProjection();
    void RestoreProjection();

    void RenderESP(JNIEnv* env);
    void RenderTracers(JNIEnv* env);
    void RenderHUD(JNIEnv* env);
    void RenderClickGUI(JNIEnv* env);

    struct {
        float proj[16];
        float model[16];
        int viewport[4];
    } saved_;

    bool initialized_;
};
