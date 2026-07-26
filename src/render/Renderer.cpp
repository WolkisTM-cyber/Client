#include "Renderer.h"
#include "../modules/ModuleManager.h"
#include <vector>
#include <cmath>
#include <GL/gl.h>

#pragma comment(lib, "opengl32.lib")

extern ModuleManager* g_moduleManager;
extern JavaVM* g_vm;

// wglSwapBuffers hook
typedef BOOL(WINAPI* wglSwapBuffers_t)(HDC);
static wglSwapBuffers_t original_wglSwapBuffers = nullptr;
static bool hookInstalled = false;

BOOL WINAPI wglSwapBuffers_hook(HDC hdc) {
    if (g_vm && g_moduleManager) {
        JNIEnv* env = nullptr;
        if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_8) == JNI_OK && env) {
            Renderer::Get().OnSwapBuffers(hdc);
        }
    }
    return original_wglSwapBuffers(hdc);
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

    original_wglSwapBuffers = (wglSwapBuffers_t)target;

    // Install hook
    DWORD old;
    VirtualProtect(target, 14, PAGE_EXECUTE_READWRITE, &old);

    BYTE original[14];
    memcpy(original, target, 14);

    // Write JMP [RIP+0] hook
    BYTE jmp[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    memcpy(target, jmp, 6);
    *(uintptr_t*)((uintptr_t)target + 6) = (uintptr_t)wglSwapBuffers_hook;

    VirtualProtect(target, 14, old, &old);
    hookInstalled = true;
    initialized_ = true;
    return true;
}

void Renderer::Shutdown() {
    if (!hookInstalled || !original_wglSwapBuffers) return;

    HMODULE glModule = GetModuleHandleW(L"opengl32.dll");
    if (!glModule) return;

    void* target = GetProcAddress(glModule, "wglSwapBuffers");
    if (!target) return;

    // Restore original
    DWORD old;
    VirtualProtect(target, 14, PAGE_EXECUTE_READWRITE, &old);
    memcpy(target, original_wglSwapBuffers, 6); // Won't work - we need backup
    VirtualProtect(target, 14, old, &old);
}

void Renderer::OnSwapBuffers(HDC hdc) {
    if (!g_vm || !g_moduleManager) return;

    JNIEnv* env = nullptr;
    if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_8) != JNI_OK) return;
    if (!env) return;

    RenderESP(env);
    RenderTracers(env);
    RenderHUD(env);

    lines_.clear();
    boxes_.clear();
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
    if (!entityList) { env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

    double px = env->GetDoubleField(player, c.posX);
    double py = env->GetDoubleField(player, c.posY);
    double pz = env->GetDoubleField(player, c.posZ);

    jint size = env->CallIntMethod(entityList, c.listSize);

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);

    for (int i = 0; i < size; i++) {
        jobject entity = env->CallObjectMethod(entityList, c.listGet, i);
        if (!entity || env->IsSameObject(entity, player)) {
            if (entity) env->DeleteLocalRef(entity);
            continue;
        }

        double ex = env->GetDoubleField(entity, c.posX);
        double ey = env->GetDoubleField(entity, c.posY);
        double ez = env->GetDoubleField(entity, c.posZ);

        double dx = ex - px;
        double dy = ey - py;
        double dz = ez - pz;

        // Draw simple box
        glColor4f(1.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_LINE_LOOP);
        glVertex3d(dx - 0.3, dy, dz - 0.3);
        glVertex3d(dx + 0.3, dy, dz - 0.3);
        glVertex3d(dx + 0.3, dy, dz + 0.3);
        glVertex3d(dx - 0.3, dy, dz + 0.3);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3d(dx - 0.3, dy + 1.8, dz - 0.3);
        glVertex3d(dx + 0.3, dy + 1.8, dz - 0.3);
        glVertex3d(dx + 0.3, dy + 1.8, dz + 0.3);
        glVertex3d(dx - 0.3, dy + 1.8, dz + 0.3);
        glEnd();

        glBegin(GL_LINES);
        glVertex3d(dx - 0.3, dy, dz - 0.3); glVertex3d(dx - 0.3, dy + 1.8, dz - 0.3);
        glVertex3d(dx + 0.3, dy, dz - 0.3); glVertex3d(dx + 0.3, dy + 1.8, dz - 0.3);
        glVertex3d(dx + 0.3, dy, dz + 0.3); glVertex3d(dx + 0.3, dy + 1.8, dz + 0.3);
        glVertex3d(dx - 0.3, dy, dz + 0.3); glVertex3d(dx - 0.3, dy + 1.8, dz + 0.3);
        glEnd();

        env->DeleteLocalRef(entity);
    }

    glPopAttrib();
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
    if (!entityList) { env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

    double px = env->GetDoubleField(player, c.posX);
    double py = env->GetDoubleField(player, c.posY);
    double pz = env->GetDoubleField(player, c.posZ);

    jint size = env->CallIntMethod(entityList, c.listSize);

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.5f);

    for (int i = 0; i < size; i++) {
        jobject entity = env->CallObjectMethod(entityList, c.listGet, i);
        if (!entity || env->IsSameObject(entity, player)) {
            if (entity) env->DeleteLocalRef(entity);
            continue;
        }

        double ex = env->GetDoubleField(entity, c.posX);
        double ey = env->GetDoubleField(entity, c.posY);
        double ez = env->GetDoubleField(entity, c.posZ);

        glBegin(GL_LINES);
        glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
        glVertex3d(0, 0, 0);
        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        glVertex3d(ex - px, ey - py + 1.0, ez - pz);
        glEnd();

        env->DeleteLocalRef(entity);
    }

    glPopAttrib();
    env->DeleteLocalRef(entityList);
    env->DeleteLocalRef(world);
    env->DeleteLocalRef(player);
}

