#include "Renderer.h"
#include "../modules/ModuleManager.h"
#include "../modules/JNIHelper.h"
#include "../gui/ClickGUI.h"
#include "../gui/TabGUI.h"
#include "../gui/ModuleSearch.h"
#include "../modules/visual/TargetHUD.h"
#include "../modules/visual/ArmorHUD.h"
#include "../modules/visual/ItemESP.h"
#include "../modules/visual/NameTags.h"
#include "../modules/visual/Crosshair.h"
#include "../modules/visual/BlockOutline.h"
#include "../modules/visual/BreakProgress.h"
#include "../modules/visual/Trail.h"
#include "../modules/visual/PotionEffects.h"
#include "../modules/visual/Capes.h"
#include "ShaderEngine.h"
#include "../modules/world/PlayerRadar.h"
#include "../modules/world/Waypoints.h"
#include "../modules/world/ChunkBorders.h"
#include "../modules/world/NewChunks.h"
#include "../modules/world/CaveFinder.h"
#include "../modules/misc/Notifications.h"
#include "../modules/misc/CPSCounter.h"
#include "../modules/misc/Keystrokes.h"
#include "../modules/misc/ServerInfo.h"
#include "../modules/visual/MotionBlur.h"
#include "../modules/quality/HUDEditor.h"
#include <cmath>
#include <cstdio>
#include <GL/gl.h>

#pragma comment(lib, "opengl32.lib")

extern ModuleManager* g_moduleManager;
extern JavaVM* g_vm;
extern ClickGUI* g_clickGUI;
extern TabGUI* g_tabGUI;
extern ModuleSearch* g_moduleSearch;
extern TargetHUD* g_targetHUD;

// Trampoline: 14 bytes original + 14-byte JMP back
static BYTE g_originalBytes[14];
static BYTE g_trampoline[28];
static bool g_hookInstalled = false;
typedef BOOL(WINAPI* wglSwapBuffers_t)(HDC);
static wglSwapBuffers_t g_original = nullptr;

static BOOL WINAPI wglSwapBuffers_hook(HDC hdc) {
    if (g_vm && g_moduleManager) {
        JNIEnv* env = nullptr;
        if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_8) == JNI_OK && env) {
            Renderer::Get().OnSwapBuffers(hdc);
        }
    }
    // Call trampoline (original + JMP back)
    return ((wglSwapBuffers_t)(void*)g_trampoline)(hdc);
}

Renderer& Renderer::Get() {
    static Renderer instance;
    return instance;
}

