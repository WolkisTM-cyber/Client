#include "ShaderEngine.h"

bool ShaderEngine::Init() {
    supported_ = true;
    return true;
}

void ShaderEngine::Shutdown() {
    supported_ = false;
}

void ShaderEngine::BeginGlowPass() {
    if (!supported_) return;
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void ShaderEngine::EndGlowPass() {
    if (!supported_) return;
    glPopAttrib();
}
