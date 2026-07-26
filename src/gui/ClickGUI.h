#pragma once
#include "../modules/Module.h"
#include "../modules/JNIHelper.h"
#include <vector>
#include <mutex>
#include <atomic>

class ClickGUI : public Module {
public:
    ClickGUI() : Module("ClickGUI", "Click GUI", Category::Visual, VK_RSHIFT) {
        AddSetting(Setting::BoolSetting("Rainbow", "Rainbow", false));
    }

    void OnEnable(JNIEnv* env) override;
    void OnDisable(JNIEnv* env) override;
    void OnTick(JNIEnv* env) override;

    void Render(JNIEnv* env, jobject fontRenderer, jmethodID drawString);
    bool IsOpen() const { return open_.load(); }

private:
    struct Panel {
        Category category;
        int x, y, w, h;
        int listH;
        Module* expandedMod = nullptr;
    };

    void DrawPanel(JNIEnv* env, jobject fontRenderer, jmethodID drawString, Panel& panel);
    void DrawRect(int x, int y, int w, int h, float r, float g, float b, float a);
    void DrawOutline(int x, int y, int w, int h, float r, float g, float b, float a);
    int GetCategoryColor(Category cat);

    std::vector<Panel> panels_;
    std::atomic<bool> open_;
    std::mutex panelMutex_;
    int mouseX_ = 0, mouseY_ = 0;
    bool dragging_ = false;
    Panel* dragPanel_ = nullptr;
    int dragOffX_ = 0, dragOffY_ = 0;
};
