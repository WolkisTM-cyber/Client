#include <windows.h>
#include "AutoSprint.h"
#include "GUI.h"

static AutoSprint* g_autoSprint = nullptr;
static GUI* g_gui = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved) {
    switch (ulReason) {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);

        g_gui = new GUI();
        if (g_gui) {
            g_gui->Create(hModule);
        }

        g_autoSprint = new AutoSprint();
        if (g_autoSprint) {
            g_autoSprint->Start(g_gui);
        }
        break;
    }
    case DLL_PROCESS_DETACH: {
        if (g_autoSprint) {
            g_autoSprint->Stop();
            delete g_autoSprint;
            g_autoSprint = nullptr;
        }
        if (g_gui) {
            g_gui->Destroy();
            delete g_gui;
            g_gui = nullptr;
        }
        break;
    }
    }
    return TRUE;
}