bool Renderer::Init() {
    if (initialized_) return true;

    HMODULE glModule = GetModuleHandleW(L"opengl32.dll");
    if (!glModule) return false;

    void* target = GetProcAddress(glModule, "wglSwapBuffers");
    if (!target) return false;

    g_original = (wglSwapBuffers_t)target;

    DWORD old;
    VirtualProtect(target, 14, PAGE_EXECUTE_READWRITE, &old);

    // Save original 14 bytes
    memcpy(g_originalBytes, target, 14);

    // Build trampoline: original bytes + JMP back to target+14
    memcpy(g_trampoline, g_originalBytes, 14);
    // JMP [RIP+0] back to original+14
    g_trampoline[14] = 0xFF;
    g_trampoline[15] = 0x25;
    *(uintptr_t*)&g_trampoline[16] = 0; // placeholder
    *(uintptr_t*)&g_trampoline[16] = (uintptr_t)target + 14;
    DWORD tmp;
    VirtualProtect(g_trampoline, sizeof(g_trampoline), PAGE_EXECUTE_READWRITE, &tmp);

    // Write JMP [RIP+0] to our hook
    BYTE jmp[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    memcpy(target, jmp, 6);
    *(uintptr_t*)((uintptr_t)target + 6) = (uintptr_t)wglSwapBuffers_hook;

    VirtualProtect(target, 14, old, &old);
    g_hookInstalled = true;
    initialized_ = true;
    return true;
}

void Renderer::Shutdown() {
    if (!g_hookInstalled || !g_original) return;

    HMODULE glModule = GetModuleHandleW(L"opengl32.dll");
    if (!glModule) return;

    void* target = GetProcAddress(glModule, "wglSwapBuffers");
    if (!target) return;

    DWORD old;
    VirtualProtect(target, 14, PAGE_EXECUTE_READWRITE, &old);
    memcpy(target, g_originalBytes, 14);
    VirtualProtect(target, 14, old, &old);
    g_hookInstalled = false;
}

void Renderer::Setup3DProjection() {
    glGetIntegerv(GL_VIEWPORT, saved_.viewport);
    glGetFloatv(GL_PROJECTION_MATRIX, saved_.proj);
    glGetFloatv(GL_MODELVIEW_MATRIX, saved_.model);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(70.0, (double)saved_.viewport[2] / saved_.viewport[3], 0.1, 256.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void Renderer::Setup2DProjection() {
    glGetIntegerv(GL_VIEWPORT, saved_.viewport);
    glGetFloatv(GL_PROJECTION_MATRIX, saved_.proj);
    glGetFloatv(GL_MODELVIEW_MATRIX, saved_.model);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, saved_.viewport[2], saved_.viewport[3], 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void Renderer::RestoreProjection() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void Renderer::OnSwapBuffers(HDC hdc) {
    if (!g_vm || !g_moduleManager) return;

    JNIEnv* env = nullptr;
    if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_8) != JNI_OK) return;
    if (!env) return;

    // MotionBlur before ESP
    auto* motionBlur = g_moduleManager->Find("MotionBlur");
    if (motionBlur && motionBlur->IsEnabled()) {
        ((MotionBlur*)motionBlur)->OnSwapBuffers();
    }

    RenderESP(env);
    RenderTracers(env);
    RenderClickGUI(env);
    RenderHUD(env);
    Render3D(env);
    Render2D(env);
}

void Renderer::RenderESP(JNIEnv* env) {
    auto* esp = g_moduleManager->Find("ESP");
    if (!esp || !esp->IsEnabled()) return;

    auto player = JNIHelper::GetPlayer(env);
    if (!player) return;
    auto world = JNIHelper::GetWorld(env);
    if (!world) { env->DeleteLocalRef(player); return; }

    auto& c = JNIHelper::Get();

    jobject entityList = env->CallObjectMethod(world, c.getLoadedEntityList);
    if (!entityList || env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
        return;
    }

    double px = env->GetDoubleField(player, c.posX);
    double py = env->GetDoubleField(player, c.posY);
    double pz = env->GetDoubleField(player, c.posZ);

    jint size = env->CallIntMethod(entityList, c.listSize);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(entityList); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

    Setup3DProjection();

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);

    for (int i = 0; i < size && i < 200; i++) {
        jobject entity = env->CallObjectMethod(entityList, c.listGet, i);
        if (!entity || env->ExceptionCheck()) {
            if (entity) env->DeleteLocalRef(entity);
            env->ExceptionClear();
            continue;
        }
        if (env->IsSameObject(entity, player)) { env->DeleteLocalRef(entity); continue; }

        double ex = env->GetDoubleField(entity, c.posX);
        double ey = env->GetDoubleField(entity, c.posY);
        double ez = env->GetDoubleField(entity, c.posZ);

        if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(entity); break; }

        // Box in world coordinates
        glColor4f(1.0f, 0.2f, 0.2f, 0.8f);
        glBegin(GL_LINE_LOOP);
        glVertex3d(ex - 0.3, ey, ez - 0.3);
        glVertex3d(ex + 0.3, ey, ez - 0.3);
        glVertex3d(ex + 0.3, ey, ez + 0.3);
        glVertex3d(ex - 0.3, ey, ez + 0.3);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3d(ex - 0.3, ey + 1.8, ez - 0.3);
        glVertex3d(ex + 0.3, ey + 1.8, ez - 0.3);
        glVertex3d(ex + 0.3, ey + 1.8, ez + 0.3);
        glVertex3d(ex - 0.3, ey + 1.8, ez + 0.3);
        glEnd();

        glBegin(GL_LINES);
        glVertex3d(ex - 0.3, ey, ez - 0.3); glVertex3d(ex - 0.3, ey + 1.8, ez - 0.3);
        glVertex3d(ex + 0.3, ey, ez - 0.3); glVertex3d(ex + 0.3, ey + 1.8, ez - 0.3);
        glVertex3d(ex + 0.3, ey, ez + 0.3); glVertex3d(ex + 0.3, ey + 1.8, ez + 0.3);
        glVertex3d(ex - 0.3, ey, ez + 0.3); glVertex3d(ex - 0.3, ey + 1.8, ez + 0.3);
        glEnd();

        env->DeleteLocalRef(entity);
    }

    glPopAttrib();
    RestoreProjection();

    env->DeleteLocalRef(entityList);
    env->DeleteLocalRef(world);
    env->DeleteLocalRef(player);
}

