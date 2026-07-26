#pragma once
#include "../Module.h"
#include "../ModuleManager.h"
#include "../../render/Renderer.h"
#include <windows.h>

extern ModuleManager* g_moduleManager;

class SelfDestruct : public Module {
public:
    SelfDestruct() : Module("SelfDestruct", "Self Destruct", Category::Misc, 0) {}

    void OnEnable(JNIEnv* env) override {
        if (!g_moduleManager) return;

        // 1. Disable all active modules
        for (auto* mod : g_moduleManager->GetAll()) {
            if (mod != this && mod->IsEnabled()) {
                mod->Toggle(env);
            }
        }

        // 2. Unhook OpenGL wglSwapBuffers
        Renderer::Get().Shutdown();

        // 3. Reset Window Affinity (if StreamerMode was active)
        HWND hwnd = GetActiveWindow();
        if (hwnd) {
            SetWindowDisplayAffinity(hwnd, 0x00000000);
        }

        // 4. Disable SelfDestruct itself
        Toggle(env);
    }
};
