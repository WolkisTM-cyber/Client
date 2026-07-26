#pragma once
#include "../Module.h"
#include "../ModuleManager.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <shlobj.h>
#include <GL/gl.h>
#include <map>
#include <fstream>

struct DraggableHUD {
    std::string name;
    int x, y, w, h;
    bool dragging;
    int dragOffX, dragOffY;
};

class HUDEditor : public Module {
public:
    HUDEditor() : Module("HUDEditor", "HUD Editor", Category::Quality, 0) {
        AddSetting(Setting::BoolSetting("Grid", "Snap to Grid", true));
        AddSetting(Setting::IntSetting("GridSize", "Grid Size", 10, 5, 50));
    }

    void OnEnable(JNIEnv* env) override {
        LoadPositions();
    }

    void OnDisable(JNIEnv* env) override {
        SavePositions();
    }

    void Render(JNIEnv* env, jobject fr, jmethodID drawStr) {
        if (!IsEnabled()) return;

        auto* renderer = Renderer::GetInstance();
        RECT r; GetClientRect(GetDesktopWindow(), &r);
        int sw = r.right, sh = r.bottom;

        renderer->Setup2DProjection();

        // Dim background
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2i(0, 0); glVertex2i(sw, 0);
        glVertex2i(sw, sh); glVertex2i(0, sh);
        glEnd();

        // Grid
        if (GetSetting("Grid")->bVal) {
            int gs = GetSetting("GridSize")->iVal;
            glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
            glBegin(GL_LINES);
            for (int x = 0; x < sw; x += gs) {
                glVertex2i(x, 0); glVertex2i(x, sh);
            }
            for (int y = 0; y < sh; y += gs) {
                glVertex2i(0, y); glVertex2i(sw, y);
            }
            glEnd();
        }

        // Module positions
        auto modules = g_moduleManager->GetAll();
        int y = 10;
        for (auto* mod : modules) {
            if (!mod->IsEnabled()) continue;

            // Ensure entry exists
            auto it = positions_.find(mod->GetName());
            if (it == positions_.end()) {
                positions_[mod->GetName()] = {mod->GetName(), 4, y, 100, 10, false, 0, 0};
            }

            auto& hud = positions_[mod->GetName()];
            std::string display = mod->GetDisplayName();

            // Box
            glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
            glBegin(GL_QUADS);
            glVertex2i(hud.x, hud.y);
            glVertex2i(hud.x + hud.w, hud.y);
            glVertex2i(hud.x + hud.w, hud.y + hud.h);
            glVertex2i(hud.x, hud.y + hud.h);
            glEnd();

            glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
            glBegin(GL_LINE_LOOP);
            glVertex2i(hud.x, hud.y);
            glVertex2i(hud.x + hud.w, hud.y);
            glVertex2i(hud.x + hud.w, hud.y + hud.h);
            glVertex2i(hud.x, hud.y + hud.h);
            glEnd();

            jstring text = env->NewStringUTF(display.c_str());
            if (text && drawStr) {
                env->CallIntMethod(fr, drawStr, text, hud.x + 2, hud.y + 2, 0x55FFFF);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }

            y += 12;
        }

        // Mouse input
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            POINT pt; GetCursorPos(&pt);
            if (!dragging_) {
                for (auto& [name, hud] : positions_) {
                    if (pt.x >= hud.x && pt.x <= hud.x + hud.w &&
                        pt.y >= hud.y && pt.y <= hud.y + hud.h) {
                        dragging_ = true;
                        dragModule_ = name;
                        dragOffX_ = pt.x - hud.x;
                        dragOffY_ = pt.y - hud.y;
                        break;
                    }
                }
            }
        } else {
            dragging_ = false;
        }

        if (dragging_) {
            POINT pt; GetCursorPos(&pt);
            auto it = positions_.find(dragModule_);
            if (it != positions_.end()) {
                it->second.x = pt.x - dragOffX_;
                it->second.y = pt.y - dragOffY_;
            }
        }

        glPopAttrib();
        renderer->RestoreProjection();
    }

    int GetPosX(const std::string& name, int defaultX) {
        auto it = positions_.find(name);
        return it != positions_.end() ? it->second.x : defaultX;
    }

    int GetPosY(const std::string& name, int defaultY) {
        auto it = positions_.find(name);
        return it != positions_.end() ? it->second.y : defaultY;
    }

private:
    void LoadPositions() {
        positions_.clear();
        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) return;

        std::string path = std::string(appData, appData + wcslen(appData)) + "\\Client\\hudpos.txt";
        std::ifstream file(path);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            char name[64]; int x,y,w,h;
            if (sscanf_s(line.c_str(), "%63[^,],%d,%d,%d,%d",
                         name, (unsigned)sizeof(name), &x, &y, &w, &h) >= 5) {
                positions_[name] = {name, x, y, w, h, false, 0, 0};
            }
        }
    }

    void SavePositions() {
        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) return;

        std::string path = std::string(appData, appData + wcslen(appData)) + "\\Client\\hudpos.txt";
        SHCreateDirectoryExW(nullptr, std::wstring(
            path.begin(), path.end()).c_str(), nullptr);

        std::ofstream file(path);
        if (!file.is_open()) return;

        for (auto& [name, hud] : positions_) {
            file << hud.name << "," << hud.x << "," << hud.y << ","
                 << hud.w << "," << hud.h << std::endl;
        }
    }

    std::map<std::string, DraggableHUD> positions_;
    bool dragging_ = false;
    std::string dragModule_;
    int dragOffX_ = 0, dragOffY_ = 0;
};