void Renderer::RenderTracers(JNIEnv* env) {
    auto* tracers = g_moduleManager->Find("Tracers");
    if (!tracers || !tracers->IsEnabled()) return;

    auto player = JNIHelper::GetPlayer(env);
    if (!player) return;
    auto world = JNIHelper::GetWorld(env);
    if (!world) { env->DeleteLocalRef(player); return; }

    auto& c = JNIHelper::Get();
    jobject entityList = env->CallObjectMethod(world, c.getLoadedEntityList);
    if (!entityList || env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
        return;
    }

    double px = env->GetDoubleField(player, c.posX);
    double py = env->GetDoubleField(player, c.posY);
    double pz = env->GetDoubleField(player, c.posZ);

    jint size = env->CallIntMethod(entityList, c.listSize);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(entityList); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

    Setup3DProjection();

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.5f);

    for (int i = 0; i < size && i < 200; i++) {
        jobject entity = env->CallObjectMethod(entityList, c.listGet, i);
        if (!entity || env->ExceptionCheck()) {
            if (entity) env->DeleteLocalRef(entity);
            env->ExceptionClear();
            continue;
        }
        if (env->IsSameObject(entity, player)) { env->DeleteLocalRef(entity); continue; }

        double ex = env->GetDoubleField(entity, c.posX);
        double ey = env->GetDoubleField(entity, c.posY);
        double ez = env->GetDoubleField(entity, c.posZ);

        if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(entity); break; }

        glBegin(GL_LINES);
        glColor4f(0.0f, 1.0f, 0.0f, 0.6f);
        glVertex3d(px, py + 1.0, pz);
        glColor4f(1.0f, 0.0f, 0.0f, 0.6f);
        glVertex3d(ex, ey + 1.0, ez);
        glEnd();

        env->DeleteLocalRef(entity);
    }

    glPopAttrib();
    RestoreProjection();

    env->DeleteLocalRef(entityList);
    env->DeleteLocalRef(world);
    env->DeleteLocalRef(player);
}

void Renderer::RenderClickGUI(JNIEnv* env) {
    if (!g_clickGUI || !g_clickGUI->IsOpen()) return;

    auto mc = JNIHelper::GetMinecraft(env);
    if (!mc) return;

    auto& c = JNIHelper::Get();
    jfieldID fontRendererField = env->GetFieldID(c.minecraft, "fontRendererObj",
        "Lnet/minecraft/client/gui/FontRenderer;");
    if (!fontRendererField || env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(mc);
        return;
    }

    jobject fontRenderer = env->GetObjectField(mc, fontRendererField);
    if (!fontRenderer) { env->DeleteLocalRef(mc); return; }

    jmethodID drawString = env->GetMethodID(
        env->GetObjectClass(fontRenderer), "drawString",
        "(Ljava/lang/String;III)I");
    if (!drawString || env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(fontRenderer);
        env->DeleteLocalRef(mc);
        return;
    }

    Setup2DProjection();
    g_clickGUI->Render(env, fontRenderer, drawString);
    RestoreProjection();

    env->DeleteLocalRef(fontRenderer);
    env->DeleteLocalRef(mc);
}

