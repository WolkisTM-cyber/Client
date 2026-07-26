#pragma once
#include "../modules/Module.h"
#include "../modules/JNIHelper.h"
#include <vector>

class ClickGUI : public Module {
public:
    ClickGUI() : Module("ClickGUI", "Click GUI", Category::Visual, VK_RSHIFT) {
        AddSetting(Setting::BoolSetting("Rainbow", "Rainbow", false));
    }

    void OnEnable(JNIEnv* env) override;
    void OnDisable(JNIEnv* env) override;
    void OnTick(JNIEnv* env) override;

    void Render(JNIEnv* env, jobject fontRenderer, jmethodID drawString);
    bool IsOpen() const { return open_; }

private:
    struct Panel {
        Category category;
        int x, y, w, h;
        bool expanded;
    };

    void DrawPanel(JNIEnv* env, jobject fontRenderer, jmethodID drawString, Panel& panel);
    int GetCategoryColor(Category cat);

    std::vector<Panel> panels_;
    bool open_ = false;
    int mouseX_ = 0;
    int mouseY_ = 0;
    bool dragging_ = false;
    int dragX_, dragY_;
    Panel* dragPanel_ = nullptr;
};
