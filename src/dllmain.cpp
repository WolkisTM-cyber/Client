#include <windows.h>
#include <atomic>
#include <jni.h>
#include "GUI.h"
#include "modules/ModuleManager.h"
#include "modules/JNIHelper.h"
#include "config/ConfigManager.h"
#include "render/Renderer.h"
#include "command/CommandManager.h"

// Movement
#include "modules/movement/AutoSprint.h"
#include "modules/movement/KeepSprint.h"
#include "modules/movement/Speed.h"
#include "modules/movement/Flight.h"
#include "modules/movement/NoSlowdown.h"
#include "modules/movement/Step.h"
#include "modules/movement/Glide.h"
#include "modules/movement/LongJump.h"
#include "modules/movement/NoWeb.h"
#include "modules/movement/Strafe.h"
#include "modules/movement/IceSpeed.h"

// Combat
#include "modules/combat/Velocity.h"
#include "modules/combat/MurdererFinder.h"
#include "modules/combat/KillAura.h"
#include "modules/combat/Reach.h"
#include "modules/combat/Criticals.h"
#include "modules/combat/AutoBlock.h"
#include "modules/combat/AntiKnockback.h"
#include "modules/combat/BedAura.h"

// Visual
#include "modules/visual/FullBright.h"
#include "modules/visual/NoHurtCam.h"
#include "modules/visual/NoRotate.h"
#include "modules/visual/NoBob.h"
#include "modules/visual/Tracers.h"
#include "modules/visual/ESP.h"
#include "modules/visual/ChestESP.h"
#include "modules/visual/Chams.h"
#include "modules/visual/XRay.h"
#include "modules/visual/NoCameraClip.h"
#include "modules/visual/Ambiance.h"

// Player
#include "modules/player/NoFall.h"
#include "modules/player/AntiVoid.h"
#include "modules/player/Sneak.h"
#include "modules/player/Scaffold.h"
#include "modules/player/FastPlace.h"
#include "modules/player/AutoTool.h"
#include "modules/player/AutoArmor.h"
#include "modules/player/ChestStealer.h"
#include "modules/player/InvCleaner.h"

// HUD
#include "modules/misc/HUD.h"

// Misc
#include "modules/misc/Timer.h"
#include "modules/misc/AntiBot.h"
#include "modules/misc/NoClickDelay.h"
#include "modules/misc/Spammer.h"
#include "modules/misc/Derp.h"
#include "modules/misc/AutoL.h"

// Gui
#include "gui/ClickGUI.h"

ModuleManager* g_moduleManager = nullptr;
GUI* g_gui = nullptr;
JavaVM* g_vm = nullptr;
static HANDLE g_tickThread = nullptr;
static std::atomic<bool> g_running(false);
static CommandManager* g_commandManager = nullptr;
static ClickGUI* g_clickGUI = nullptr;

void SaveConfig() {
    if (g_moduleManager) ConfigManager::Get().Save(g_moduleManager);
}

DWORD WINAPI TickThreadProc(LPVOID) {
    while (g_running.load()) {
        if (g_vm && g_moduleManager) {
            JNIEnv* env = nullptr;
            jint getEnvErr = g_vm->GetEnv((void**)&env, JNI_VERSION_1_8);
            if (getEnvErr == JNI_OK && env) {
                g_moduleManager->OnTick(env);

                // Check keybinds
                for (auto* mod : g_moduleManager->GetAll()) {
                    int key = mod->GetKey();
                    if (key && (GetAsyncKeyState(key) & 1)) {
                        mod->Toggle(env);
                    }
                }
            }
        }
        Sleep(30);
    }
    return 0;
}

