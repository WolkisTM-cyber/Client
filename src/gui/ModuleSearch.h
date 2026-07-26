#pragma once
#include "../modules/Module.h"
#include "../modules/ModuleManager.h"
#include "../render/Renderer.h"
#include <GL/gl.h>
#include <string>
#include <vector>
#include <algorithm>

extern ModuleManager* g_moduleManager;

class ModuleSearch {
public:
    ModuleSearch() {
        query_ = "";
    }

    void Toggle() { open_ = !open_; if (open_) query_ = ""; }
    bool IsOpen() { return open_; }

    void OnChar(char c) {
        if (!open_) return;
        if (c == 8 && !query_.empty()) query_.pop_back();
        else if (c >= 32 && c <= 126) query_ += c;
    }

    void Render(JNIEnv* env, jobject fontRenderer, jmethodID drawString) {
        if (!open_) return;

        auto* renderer = Renderer::GetInstance();
        RECT r; GetClientRect(GetDesktopWindow(), &r);
        int sw = r.right, sh = r.bottom;

        renderer->Setup2DProjection();

        int boxW = 200;
        int boxH = 200;
        int boxX = sw / 2 - boxW / 2;
        int boxY = sh / 2 - boxH / 2;

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);

        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + boxH);
        glVertex2i(boxX, boxY + boxH);
        glEnd();

        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + boxH);
        glVertex2i(boxX, boxY + boxH);
        glEnd();

        // Search query
        std::string display = "> " + query_ + "_";
        jstring queryStr = env->NewStringUTF(display.c_str());
        if (queryStr && drawString) {
            env->CallIntMethod(fontRenderer, drawString, queryStr, boxX + 5, boxY + 5, 0xFFFFFF);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(queryStr);
        }

        // Results
        if (!query_.empty()) {
            std::string q = query_;
            std::transform(q.begin(), q.end(), q.begin(), ::tolower);

            auto allMods = g_moduleManager->GetAll();
            std::vector<Module*> results;
            for (auto* mod : allMods) {
                std::string name = mod->GetName();
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                if (name.find(q) != std::string::npos) {
                    results.push_back(mod);
                }
            }

            int y = boxY + 20;
            for (size_t i = 0; i < results.size() && i < 12; i++) {
                jstring text = env->NewStringUTF(results[i]->GetDisplayName().c_str());
                if (text && drawString) {
                    env->CallIntMethod(fontRenderer, drawString, text,
                        boxX + 5, y,
                        results[i]->IsEnabled() ? 0x55FF55 : 0xFF5555);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                    env->DeleteLocalRef(text);
                }
                y += 12;
            }
        }

        // Enter toggles first result
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
            if (!query_.empty()) {
                std::string q = query_;
                std::transform(q.begin(), q.end(), q.begin(), ::tolower);
                auto allMods = g_moduleManager->GetAll();
                for (auto* mod : allMods) {
                    std::string name = mod->GetName();
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    if (name.find(q) != std::string::npos) {
                        mod->Toggle(nullptr);
                        break;
                    }
                }
            }
            open_ = false;
            Sleep(150); // debounce
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            open_ = false;
            Sleep(100);
        }

        glPopAttrib();
        renderer->RestoreProjection();
    }

private:
    bool open_ = false;
    std::string query_;
};
