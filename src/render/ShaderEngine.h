#pragma once
#include <windows.h>
#include <GL/gl.h>
#include <string>

class ShaderEngine {
public:
    static ShaderEngine& Get() {
        static ShaderEngine instance;
        return instance;
    }

    bool Init();
    void Shutdown();

    void BeginGlowPass();
    void EndGlowPass();

    bool IsSupported() const { return supported_; }

private:
    bool supported_ = false;
    GLuint glowFramebuffer_ = 0;
    GLuint glowTexture_ = 0;
};
