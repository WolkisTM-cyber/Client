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
#include "modules/misc/AntiKnockback.h"
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

// AntiCheat
#include "modules/anticheat/HypixelNPC.h"
#include "modules/anticheat/AntiDesync.h"
#include "modules/anticheat/AntiFlag.h"
#include "modules/anticheat/StaffDetect.h"
#include "modules/anticheat/AutoGG.h"
#include "modules/anticheat/NoPitchLimit.h"

// Packet
#include "modules/packet/PacketUtil.h"

// Combat additions
#include "modules/combat/WTap.h"
#include "modules/combat/BackTrack.h"
#include "modules/combat/TriggerBot.h"
#include "modules/combat/SuperKnockback.h"
#include "modules/combat/HitBox.h"
#include "modules/combat/BowAimbot.h"
#include "modules/combat/AutoRod.h"
#include "modules/combat/AutoPot.h"
#include "modules/combat/AntiCactus.h"
#include "modules/combat/Jesus.h"
#include "modules/combat/NoPush.h"
#include "modules/combat/Zoot.h"

// Visual additions
#include "modules/visual/TargetHUD.h"
#include "modules/visual/ArmorHUD.h"
#include "modules/visual/ItemESP.h"
#include "modules/visual/NameTags.h"
#include "modules/visual/FreeCam.h"
#include "modules/visual/Crosshair.h"
#include "modules/visual/MotionBlur.h"
#include "modules/visual/NoFov.h"
#include "modules/visual/ClearWater.h"
#include "modules/visual/BlockOutline.h"
#include "modules/visual/BreakProgress.h"
#include "modules/visual/Scoreboard.h"
#include "modules/visual/BossBar.h"
#include "modules/visual/Capes.h"
#include "modules/visual/Trail.h"
#include "modules/visual/TargetESP.h"
#include "modules/visual/HitColor.h"
#include "modules/visual/ItemPhysics3D.h"
#include "modules/visual/CustomSky.h"
#include "modules/visual/SessionInfo.h"
#include "modules/visual/InventoryHUD.h"
#include "modules/visual/SmoothCam.h"
#include "modules/combat/AutoGapple.h"

// Misc additions
#include "modules/misc/Notifications.h"
#include "modules/misc/CPSCounter.h"
#include "modules/misc/Keystrokes.h"
#include "modules/misc/Friends.h"
#include "modules/misc/AutoAccept.h"
#include "modules/misc/AutoHypixel.h"
#include "modules/misc/AutoTip.h"
#include "modules/misc/BanNotifier.h"
#include "modules/misc/ServerInfo.h"
#include "modules/player/AutoQueue.h"

// Movement additions
#include "modules/movement/TargetStrafe.h"
#include "modules/movement/InventoryMove.h"
#include "modules/movement/SafeWalk.h"
#include "modules/movement/Parkour.h"
#include "modules/movement/Spider.h"
#include "modules/movement/NoJumpDelay.h"
#include "modules/movement/FastLadder.h"
#include "modules/movement/AntiHunger.h"

// Player additions
#include "modules/player/Blink.h"
#include "modules/player/SpeedMine.h"
#include "modules/player/AutoRespawn.h"
#include "modules/player/Eagle.h"
#include "modules/player/AntiAfk.h"
#include "modules/player/AutoEat.h"
#include "modules/player/AntiThrow.h"
#include "modules/player/ClickTP.h"
#include "modules/player/NoFire.h"
#include "modules/player/Nuker.h"

// Exploit
#include "modules/exploit/Disabler.h"
#include "modules/exploit/Regen.h"
#include "modules/exploit/Phase.h"
#include "modules/exploit/PacketLogger.h"
#include "modules/exploit/AntiBan.h"
#include "modules/exploit/Crash.h"
#include "modules/exploit/GodMode.h"
#include "modules/exploit/PacketCancel.h"

// World
#include "modules/world/Waypoints.h"
#include "modules/world/ChunkBorders.h"
#include "modules/world/PlayerRadar.h"
#include "modules/world/LightLevel.h"
#include "modules/world/CaveFinder.h"
#include "modules/world/Fucker.h"
#include "modules/world/GhostHand.h"
#include "modules/world/NewChunks.h"
#include "modules/world/PathFinder.h"

// Quality
#include "modules/quality/HUDEditor.h"
#include "modules/quality/Theme.h"
#include "modules/quality/Profiles.h"
#include "modules/quality/DiscordRPC.h"
#include "modules/quality/StreamerMode.h"
#include "modules/quality/AltManager.h"

// System
#include "LogSystem.h"
#include "modules/EventSystem.h"
#include "modules/OnlineConfig.h"

// Misc
#include "modules/misc/Timer.h"
#include "modules/misc/AntiBot.h"
#include "modules/misc/NoClickDelay.h"
#include "modules/misc/Spammer.h"
#include "modules/misc/Derp.h"
#include "modules/misc/AutoL.h"
#include "modules/misc/SelfDestruct.h"