DWORD WINAPI InitThreadProc(LPVOID) {
    Sleep(2000);

    typedef jint(JNICALL* JNI_GetCreatedJavaVMs_t)(JavaVM**, jsize, jsize*);
    HMODULE jvmModule = GetModuleHandleW(L"jvm.dll");
    if (!jvmModule) jvmModule = LoadLibraryW(L"jvm.dll");
    if (!jvmModule) return 0;

    auto JNI_GetCreatedJavaVMs = (JNI_GetCreatedJavaVMs_t)GetProcAddress(jvmModule, "JNI_GetCreatedJavaVMs");
    if (!JNI_GetCreatedJavaVMs) return 0;

    jsize count = 0;
    if (JNI_GetCreatedJavaVMs(&g_vm, 1, &count) != JNI_OK || count == 0) return 0;

    JNIEnv* env = nullptr;
    jint getEnvErr = g_vm->GetEnv((void**)&env, JNI_VERSION_1_8);
    if (getEnvErr != JNI_OK || !env) {
        JavaVMAttachArgs args = { JNI_VERSION_1_8, "ClientInit", nullptr };
        if (g_vm->AttachCurrentThread((void**)&env, &args) != JNI_OK) return 0;
    }

    if (!JNIHelper::Initialize(env)) {
        g_vm->DetachCurrentThread();
        return 0;
    }

    // Load config and init modules
    ConfigManager::Get().Load(g_moduleManager);
    g_moduleManager->Init(env);

    // Init renderer
    Renderer::Get().Init();

    if (getEnvErr == JNI_EDETACHED) g_vm->DetachCurrentThread();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved) {
    switch (ulReason) {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);

        g_gui = new GUI();
        if (g_gui) g_gui->Create(hModule);

        g_moduleManager = new ModuleManager();
        g_commandManager = new CommandManager();

        // Combat
        g_moduleManager->AddModule<Velocity>();
        g_moduleManager->AddModule<MurdererFinder>();
        g_moduleManager->AddModule<KillAura>();
        g_moduleManager->AddModule<Reach>();
        g_moduleManager->AddModule<Criticals>();
        g_moduleManager->AddModule<AutoBlock>();
        g_moduleManager->AddModule<AntiKnockback>();
        g_moduleManager->AddModule<BedAura>();

        // Movement
        g_moduleManager->AddModule<AutoSprintMod>();
        g_moduleManager->AddModule<KeepSprint>();
        g_moduleManager->AddModule<Speed>();
        g_moduleManager->AddModule<Flight>();
        g_moduleManager->AddModule<NoSlowdown>();
        g_moduleManager->AddModule<Step>();
        g_moduleManager->AddModule<Glide>();
        g_moduleManager->AddModule<LongJump>();
        g_moduleManager->AddModule<NoWeb>();
        g_moduleManager->AddModule<Strafe>();
        g_moduleManager->AddModule<IceSpeed>();

        // Visual
        g_moduleManager->AddModule<FullBright>();
        g_moduleManager->AddModule<NoHurtCam>();
        g_moduleManager->AddModule<NoRotate>();
        g_moduleManager->AddModule<NoBob>();
        g_moduleManager->AddModule<Tracers>();
        g_moduleManager->AddModule<ESP>();
        g_moduleManager->AddModule<ChestESP>();
        g_moduleManager->AddModule<Chams>();
        g_moduleManager->AddModule<XRay>();
        g_moduleManager->AddModule<NoCameraClip>();
        g_moduleManager->AddModule<Ambiance>();

        // HUD
        g_moduleManager->AddModule<HUD>();

        // Player
        g_moduleManager->AddModule<NoFall>();
        g_moduleManager->AddModule<AntiVoid>();
        g_moduleManager->AddModule<Sneak>();
        g_moduleManager->AddModule<Scaffold>();
        g_moduleManager->AddModule<FastPlace>();
        g_moduleManager->AddModule<AutoTool>();
        g_moduleManager->AddModule<AutoArmor>();
        g_moduleManager->AddModule<ChestStealer>();
        g_moduleManager->AddModule<InvCleaner>();

        // Misc
        g_moduleManager->AddModule<Timer>();
        g_moduleManager->AddModule<AntiBot>();
        g_moduleManager->AddModule<NoClickDelay>();
        g_moduleManager->AddModule<Spammer>();
        g_moduleManager->AddModule<Derp>();
        g_moduleManager->AddModule<AutoL>();

        // ClickGUI
        g_clickGUI = g_moduleManager->AddModule<ClickGUI>();

        g_running.store(true);
        g_tickThread = CreateThread(nullptr, 0, TickThreadProc, nullptr, 0, nullptr);

        HANDLE initThread = CreateThread(nullptr, 0, InitThreadProc, nullptr, 0, nullptr);
        if (initThread) CloseHandle(initThread);

        break;
    }
    case DLL_PROCESS_DETACH: {
        SaveConfig();

        g_running.store(false);
        if (g_tickThread) {
            WaitForSingleObject(g_tickThread, 1500);
            CloseHandle(g_tickThread);
            g_tickThread = nullptr;
        }

        Renderer::Get().Shutdown();

        if (g_vm) {
            JNIEnv* env = nullptr;
            if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_8) == JNI_OK && env) {
                JNIHelper::Cleanup(env);
            }
        }

        delete g_commandManager;
        g_commandManager = nullptr;
        delete g_moduleManager;
        g_moduleManager = nullptr;

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
