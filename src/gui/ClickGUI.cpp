#include "ClickGUI.h"
#include "../modules/ModuleManager.h"
#include <GL/gl.h>

extern ModuleManager* g_moduleManager;

void ClickGUI::OnEnable(JNIEnv* env) {
    open_ = true;
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
        p.expanded = true;
        panels_.push_back(p);
        x += 130;
    }
}

void ClickGUI::OnDisable(JNIEnv* env) {
    open_ = false;
    panels_.clear();
}

void ClickGUI::OnTick(JNIEnv* env) {
    if (!open_) return;

    // Toggle on RSHIFT press
    if (GetAsyncKeyState(VK_RSHIFT) & 1) {
        Toggle(env);
        return;
    }

    // Mouse handling
    POINT pt;
    GetCursorPos(&pt);

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        if (!dragging_) {
            for (auto& panel : panels_) {
                if (pt.x >= panel.x && pt.x <= panel.x + panel.w &&
                    pt.y >= panel.y && pt.y <= panel.y + panel.h) {
                    dragging_ = true;
                    dragPanel_ = &panel;
                    dragX_ = pt.x - panel.x;
                    dragY_ = pt.y - panel.y;
                    break;
                }
            }
        }

        if (dragging_ && dragPanel_) {
            dragPanel_->x = pt.x - dragX_;
            dragPanel_->y = pt.y - dragY_;
        }
    } else {
        // Check for clicks on module names
        if (dragging_ && dragPanel_) {
            // Click released - check if we clicked a module
            for (auto& panel : panels_) {
                if (pt.x >= panel.x && pt.x <= panel.x + panel.w &&
                    pt.y >= panel.y && pt.y <= panel.y + panel.h) {
                    int yOff = pt.y - panel.y - 20;
                    if (yOff >= 0) {
                        int idx = yOff / 12;
                        auto mods = g_moduleManager->GetByCategory(panel.category);
                        if (idx >= 0 && idx < (int)mods.size()) {
                            mods[idx]->Toggle(env);
                        }
                    }
                }
            }
        }
        dragging_ = false;
        dragPanel_ = nullptr;
    }
}

void ClickGUI::Render(JNIEnv* env, jobject fontRenderer, jmethodID drawString) {
    if (!open_) return;

    glPushAttrib(GL_ENABLE_BIT);
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

    // Panel header background
    glColor4f(
        ((color >> 16) & 0xFF) / 255.0f,
        ((color >> 8) & 0xFF) / 255.0f,
        (color & 0xFF) / 255.0f,
        0.8f);
    glBegin(GL_QUADS);
    glVertex2i(panel.x, panel.y);
    glVertex2i(panel.x + panel.w, panel.y);
    glVertex2i(panel.x + panel.w, panel.y + 18);
    glVertex2i(panel.x, panel.y + 18);
    glEnd();

    // Category name
    jstring name = env->NewStringUTF(CategoryName(panel.category));
    if (drawString) {
        env->CallIntMethod(fontRenderer, drawString, name, panel.x + 4, panel.y + 4, 0xFFFFFF);
    }
    env->DeleteLocalRef(name);

    if (!panel.expanded) return;

    // Module list background
    auto mods = g_moduleManager->GetByCategory(panel.category);
    int listY = panel.y + 20;
    int listH = (int)mods.size() * 12 + 4;

    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2i(panel.x, listY);
    glVertex2i(panel.x + panel.w, listY);
    glVertex2i(panel.x + panel.w, listY + listH);
    glVertex2i(panel.x, listY + listH);
    glEnd();

    // Module names
    int y = listY + 2;
    for (auto* mod : mods) {
        int textColor = mod->IsEnabled() ? 0x55FF55 : 0x888888;
        jstring modName = env->NewStringUTF(mod->GetDisplayName().c_str());
        if (drawString) {
            env->CallIntMethod(fontRenderer, drawString, modName, panel.x + 4, y, textColor);
        }
        env->DeleteLocalRef(modName);
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
    default: return 0x888888;
    }
}