// Gui
#include "gui/ClickGUI.h"
#include "gui/TabGUI.h"
#include "gui/ModuleSearch.h"

ModuleManager* g_moduleManager = nullptr;
GUI* g_gui = nullptr;
JavaVM* g_vm = nullptr;
static HANDLE g_tickThread = nullptr;
static std::atomic<bool> g_running(false);
static CommandManager* g_commandManager = nullptr;
ClickGUI* g_clickGUI = nullptr;
TabGUI* g_tabGUI = nullptr;
ModuleSearch* g_moduleSearch = nullptr;

void SaveConfig() {
    if (g_moduleManager) ConfigManager::Get().Save(g_moduleManager);
}

DWORD WINAPI TickThreadProc(LPVOID) {
    SEH_TRY {
        while (g_running.load()) {
            if (g_vm && g_moduleManager) {
                JNIEnv* env = nullptr;
                jint getEnvErr = g_vm->GetEnv((void**)&env, JNI_VERSION_1_8);
                if (getEnvErr == JNI_OK && env) {
                    SEH_TRY {
                        g_moduleManager->OnTick(env);

                        // Check keybinds
                        for (auto* mod : g_moduleManager->GetAll()) {
                            int key = mod->GetKey();
                            if (key && (GetAsyncKeyState(key) & 1)) {
                                mod->Toggle(env);
                            }
                        }

                        // TabGUI key
                        if (g_tabGUI && (GetAsyncKeyState(VK_TAB) & 1)) {
                            g_tabGUI->Toggle();
                        }

                        // ClickGUI key (Right Shift or Insert)
                        if (g_clickGUI && (GetAsyncKeyState(VK_INSERT) & 1)) {
                            g_clickGUI->Toggle(env);
                        }
                        if (g_clickGUI && (GetAsyncKeyState(VK_RSHIFT) & 1)) {
                            g_clickGUI->Toggle(env);
                        }

                        // ModuleSearch key (F6)
                        if (g_moduleSearch && (GetAsyncKeyState(VK_F6) & 1)) {
                            g_moduleSearch->Toggle();
                        }

                        // TabGUI navigation
                        if (g_tabGUI && g_tabGUI->IsOpen()) {
                            for (int vk = VK_UP; vk <= VK_RIGHT; vk++) {
                                if (GetAsyncKeyState(vk) & 1) {
                                    g_tabGUI->OnKeyPress(vk);
                                }
                            }
                            if (GetAsyncKeyState(VK_RETURN) & 1) {
                                g_tabGUI->OnKeyPress(VK_RETURN);
                            }
                        }
                    } SEH_EXCEPT("TickThreadProc iteration exception caught cleanly.")
                }
            }
            Sleep(30);
        }
    } SEH_EXCEPT("TickThreadProc fatal exception caught cleanly.")
    return 0;
}

