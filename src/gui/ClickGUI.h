#pragma once
#include "../modules/Module.h"
#include "../modules/JNIHelper.h"
#include <vector>
#include <mutex>

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
    };

    void DrawPanel(JNIEnv* env, jobject fontRenderer, jmethodID drawString, Panel& panel);
    int GetCategoryColor(Category cat);

    std::vector<Panel> panels_;
    std::atomic<bool> open_;
    std::mutex panelMutex_;
    int mouseX_, mouseY_;
    bool dragging_;
    Panel* dragPanel_;
    int dragOffX_, dragOffY_;
};