void Renderer::Render3D(JNIEnv* env) {
    // ItemESP
    auto* itemESP = g_moduleManager->Find("ItemESP");
    if (itemESP && itemESP->IsEnabled()) {
        ((ItemESP*)itemESP)->Render3D();
    }

    // NameTags
    auto* nameTags = g_moduleManager->Find("NameTags");
    if (nameTags && nameTags->IsEnabled()) {
        ((NameTags*)nameTags)->Render3D(env);
    }

    // Waypoints
    auto* waypoints = g_moduleManager->Find("Waypoints");
    if (waypoints && waypoints->IsEnabled()) {
        ((Waypoints*)waypoints)->Render3D(env);
    }

    // ChunkBorders
    auto* chunks = g_moduleManager->Find("ChunkBorders");
    if (chunks && chunks->IsEnabled()) {
        ((ChunkBorders*)chunks)->Render3D(env);
    }

    // BreakProgress
    auto* breakP = g_moduleManager->Find("BreakProgress");
    if (breakP && breakP->IsEnabled()) {
        ((BreakProgress*)breakP)->OnSwapBuffers(env);
    }

    // BlockOutline
    auto* blockO = g_moduleManager->Find("BlockOutline");
    if (blockO && blockO->IsEnabled()) {
        ((BlockOutline*)blockO)->OnSwapBuffers(env);
    }

    // Trail
    auto* trail = g_moduleManager->Find("Trail");
    if (trail && trail->IsEnabled()) {
        ((Trail*)trail)->Render3D();
    }

    // NewChunks
    auto* newChunks = g_moduleManager->Find("NewChunks");
    if (newChunks && newChunks->IsEnabled()) {
        ((NewChunks*)newChunks)->Render3D();
    }

    // CaveFinder
    auto* caveFinder = g_moduleManager->Find("CaveFinder");
    if (caveFinder && caveFinder->IsEnabled()) {
        ((CaveFinder*)caveFinder)->Render3D();
    }

    // Capes
    auto* capes = g_moduleManager->Find("Capes");
    if (capes && capes->IsEnabled()) {
        ((Capes*)capes)->Render3D(env);
    }
}

void Renderer::Render2D(JNIEnv* env) {
    // Crosshair
    auto* crosshair = g_moduleManager->Find("Crosshair");
    if (crosshair && crosshair->IsEnabled()) {
        ((Crosshair*)crosshair)->Render2D();
    }

    // PlayerRadar
    auto* radar = g_moduleManager->Find("PlayerRadar");
    if (radar && radar->IsEnabled()) {
        ((PlayerRadar*)radar)->Render2D(env);
    }

    // TabGUI
    if (g_tabGUI && g_tabGUI->IsOpen()) {
        auto mc = JNIHelper::GetMinecraft(env);
        if (mc) {
            auto& c = JNIHelper::Get();
            jfieldID fontField = env->GetFieldID(c.minecraft, "fontRendererObj",
                "Lnet/minecraft/client/gui/FontRenderer;");
            if (fontField) {
                jobject fr = env->GetObjectField(mc, fontField);
                if (fr) {
                    jmethodID drawStr = env->GetMethodID(
                        env->GetObjectClass(fr), "drawString",
                        "(Ljava/lang/String;III)I");
                    if (drawStr) {
                        g_tabGUI->Render(env, fr, drawStr);
                    }
                    env->DeleteLocalRef(fr);
                }
            }
            env->DeleteLocalRef(mc);
        }
    }

    // ModuleSearch
    if (g_moduleSearch && g_moduleSearch->IsOpen()) {
        auto mc = JNIHelper::GetMinecraft(env);
        if (mc) {
            auto& c = JNIHelper::Get();
            jfieldID fontField = env->GetFieldID(c.minecraft, "fontRendererObj",
                "Lnet/minecraft/client/gui/FontRenderer;");
            if (fontField) {
                jobject fr = env->GetObjectField(mc, fontField);
                if (fr) {
                    jmethodID drawStr = env->GetMethodID(
                        env->GetObjectClass(fr), "drawString",
                        "(Ljava/lang/String;III)I");
                    if (drawStr) {
                        g_moduleSearch->Render(env, fr, drawStr);
                    }
                    env->DeleteLocalRef(fr);
                }
            }
            env->DeleteLocalRef(mc);
        }
    }

    // HUDEditor
    auto* hudEditor = g_moduleManager->Find("HUDEditor");
    if (hudEditor && hudEditor->IsEnabled()) {
        auto mc = JNIHelper::GetMinecraft(env);
        if (mc) {
            auto& c = JNIHelper::Get();
            jfieldID fontField = env->GetFieldID(c.minecraft, "fontRendererObj",
                "Lnet/minecraft/client/gui/FontRenderer;");
            if (fontField) {
                jobject fr = env->GetObjectField(mc, fontField);
                if (fr) {
                    jmethodID drawStr = env->GetMethodID(
                        env->GetObjectClass(fr), "drawString",
                        "(Ljava/lang/String;III)I");
                    if (drawStr) {
                        ((HUDEditor*)hudEditor)->Render(env, fr, drawStr);
                    }
                    env->DeleteLocalRef(fr);
                }
            }
            env->DeleteLocalRef(mc);
        }
    }
}

