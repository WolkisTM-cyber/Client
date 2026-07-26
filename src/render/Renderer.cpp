#include "Renderer.h"
#include "../modules/ModuleManager.h"
#include "../modules/JNIHelper.h"
#include "../gui/ClickGUI.h"
#include <cmath>
#include <cstdio>
#include <GL/gl.h>

#pragma comment(lib, "opengl32.lib")

extern ModuleManager* g_moduleManager;
extern JavaVM* g_vm;
extern ClickGUI* g_clickGUI;

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

    // Get player rotation for camera
    // Simplified: just use identity, ESP/Tracers will use world coords
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

    RenderESP(env);
    RenderTracers(env);
    RenderClickGUI(env);
    RenderHUD(env);
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

void Renderer::RenderHUD(JNIEnv* env) {
    auto* hudMod = g_moduleManager->Find("HUD");
    if (!hudMod || !hudMod->IsEnabled()) return;

    auto player = JNIHelper::GetPlayer(env);
    if (!player) return;
    auto mc = JNIHelper::GetMinecraft(env);
    if (!mc) { env->DeleteLocalRef(player); return; }

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

    if (!cached) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

    jobject fontRenderer = env->GetObjectField(mc, fontField);
    if (!fontRenderer) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

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

    env->DeleteLocalRef(fontRenderer);
    env->DeleteLocalRef(mc);
    env->DeleteLocalRef(player);
}