DWORD WINAPI InitThreadProc(LPVOID) {
    SEH_TRY {
        Sleep(1500);

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
            JavaVMAttachArgs args = { JNI_VERSION_1_8, (char*)"ClientInit", nullptr };
            if (g_vm->AttachCurrentThread((void**)&env, &args) != JNI_OK) return 0;
        }

        if (!JNIHelper::Initialize(env)) {
            g_vm->DetachCurrentThread();
            return 0;
        }

        // Initialize LogSystem
        LogSystem::Get().Init();
        LogSystem::Get().WriteInfo("Client initialization starting...");

        // Load config and init modules
        ConfigManager::Get().Load(g_moduleManager);
        g_moduleManager->Init(env);

        // Init renderer
        Renderer::Get().Init();
        LogSystem::Get().WriteInfo("Client initialization complete!");

        MessageBeep(MB_OK); // Audible beep signal when Client is fully initialized

        if (getEnvErr == JNI_EDETACHED) g_vm->DetachCurrentThread();
    } SEH_EXCEPT("InitThreadProc exception caught cleanly during startup.")
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
        g_moduleManager->AddModule<WTap>();
        g_moduleManager->AddModule<BackTrack>();
        g_moduleManager->AddModule<BedAura>();
        g_moduleManager->AddModule<TriggerBot>();
        g_moduleManager->AddModule<SuperKnockback>();
        g_moduleManager->AddModule<HitBox>();
        g_moduleManager->AddModule<BowAimbot>();
        g_moduleManager->AddModule<AutoRod>();
        g_moduleManager->AddModule<AutoPot>();
        g_moduleManager->AddModule<AntiCactus>();
        g_moduleManager->AddModule<Jesus>();
        g_moduleManager->AddModule<NoPush>();
        g_moduleManager->AddModule<Zoot>();

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
        g_moduleManager->AddModule<TargetStrafe>();
        g_moduleManager->AddModule<InventoryMove>();
        g_moduleManager->AddModule<SafeWalk>();
        g_moduleManager->AddModule<Parkour>();
        g_moduleManager->AddModule<Spider>();
        g_moduleManager->AddModule<NoJumpDelay>();
        g_moduleManager->AddModule<FastLadder>();
        g_moduleManager->AddModule<AntiHunger>();

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
        g_moduleManager->AddModule<ItemESP>();
        g_moduleManager->AddModule<NameTags>();
        g_moduleManager->AddModule<FreeCam>();
        g_moduleManager->AddModule<Crosshair>();
        g_moduleManager->AddModule<MotionBlur>();
        g_moduleManager->AddModule<NoFov>();
        g_moduleManager->AddModule<ClearWater>();
        g_moduleManager->AddModule<BlockOutline>();
        g_moduleManager->AddModule<BreakProgress>();
        g_moduleManager->AddModule<Scoreboard>();
        g_moduleManager->AddModule<BossBar>();
        g_moduleManager->AddModule<Trail>();
        g_moduleManager->AddModule<Capes>();
        g_moduleManager->AddModule<TargetESP>();
        g_moduleManager->AddModule<HitColor>();
        g_moduleManager->AddModule<ItemPhysics3D>();
        g_moduleManager->AddModule<CustomSky>();
        g_moduleManager->AddModule<SessionInfo>();
        g_moduleManager->AddModule<InventoryHUD>();
        g_moduleManager->AddModule<SmoothCam>();
        g_moduleManager->AddModule<AutoGapple>();

        // HUD
        g_moduleManager->AddModule<HUD>();
        g_moduleManager->AddModule<TargetHUD>();
        g_moduleManager->AddModule<ArmorHUD>();
        g_moduleManager->AddModule<Keystrokes>();
        g_moduleManager->AddModule<CPSCounter>();

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
        g_moduleManager->AddModule<Blink>();
        g_moduleManager->AddModule<SpeedMine>();
        g_moduleManager->AddModule<AutoRespawn>();
        g_moduleManager->AddModule<Eagle>();
        g_moduleManager->AddModule<AntiAfk>();
        g_moduleManager->AddModule<AutoEat>();
        g_moduleManager->AddModule<AntiThrow>();
        g_moduleManager->AddModule<ClickTP>();
        g_moduleManager->AddModule<NoFire>();
        g_moduleManager->AddModule<Nuker>();

        // AntiCheat
        g_moduleManager->AddModule<HypixelNPC>();
        g_moduleManager->AddModule<AntiDesync>();
        g_moduleManager->AddModule<AntiFlag>();
        g_moduleManager->AddModule<StaffDetect>();
        g_moduleManager->AddModule<AutoGG>();
        g_moduleManager->AddModule<NoPitchLimit>();

        // Misc
        g_moduleManager->AddModule<Timer>();
        g_moduleManager->AddModule<AntiBot>();
        g_moduleManager->AddModule<NoClickDelay>();
        g_moduleManager->AddModule<Spammer>();
        g_moduleManager->AddModule<Derp>();
        g_moduleManager->AddModule<AutoL>();
        g_moduleManager->AddModule<AutoQueue>();
        g_moduleManager->AddModule<Notifications>();
        g_moduleManager->AddModule<Friends>();
        g_moduleManager->AddModule<AutoAccept>();
        g_moduleManager->AddModule<AutoHypixel>();
        g_moduleManager->AddModule<AutoTip>();
        g_moduleManager->AddModule<BanNotifier>();
        g_moduleManager->AddModule<ServerInfo>();
        g_moduleManager->AddModule<SelfDestruct>();

        // Exploit
        g_moduleManager->AddModule<Disabler>();
        g_moduleManager->AddModule<Regen>();
        g_moduleManager->AddModule<Phase>();
        g_moduleManager->AddModule<PacketLogger>();
        g_moduleManager->AddModule<AntiBan>();
        g_moduleManager->AddModule<Crash>();
        g_moduleManager->AddModule<GodMode>();
        g_moduleManager->AddModule<PacketCancel>();

        // World
        g_moduleManager->AddModule<Waypoints>();
        g_moduleManager->AddModule<ChunkBorders>();
        g_moduleManager->AddModule<PlayerRadar>();
        g_moduleManager->AddModule<LightLevel>();
        g_moduleManager->AddModule<CaveFinder>();
        g_moduleManager->AddModule<Fucker>();
        g_moduleManager->AddModule<GhostHand>();
        g_moduleManager->AddModule<NewChunks>();
        g_moduleManager->AddModule<PathFinder>();

        // Quality
        g_moduleManager->AddModule<HUDEditor>();
        g_moduleManager->AddModule<Theme>();
        g_moduleManager->AddModule<Profiles>();
        g_moduleManager->AddModule<DiscordRPC>();
        g_moduleManager->AddModule<StreamerMode>();
        g_moduleManager->AddModule<AltManager>();

        // ClickGUI
        g_clickGUI = g_moduleManager->AddModule<ClickGUI>();
        g_tabGUI = new TabGUI();
        g_moduleSearch = new ModuleSearch();

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

        delete g_tabGUI; g_tabGUI = nullptr;
        delete g_moduleSearch; g_moduleSearch = nullptr;
        delete g_commandManager; g_commandManager = nullptr;
        delete g_moduleManager; g_moduleManager = nullptr;

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

