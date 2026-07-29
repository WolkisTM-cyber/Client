#include "ClickGUI.h"
#include "../modules/ModuleManager.h"
#include <GL/gl.h>
#include <algorithm>
#include <cstdio>

extern ModuleManager* g_moduleManager;

struct FindWindowData { DWORD pid; HWND* result; };

static BOOL CALLBACK FindMinecraftEnumProc(HWND hwnd, LPARAM lParam) {
    auto* data = reinterpret_cast<FindWindowData*>(lParam);
    DWORD wpid;
    GetWindowThreadProcessId(hwnd, &wpid);
    if (wpid == data->pid && IsWindowVisible(hwnd)) {
        wchar_t cls[64];
        GetClassNameW(hwnd, cls, 64);
        if (wcscmp(cls, L"LWJGL") == 0 || wcscmp(cls, L"GLFW30") == 0) {
            *data->result = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

HWND ClickGUI::FindMinecraftWindow() {
    DWORD pid = GetCurrentProcessId();
    HWND result = nullptr;
    FindWindowData data = { pid, &result };
    EnumWindows(FindMinecraftEnumProc, reinterpret_cast<LPARAM>(&data));
    return result;
}

void ClickGUI::OnEnable(JNIEnv* env) {
    open_.store(true);
    std::lock_guard<std::mutex> lock(panelMutex_);
    panels_.clear();

    int x = 15;
    int catCount = (int)Category::COUNT;
    for (int i = 0; i < catCount; i++) {
        Category cat = (Category)i;
        Panel p;
        p.category = cat;
        p.x = x;
        p.y = 25;
        p.w = 140;
        p.h = 24;
        p.listH = 0;
        p.expandedMod = nullptr;
        panels_.push_back(p);
        x += 148;
    }

    dragging_ = false;
    dragPanel_ = nullptr;
    wasMouseDown_ = false;
    bindingKey_ = false;
    bindingMod_ = nullptr;
}

void ClickGUI::OnDisable(JNIEnv* env) {
    open_.store(false);
    std::lock_guard<std::mutex> lock(panelMutex_);
    panels_.clear();
    bindingKey_ = false;
    bindingMod_ = nullptr;
}

void ClickGUI::OnTick(JNIEnv* env) {
    if (!open_.load()) return;

    if (GetAsyncKeyState(VK_RSHIFT) & 1) {
        Toggle(env);
        return;
    }

    // Key binding capture listener
    if (bindingKey_ && bindingMod_) {
        for (int k = 1; k < 255; k++) {
            if (k == VK_LBUTTON || k == VK_RBUTTON) continue;
            if (GetAsyncKeyState(k) & 1) {
                if (k == VK_ESCAPE) {
                    bindingMod_->SetKey(0);
                } else {
                    bindingMod_->SetKey(k);
                }
                bindingKey_ = false;
                bindingMod_ = nullptr;
                break;
            }
        }
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

    // Retrieve viewport mouse coordinates safely
    POINT pt;
    GetCursorPos(&pt);
    HWND mcWnd = FindMinecraftWindow();
    if (mcWnd) {
        ScreenToClient(mcWnd, &pt);
    }

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int vw = vp[2] > 0 ? vp[2] : 854;
    int vh = vp[3] > 0 ? vp[3] : 480;

    RECT clientRect;
    if (mcWnd && GetClientRect(mcWnd, &clientRect)) {
        int cw = clientRect.right - clientRect.left;
        int ch = clientRect.bottom - clientRect.top;
        if (cw > 0 && ch > 0) {
            pt.x = (int)((float)pt.x * ((float)vw / (float)cw));
            pt.y = (int)((float)pt.y * ((float)vh / (float)ch));
        }
    }

    mouseX_ = pt.x;
    mouseY_ = pt.y;

    bool isMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    bool isRightClick = (GetAsyncKeyState(VK_RBUTTON) & 1) != 0;
    bool clicked = isMouseDown && !wasMouseDown_;
    wasMouseDown_ = isMouseDown;

    std::lock_guard<std::mutex> lock(panelMutex_);

    // Handle header dragging
    if (isMouseDown) {
        if (!dragging_) {
            for (auto& panel : panels_) {
                if (mouseX_ >= panel.x && mouseX_ <= panel.x + panel.w &&
                    mouseY_ >= panel.y && mouseY_ <= panel.y + 22) {
                    dragging_ = true;
                    dragPanel_ = &panel;
                    dragOffX_ = mouseX_ - panel.x;
                    dragOffY_ = mouseY_ - panel.y;
                    break;
                }
            }
        }
        if (dragging_ && dragPanel_) {
            dragPanel_->x = mouseX_ - dragOffX_;
            dragPanel_->y = mouseY_ - dragOffY_;
        }
    } else {
        dragging_ = false;
        dragPanel_ = nullptr;
    }

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& panel : panels_) {
        DrawPanel(env, fontRenderer, drawString, panel, mouseX_, mouseY_, clicked || isRightClick);
    }

    glPopAttrib();
}

void ClickGUI::DrawPanel(JNIEnv* env, jobject fontRenderer, jmethodID drawString, Panel& panel, int mouseX, int mouseY, bool clicked) {
    // 1. Header Card (Dark Violet Accent)
    glDisable(GL_TEXTURE_2D);
    DrawRect(panel.x, panel.y, panel.w, 22, 0.09f, 0.09f, 0.12f, 0.95f);
    DrawRect(panel.x, panel.y, panel.w, 2, 0.42f, 0.36f, 0.90f, 1.0f); // Violet Header Bar
    DrawOutline(panel.x, panel.y, panel.w, 22, 0.22f, 0.22f, 0.28f, 0.9f);

    // Draw Category Title
    glEnable(GL_TEXTURE_2D);
    jstring catTitle = env->NewStringUTF(CategoryName(panel.category));
    if (catTitle && drawString) {
        env->CallIntMethod(fontRenderer, drawString, catTitle, panel.x + 8, panel.y + 6, 0xF1F2F6);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(catTitle);
    }

    auto mods = g_moduleManager->GetByCategory(panel.category);
    int curY = panel.y + 24;
    int totalH = 0;

    // Calculate background list height
    for (auto* mod : mods) {
        totalH += 18;
        if (panel.expandedMod == mod) {
            totalH += (int)mod->GetSettings().size() * 16 + 18; // +18 for Keybind setting
        }
    }
    panel.listH = totalH + 4;

    // Draw Tree Container Body
    glDisable(GL_TEXTURE_2D);
    DrawRect(panel.x, curY, panel.w, panel.listH, 0.07f, 0.07f, 0.09f, 0.94f);
    DrawOutline(panel.x, curY, panel.w, panel.listH, 0.18f, 0.18f, 0.22f, 0.7f);

    int y = curY + 2;
    bool isRightClick = (GetAsyncKeyState(VK_RBUTTON) & 1) != 0;

    for (auto* mod : mods) {
        bool hoverMod = (mouseX >= panel.x && mouseX <= panel.x + panel.w &&
                         mouseY >= y && mouseY < y + 18);

        // Handle clicks on module tree node
        if (hoverMod && clicked) {
            if (isRightClick) {
                // Right click toggles settings tree expansion
                panel.expandedMod = (panel.expandedMod == mod) ? nullptr : mod;
            } else {
                // Left click toggles module state
                mod->Toggle(env);
            }
        }

        // Draw Module Tree Node
        glDisable(GL_TEXTURE_2D);
        if (mod->IsEnabled()) {
            DrawRect(panel.x + 2, y + 1, panel.w - 4, 16, 0.0f, 0.78f, 0.75f, hoverMod ? 0.30f : 0.20f);
            DrawRect(panel.x + 2, y + 1, 3, 16, 0.0f, 0.85f, 0.80f, 1.0f);
        } else if (hoverMod) {
            DrawRect(panel.x + 2, y + 1, panel.w - 4, 16, 1.0f, 1.0f, 1.0f, 0.06f);
        }

        // Draw Module Status & Title
        glEnable(GL_TEXTURE_2D);
        std::string modLabel = (mod->IsEnabled() ? "[X] " : "[ ] ") + mod->GetDisplayName();
        if (!mod->GetSettings().empty()) {
            modLabel += (panel.expandedMod == mod ? "  v" : "  >");
        }

        int textColor = mod->IsEnabled() ? 0x00CEC9 : (hoverMod ? 0xDCDDE1 : 0x8C92AC);
        jstring jModName = env->NewStringUTF(modLabel.c_str());
        if (jModName && drawString) {
            env->CallIntMethod(fontRenderer, drawString, jModName, panel.x + 8, y + 4, textColor);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(jModName);
        }
        y += 18;

        // Draw Expanded Settings Sub-Tree Nodes
        if (panel.expandedMod == mod) {
            auto& settings = mod->GetSettings();
            for (auto& setting : settings) {
                bool hoverSet = (mouseX >= panel.x + 10 && mouseX <= panel.x + panel.w - 4 &&
                                 mouseY >= y && mouseY < y + 16);

                glDisable(GL_TEXTURE_2D);
                // Sub-tree branch line
                DrawRect(panel.x + 10, y + 2, 2, 12, 0.35f, 0.35f, 0.45f, 0.6f);

                if (setting.IsBool()) {
                    if (hoverSet && clicked) {
                        setting.SetBool(!setting.GetBool());
                    }
                    glEnable(GL_TEXTURE_2D);
                    std::string setStr = "  " + setting.GetName() + ": " + (setting.GetBool() ? "ON" : "OFF");
                    jstring jSetStr = env->NewStringUTF(setStr.c_str());
                    if (jSetStr && drawString) {
                        env->CallIntMethod(fontRenderer, drawString, jSetStr, panel.x + 14, y + 3, setting.GetBool() ? 0x55FF55 : 0xAAAAAA);
                        if (env->ExceptionCheck()) env->ExceptionClear();
                        env->DeleteLocalRef(jSetStr);
                    }
                } else if (setting.IsNumber()) {
                    float val = setting.GetNumber();
                    float minV = setting.GetMin();
                    float maxV = setting.GetMax();
                    float pct = (val - minV) / (maxV - minV > 0.0001f ? maxV - minV : 1.0f);
                    pct = (std::max)(0.0f, (std::min)(1.0f, pct));

                    int sliderX = panel.x + 16;
                    int sliderW = panel.w - 24;

                    if (hoverSet && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
                        float newPct = (float)(mouseX - sliderX) / (float)sliderW;
                        newPct = (std::max)(0.0f, (std::min)(1.0f, newPct));
                        setting.SetNumber(minV + newPct * (maxV - minV));
                    }

                    DrawRect(sliderX, y + 12, sliderW, 3, 0.2f, 0.2f, 0.25f, 0.9f);
                    DrawRect(sliderX, y + 12, (int)(sliderW * pct), 3, 0.0f, 0.8f, 0.75f, 1.0f);

                    glEnable(GL_TEXTURE_2D);
                    char sbuf[64];
                    snprintf(sbuf, sizeof(sbuf), "  %s: %.1f", setting.GetName().c_str(), setting.GetNumber());
                    jstring jSetStr = env->NewStringUTF(sbuf);
                    if (jSetStr && drawString) {
                        env->CallIntMethod(fontRenderer, drawString, jSetStr, panel.x + 14, y + 2, 0xCCCCCC);
                        if (env->ExceptionCheck()) env->ExceptionClear();
                        env->DeleteLocalRef(jSetStr);
                    }
                }
                y += 16;
            }

            // Keybind Tree Node
            bool hoverKey = (mouseX >= panel.x + 10 && mouseX <= panel.x + panel.w - 4 &&
                             mouseY >= y && mouseY < y + 16);
            if (hoverKey && clicked) {
                bindingKey_ = true;
                bindingMod_ = mod;
            }

            glDisable(GL_TEXTURE_2D);
            DrawRect(panel.x + 10, y + 2, 2, 12, 0.35f, 0.35f, 0.45f, 0.6f);
            glEnable(GL_TEXTURE_2D);
            std::string keyStr = "  Bind: ";
            if (bindingKey_ && bindingMod_ == mod) {
                keyStr += "[Press Key...]";
            } else {
                keyStr += (mod->GetKey() != 0 ? std::to_string(mod->GetKey()) : "NONE");
            }
            jstring jKeyStr = env->NewStringUTF(keyStr.c_str());
            if (jKeyStr && drawString) {
                env->CallIntMethod(fontRenderer, drawString, jKeyStr, panel.x + 14, y + 2, bindingMod_ == mod ? 0xFF5555 : 0xE1B12C);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(jKeyStr);
            }
            y += 18;
        }
    }

    panel.h = 24 + panel.listH;
}

int ClickGUI::GetCategoryColor(Category cat) {
    return 0x6C5CE7;
}


