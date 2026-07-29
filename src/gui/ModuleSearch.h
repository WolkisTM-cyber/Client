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
        wasEnterDown_ = false;
        wasEscDown_ = false;
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

        GLint vp[4];
        glGetIntegerv(GL_VIEWPORT, vp);
        int sw = vp[2] > 0 ? vp[2] : 854;
        int sh = vp[3] > 0 ? vp[3] : 480;

        int boxW = 220;
        int boxH = 210;
        int boxX = sw / 2 - boxW / 2;
        int boxY = sh / 2 - boxH / 2;

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Background
        glColor4f(0.07f, 0.07f, 0.09f, 0.94f);
        glBegin(GL_QUADS);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + boxH);
        glVertex2i(boxX, boxY + boxH);
        glEnd();

        // Accent top line
        glColor4f(0.0f, 0.80f, 0.78f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + 2);
        glVertex2i(boxX, boxY + 2);
        glEnd();

        // Border outline
        glColor4f(0.20f, 0.20f, 0.25f, 0.8f);
        glBegin(GL_LINE_LOOP);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + boxH);
        glVertex2i(boxX, boxY + boxH);
        glEnd();

        // Search query string
        glEnable(GL_TEXTURE_2D);
        std::string display = "> " + query_ + "_";
        jstring queryStr = env->NewStringUTF(display.c_str());
        if (queryStr && drawString) {
            env->CallIntMethod(fontRenderer, drawString, queryStr, boxX + 8, boxY + 8, 0x00CEC9);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(queryStr);
        }
        glDisable(GL_TEXTURE_2D);

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

            int y = boxY + 24;
            for (size_t i = 0; i < results.size() && i < 11; i++) {
                glEnable(GL_TEXTURE_2D);
                jstring text = env->NewStringUTF(results[i]->GetDisplayName().c_str());
                if (text && drawString) {
                    env->CallIntMethod(fontRenderer, drawString, text,
                        boxX + 8, y,
                        results[i]->IsEnabled() ? 0x00CEC9 : 0x8C92AC);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                    env->DeleteLocalRef(text);
                }
                glDisable(GL_TEXTURE_2D);
                y += 15;
            }
        }

        // Edge-triggered keypress handling (no Sleep calls on render thread)
        bool isEnterDown = (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0;
        bool isEscDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;

        if (isEnterDown && !wasEnterDown_) {
            if (!query_.empty()) {
                std::string q = query_;
                std::transform(q.begin(), q.end(), q.begin(), ::tolower);
                auto allMods = g_moduleManager->GetAll();
                for (auto* mod : allMods) {
                    std::string name = mod->GetName();
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    if (name.find(q) != std::string::npos) {
                        mod->Toggle(env);
                        break;
                    }
                }
            }
            open_ = false;
        }

        if (isEscDown && !wasEscDown_) {
            open_ = false;
        }

        wasEnterDown_ = isEnterDown;
        wasEscDown_ = isEscDown;

        glPopAttrib();
    }

private:
    bool open_ = false;
    std::string query_;
    bool wasEnterDown_ = false;
    bool wasEscDown_ = false;
};
