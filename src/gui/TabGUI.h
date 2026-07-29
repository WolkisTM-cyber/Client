#pragma once
#include "../modules/Module.h"
#include "../modules/ModuleManager.h"
#include "../render/Renderer.h"
#include <GL/gl.h>
#include <vector>
#include <string>

extern ModuleManager* g_moduleManager;
extern JavaVM* g_vm;

class TabGUI {
public:
    TabGUI() {
        categories_ = {"Combat", "Movement", "Visual", "Player", "Misc", "Exploit", "World", "Quality"};
    }

    void Toggle() { open_ = !open_; }
    bool IsOpen() { return open_; }

    void Render(JNIEnv* env, jobject fontRenderer, jmethodID drawString) {
        if (!open_) return;

        GLint vp[4];
        glGetIntegerv(GL_VIEWPORT, vp);
        int sw = vp[2] > 0 ? vp[2] : 854;
        int sh = vp[3] > 0 ? vp[3] : 480;

        int boxW = 110;
        int boxH = 14 * (int)categories_.size() + 6;
        int boxX = 20;
        int boxY = 60;

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Background
        glColor4f(0.08f, 0.08f, 0.11f, 0.92f);
        glBegin(GL_QUADS);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + boxH);
        glVertex2i(boxX, boxY + boxH);
        glEnd();

        // Top Accent Line
        glColor4f(0.42f, 0.36f, 0.90f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2i(boxX, boxY);
        glVertex2i(boxX + boxW, boxY);
        glVertex2i(boxX + boxW, boxY + 2);
        glVertex2i(boxX, boxY + 2);
        glEnd();

        // Categories
        int itemY = boxY + 4;
        for (size_t i = 0; i < categories_.size(); i++) {
            bool selected = (int)i == sel_;
            if (selected) {
                glColor4f(0.0f, 0.80f, 0.78f, 0.35f);
                glBegin(GL_QUADS);
                glVertex2i(boxX + 2, itemY);
                glVertex2i(boxX + boxW - 2, itemY);
                glVertex2i(boxX + boxW - 2, itemY + 13);
                glVertex2i(boxX + 2, itemY + 13);
                glEnd();
            }

            glEnable(GL_TEXTURE_2D);
            jstring text = env->NewStringUTF(categories_[i].c_str());
            if (text && drawString) {
                env->CallIntMethod(fontRenderer, drawString, text,
                    boxX + 6, itemY + 2,
                    selected ? 0x00CEC9 : 0xF1F2F6);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(text);
            }
            glDisable(GL_TEXTURE_2D);

            itemY += 14;
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

            int subW = 125;
            int subH = 14 * (int)catMods.size() + 6;
            int subX = boxX + boxW + 4;
            int subY = boxY;

            glColor4f(0.08f, 0.08f, 0.11f, 0.92f);
            glBegin(GL_QUADS);
            glVertex2i(subX, subY);
            glVertex2i(subX + subW, subY);
            glVertex2i(subX + subW, subY + subH);
            glVertex2i(subX, subY + subH);
            glEnd();

            // Accent bar
            glColor4f(0.0f, 0.80f, 0.78f, 1.0f);
            glBegin(GL_QUADS);
            glVertex2i(subX, subY);
            glVertex2i(subX + subW, subY);
            glVertex2i(subX + subW, subY + 2);
            glVertex2i(subX, subY + 2);
            glEnd();

            int modY = subY + 4;
            for (size_t i = 0; i < catMods.size(); i++) {
                bool selected = (int)i == subSel_;
                if (selected) {
                    glColor4f(0.42f, 0.36f, 0.90f, 0.35f);
                    glBegin(GL_QUADS);
                    glVertex2i(subX + 2, modY);
                    glVertex2i(subX + subW - 2, modY);
                    glVertex2i(subX + subW - 2, modY + 13);
                    glVertex2i(subX + 2, modY + 13);
                    glEnd();
                }

                glEnable(GL_TEXTURE_2D);
                jstring text = env->NewStringUTF(catMods[i]->GetDisplayName().c_str());
                if (text && drawString) {
                    env->CallIntMethod(fontRenderer, drawString, text,
                        subX + 6, modY + 2,
                        catMods[i]->IsEnabled() ? 0x00CEC9 : (selected ? 0xFFFFFF : 0x8C92AC));
                    if (env->ExceptionCheck()) env->ExceptionClear();
                    env->DeleteLocalRef(text);
                }
                glDisable(GL_TEXTURE_2D);
                modY += 14;
            }
        }

        glPopAttrib();
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
                JNIEnv* env = nullptr;
                if (g_vm && g_vm->GetEnv((void**)&env, JNI_VERSION_1_8) == JNI_OK && env) {
                    catMods[subSel_]->Toggle(env);
                }
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
