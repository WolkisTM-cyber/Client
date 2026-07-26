#include "ClickGUI.h"
#include "../modules/ModuleManager.h"
#include <GL/gl.h>

extern ModuleManager* g_moduleManager;

void ClickGUI::OnEnable(JNIEnv* env) {
    open_.store(true);
    std::lock_guard<std::mutex> lock(panelMutex_);
    panels_.clear();

    int x = 20;
    int catCount = (int)Category::COUNT;
    for (int i = 0; i < catCount; i++) {
        Category cat = (Category)i;
        Panel p;
        p.category = cat;
        p.x = x;
        p.y = 20;
        p.w = 135;
        p.h = 24;
        p.listH = 0;
        p.expandedMod = nullptr;
        panels_.push_back(p);
        x += 145;
    }

    dragging_ = false;
    dragPanel_ = nullptr;
}

void ClickGUI::OnDisable(JNIEnv* env) {
    open_.store(false);
    std::lock_guard<std::mutex> lock(panelMutex_);
    panels_.clear();
}

void ClickGUI::OnTick(JNIEnv* env) {
    if (!open_.load()) return;

    if (GetAsyncKeyState(VK_RSHIFT) & 1) {
        Toggle(env);
        return;
    }

    std::lock_guard<std::mutex> lock(panelMutex_);

    POINT pt;
    GetCursorPos(&pt);

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        if (!dragging_) {
            for (auto& panel : panels_) {
                if (pt.x >= panel.x && pt.x <= panel.x + panel.w &&
                    pt.y >= panel.y && pt.y <= panel.y + 22) {
                    dragging_ = true;
                    dragPanel_ = &panel;
                    dragOffX_ = pt.x - panel.x;
                    dragOffY_ = pt.y - panel.y;
                    break;
                }
            }
        }

        if (dragging_ && dragPanel_) {
            dragPanel_->x = pt.x - dragOffX_;
            dragPanel_->y = pt.y - dragOffY_;
        }
    } else {
        if (!dragging_) {
            for (auto& panel : panels_) {
                int yOff = pt.y - panel.y - 24;
                if (pt.x >= panel.x && pt.x <= panel.x + panel.w && yOff >= 0) {
                    int idx = yOff / 16;
                    auto mods = g_moduleManager->GetByCategory(panel.category);
                    if (idx >= 0 && idx < (int)mods.size()) {
                        mods[idx]->Toggle(env);
                    }
                }
            }
        }
        dragging_ = false;
        dragPanel_ = nullptr;
    }
}

void ClickGUI::DrawRect(int x, int y, int w, int h, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
}

void ClickGUI::DrawOutline(int x, int y, int w, int h, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
}

void ClickGUI::Render(JNIEnv* env, jobject fontRenderer, jmethodID drawString) {
    if (!open_.load()) return;

    std::lock_guard<std::mutex> lock(panelMutex_);

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& panel : panels_) {
        DrawPanel(env, fontRenderer, drawString, panel);
    }

    glPopAttrib();
}

void ClickGUI::DrawPanel(JNIEnv* env, jobject fontRenderer, jmethodID drawString, Panel& panel) {
    // Premium Sleek Dark Header (#181818 with Violet Accent Bar)
    DrawRect(panel.x, panel.y, panel.w, 22, 0.09f, 0.09f, 0.11f, 0.95f);
    DrawRect(panel.x, panel.y, panel.w, 2, 0.42f, 0.36f, 0.90f, 1.0f); // Top Violet Accent Line
    DrawOutline(panel.x, panel.y, panel.w, 22, 0.20f, 0.20f, 0.25f, 0.8f);

    jstring name = env->NewStringUTF(CategoryName(panel.category));
    if (name && drawString) {
        env->CallIntMethod(fontRenderer, drawString, name, panel.x + 8, panel.y + 6, 0xF1F2F6);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(name);
    }

    // Module Body (#121212 Dark Card)
    auto mods = g_moduleManager->GetByCategory(panel.category);
    int listY = panel.y + 24;
    int listH = (int)mods.size() * 16 + 6;
    panel.listH = listH;

    DrawRect(panel.x, listY, panel.w, listH, 0.07f, 0.07f, 0.08f, 0.92f);
    DrawOutline(panel.x, listY, panel.w, listH, 0.15f, 0.15f, 0.18f, 0.6f);

    int y = listY + 3;
    for (auto* mod : mods) {
        if (mod->IsEnabled()) {
            // Sleek Cyan Active Bar & Pill (#00CEC9)
            DrawRect(panel.x + 2, y + 1, panel.w - 4, 14, 0.0f, 0.80f, 0.78f, 0.22f);
            DrawRect(panel.x + 2, y + 1, 3, 14, 0.0f, 0.80f, 0.78f, 1.0f);
        }

        int textColor = mod->IsEnabled() ? 0x00CEC9 : 0x8C92AC;
        jstring modName = env->NewStringUTF(mod->GetDisplayName().c_str());
        if (modName && drawString) {
            env->CallIntMethod(fontRenderer, drawString, modName, panel.x + 8, y + 3, textColor);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(modName);
        }
        y += 16;
    }

    panel.h = 24 + listH;
}

int ClickGUI::GetCategoryColor(Category cat) {
    return 0x6C5CE7;
}