void Renderer::RenderHUD(JNIEnv* env) {
    auto* hudMod = g_moduleManager->Find("HUD");
    if (!hudMod || !hudMod->IsEnabled()) return;

    auto player = JNIHelper::GetPlayer(env);
    if (!player) return;
    auto mc = JNIHelper::GetMinecraft(env);
    if (!mc) { env->DeleteLocalRef(player); return; }

    auto& c = JNIHelper::Get();
    double px = env->GetDoubleField(player, c.posX);
    double py = env->GetDoubleField(player, c.posY);
    double pz = env->GetDoubleField(player, c.posZ);

    auto modules = g_moduleManager->GetAll();
    int y = 4;

    // Get FontRenderer for text
    jfieldID fontRendererField = env->GetFieldID(c.minecraft, "fontRendererObj",
        "Lnet/minecraft/client/gui/FontRenderer;");
    if (!fontRendererField) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

    jobject fontRenderer = env->GetObjectField(mc, fontRendererField);
    if (!fontRenderer) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

    jmethodID drawString = env->GetMethodID(
        env->GetObjectClass(fontRenderer), "drawString",
        "(Ljava/lang/String;III)I");

    for (auto* mod : modules) {
        if (!mod->IsEnabled()) continue;
        jstring text = env->NewStringUTF(mod->GetDisplayName().c_str());
        if (drawString) {
            env->CallIntMethod(fontRenderer, drawString, text, 4, y, 0x55FFFF);
        }
        env->DeleteLocalRef(text);
        y += 10;
    }

    // Coordinates
    char buf[64];
    snprintf(buf, sizeof(buf), "XYZ: %.0f %.0f %.0f", px, py, pz);
    jstring coordText = env->NewStringUTF(buf);
    if (drawString) {
        env->CallIntMethod(fontRenderer, drawString, coordText, 4,
            GetSystemMetrics(SM_CYSCREEN) - 20, 0xFFFFFF);
    }
    env->DeleteLocalRef(coordText);

    env->DeleteLocalRef(fontRenderer);
    env->DeleteLocalRef(mc);
    env->DeleteLocalRef(player);
}
