#pragma once
#include "../modules/Module.h"
#include "../modules/ModuleManager.h"
#include "../render/Renderer.h"
#include <GL/gl.h>
#include <vector>
#include <string>

extern ModuleManager* g_moduleManager;

class TabGUI {
public:
    TabGUI() {
        categories_ = {"Combat", "Movement", "Visual", "Player", "Misc", "Exploit", "World", "Quality"};
    }

    void Toggle() { open_ = !open_; }
    bool IsOpen() { return open_; }

    void Render(JNIEnv* env, jobject fontRenderer, jmethodID drawString) {
        if (!open_) return;

        auto* renderer = Renderer::GetInstance();
        RECT r; GetClientRect(GetDesktopWindow(), &r);
        int sw = r.right, sh = r.bottom;

        renderer->Setup2DProjection();

        int boxW = 100;
        int boxH = 12 * (int)categories_.size() + 4;
        int boxX = sw / 2 - boxW / 2;
        int boxY = sh / 2 - boxH / 2;

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);

        // Background
        glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + boxH);
        glVertex2i(boxX, boxY + boxH);
        glEnd();

        // Categories
        int itemY = boxY + 2;
        for (size_t i = 0; i < categories_.size(); i++) {
            bool selected = (int)i == sel_;
            if (selected) {
                glColor4f(0.3f, 0.3f, 0.8f, 0.5f);
                glBegin(GL_QUADS);
                glVertex2i(boxX, itemY);
                glVertex2i(boxX + boxW, itemY);
                glVertex2i(boxX + boxW, itemY + 12);
                glVertex2i(boxX, itemY + 12);
                glEnd();
            }

            jstring text = env->NewStringUTF(categories_[i].c_str());
            if (text && drawString) {
                env->CallIntMethod(fontRenderer, drawString, text,
                    boxX + 5, itemY + 2,
                    selected ? 0xFFFF55 : 0xFFFFFF);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }

            itemY += 12;
        }

        // Modules sub-list
        if (sel_ >= 0 && sel_ < (int)categories_.size()) {
            auto modules = g_moduleManager->GetAll();
            std::vector<Module*> catMods;
            for (auto* mod : modules) {
                if (CategoryToString(mod->GetCategory()) == categories_[sel_]) {
                    catMods.push_back(mod);
                }
            }

            int subW = 120;
            int subH = 12 * (int)catMods.size() + 4;
            int subX = boxX + boxW + 2;
            int subY = boxY;

            glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
            glBegin(GL_QUADS);
            glVertex2i(subX, subY);
            glVertex2i(subX + subW, subY);
            glVertex2i(subX + subW, subY + subH);
            glVertex2i(subX, subY + subH);
            glEnd();

            int modY = subY + 2;
            for (size_t i = 0; i < catMods.size(); i++) {
                bool selected = (int)i == subSel_;
                if (selected) {
                    glColor4f(0.3f, 0.3f, 0.8f, 0.5f);
                    glBegin(GL_QUADS);
                    glVertex2i(subX, modY);
                    glVertex2i(subX + subW, modY);
                    glVertex2i(subX + subW, modY + 12);
                    glVertex2i(subX, modY + 12);
                    glEnd();
                }

                jstring text = env->NewStringUTF(catMods[i]->GetDisplayName().c_str());
                if (text && drawString) {
                    env->CallIntMethod(fontRenderer, drawString, text,
                        subX + 5, modY + 2,
                        catMods[i]->IsEnabled() ? 0x55FF55 : 0xFF5555);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                    env->DeleteLocalRef(text);
                }
                modY += 12;
            }
        }

        glPopAttrib();
        renderer->RestoreProjection();
    }

    void OnKeyPress(int key) {
        if (!open_ && key == VK_TAB) {
            open_ = true;
            sel_ = 0;
            subSel_ = 0;
            return;
        }

        if (key == VK_TAB) { open_ = false; return; }
        if (key == VK_UP) { subSel_--; if (subSel_ < 0) subSel_ = 0; return; }
        if (key == VK_DOWN) { subSel_++; return; }
        if (key == VK_LEFT) { sel_--; if (sel_ < 0) sel_ = 0; subSel_ = 0; return; }
        if (key == VK_RIGHT) { sel_++; if (sel_ >= (int)categories_.size()) sel_ = (int)categories_.size() - 1; subSel_ = 0; return; }
        if (key == VK_RETURN) {
            auto modules = g_moduleManager->GetAll();
            std::vector<Module*> catMods;
            for (auto* mod : modules) {
                if (CategoryToString(mod->GetCategory()) == categories_[sel_]) {
                    catMods.push_back(mod);
                }
            }
            if (subSel_ >= 0 && subSel_ < (int)catMods.size()) {
                catMods[subSel_]->Toggle();
            }
        }
    }

private:
    bool open_ = false;
    int sel_ = 0;
    int subSel_ = 0;
    std::vector<std::string> categories_;

    std::string CategoryToString(Category c) {
        switch (c) {
        case Category::Combat: return "Combat";
        case Category::Movement: return "Movement";
        case Category::Visual: return "Visual";
        case Category::Player: return "Player";
        case Category::Misc: return "Misc";
        case Category::Exploit: return "Exploit";
        case Category::World: return "World";
        case Category::Quality: return "Quality";
        default: return "";
        }
    }
};
