#include "ClickGUI.h"
#include "../modules/ModuleManager.h"
#include <GL/gl.h>

extern ModuleManager* g_moduleManager;

void ClickGUI::OnEnable(JNIEnv* env) {
    open_.store(true);
    std::lock_guard<std::mutex> lock(panelMutex_);
    panels_.clear();

    int x = 10;
    int catCount = (int)Category::COUNT;
    for (int i = 0; i < catCount; i++) {
        Category cat = (Category)i;
        Panel p;
        p.category = cat;
        p.x = x;
        p.y = 10;
        p.w = 120;
        p.h = 20;
        p.listH = 0;
        panels_.push_back(p);
        x += 130;
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

    // Toggle on RSHIFT press
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
                    pt.y >= panel.y && pt.y <= panel.y + 18) {
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
            // Module toggle click
            for (auto& panel : panels_) {
                int yOff = pt.y - panel.y - 20;
                if (pt.x >= panel.x && pt.x <= panel.x + panel.w && yOff >= 0) {
                    int idx = yOff / 12;
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
    int color = GetCategoryColor(panel.category);

    // Header
    glColor4f(
        ((color >> 16) & 0xFF) / 255.0f,
        ((color >> 8) & 0xFF) / 255.0f,
        (color & 0xFF) / 255.0f,
        0.85f);
    glBegin(GL_QUADS);
    glVertex2i(panel.x, panel.y);
    glVertex2i(panel.x + panel.w, panel.y);
    glVertex2i(panel.x + panel.w, panel.y + 18);
    glVertex2i(panel.x, panel.y + 18);
    glEnd();

    jstring name = env->NewStringUTF(CategoryName(panel.category));
    if (name && drawString) {
        env->CallIntMethod(fontRenderer, drawString, name, panel.x + 4, panel.y + 4, 0xFFFFFF);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(name);
    }

    // Module list
    auto mods = g_moduleManager->GetByCategory(panel.category);
    int listY = panel.y + 20;
    int listH = (int)mods.size() * 12 + 4;
    panel.listH = listH;

    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2i(panel.x, listY);
    glVertex2i(panel.x + panel.w, listY);
    glVertex2i(panel.x + panel.w, listY + listH);
    glVertex2i(panel.x, listY + listH);
    glEnd();

    int y = listY + 2;
    for (auto* mod : mods) {
        int textColor = mod->IsEnabled() ? 0x55FF55 : 0x777777;
        jstring modName = env->NewStringUTF(mod->GetDisplayName().c_str());
        if (modName && drawString) {
            env->CallIntMethod(fontRenderer, drawString, modName, panel.x + 4, y, textColor);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(modName);
        }
        y += 12;
    }

    panel.h = 20 + listH;
}

int ClickGUI::GetCategoryColor(Category cat) {
    switch (cat) {
    case Category::Combat: return 0xCC3333;
    case Category::Movement: return 0x33CC33;
    case Category::Visual: return 0x3333CC;
    case Category::Player: return 0xCCCC33;
    case Category::Misc: return 0x9933CC;
    default: return 0x666666;
    }
}