void Renderer::RenderHUD(JNIEnv* env) {
    auto mc = JNIHelper::GetMinecraft(env);
    if (!mc) return;

    auto& c = JNIHelper::Get();

    // Cache fontRenderer IDs
    static jfieldID fontField = nullptr;
    static jmethodID drawStr = nullptr;
    static bool cached = false;

    if (!cached) {
        fontField = env->GetFieldID(c.minecraft, "fontRendererObj",
            "Lnet/minecraft/client/gui/FontRenderer;");
        if (fontField && !env->ExceptionCheck()) {
            jclass frClass = env->FindClass("net/minecraft/client/gui/FontRenderer");
            if (frClass) {
                drawStr = env->GetMethodID(frClass, "drawString",
                    "(Ljava/lang/String;III)I");
                env->DeleteLocalRef(frClass);
                if (drawStr && !env->ExceptionCheck()) cached = true;
            }
        }
        if (env->ExceptionCheck()) env->ExceptionClear();
    }

    if (!cached) { env->DeleteLocalRef(mc); return; }

    jobject fontRenderer = env->GetObjectField(mc, fontField);
    if (!fontRenderer) { env->DeleteLocalRef(mc); return; }

    // HUD module: module list + coordinates
    auto* hudMod = g_moduleManager->Find("HUD");
    if (hudMod && hudMod->IsEnabled()) {
        auto player = JNIHelper::GetPlayer(env);
        if (player) {
            double px = env->GetDoubleField(player, c.posX);
            double py = env->GetDoubleField(player, c.posY);
            double pz = env->GetDoubleField(player, c.posZ);

            int y = 4;
            auto modules = g_moduleManager->GetAll();

            for (auto* mod : modules) {
                if (!mod->IsEnabled()) continue;
                jstring text = env->NewStringUTF(mod->GetDisplayName().c_str());
                if (text) {
                    env->CallIntMethod(fontRenderer, drawStr, text, 4, y, 0x55FFFF);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                    env->DeleteLocalRef(text);
                }
                y += 10;
            }

            char buf[64];
            snprintf(buf, sizeof(buf), "XYZ: %.0f %.0f %.0f", px, py, pz);
            jstring coordText = env->NewStringUTF(buf);
            if (coordText) {
                RECT r; GetClientRect(GetDesktopWindow(), &r);
                env->CallIntMethod(fontRenderer, drawStr, coordText, 4,
                    r.bottom - 20, 0xFFFFFF);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(coordText);
            }
            env->DeleteLocalRef(player);
        }
    }

    // Notifications
    auto* notifs = g_moduleManager->Find("Notifications");
    if (notifs && notifs->IsEnabled()) {
        ((Notifications*)notifs)->Render(env, fontRenderer, drawStr);
    }

    // TargetHUD
    auto* targetHUD = g_moduleManager->Find("TargetHUD");
    if (targetHUD && targetHUD->IsEnabled()) {
        ((TargetHUD*)targetHUD)->Render(env, fontRenderer, drawStr);
    }

    // CPSCounter
    auto* cps = g_moduleManager->Find("CPSCounter");
    if (cps && cps->IsEnabled()) {
        ((CPSCounter*)cps)->Render(env, fontRenderer, drawStr);
    }

    // ArmorHUD
    auto* armor = g_moduleManager->Find("ArmorHUD");
    if (armor && armor->IsEnabled()) {
        ((ArmorHUD*)armor)->Render(env, fontRenderer, drawStr);
    }

    // Keystrokes
    auto* keys = g_moduleManager->Find("Keystrokes");
    if (keys && keys->IsEnabled()) {
        ((Keystrokes*)keys)->Render(env, fontRenderer, drawStr);
    }

    // PotionEffects
    auto* potionEff = g_moduleManager->Find("PotionEffects");
    if (potionEff && potionEff->IsEnabled()) {
        ((PotionEffects*)potionEff)->Render(env, fontRenderer, drawStr);
    }

    // ServerInfo
    auto* serverInfo = g_moduleManager->Find("ServerInfo");
    if (serverInfo && serverInfo->IsEnabled()) {
        ((ServerInfo*)serverInfo)->Render(env, fontRenderer, drawStr);
    }

    env->DeleteLocalRef(fontRenderer);
    env->DeleteLocalRef(mc);
}

