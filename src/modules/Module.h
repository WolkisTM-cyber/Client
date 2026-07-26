#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <jni.h>
#include "Category.h"
#include "Setting.h"

class Module {
public:
    Module(const std::string& name, const std::string& displayName, Category category, int key = 0);
    virtual ~Module() = default;

    virtual void OnEnable(JNIEnv* env) {}
    virtual void OnDisable(JNIEnv* env) {}
    virtual void OnTick(JNIEnv* env) {}
    virtual void OnKeyPress(JNIEnv* env, int key) {}

    void Toggle(JNIEnv* env);

    const std::string& GetName() const { return name_; }
    const std::string& GetDisplayName() const { return displayName_; }
    Category GetCategory() const { return category_; }
    bool IsEnabled() const { return enabled_; }
    int GetKey() const { return key_; }
    void SetKey(int key) { key_ = key; }

    Setting* GetSetting(const std::string& name);
    std::vector<Setting>& GetSettings() { return settings_; }

protected:
    void AddSetting(const Setting& s) { settings_.push_back(s); }

    std::string name_;
    std::string displayName_;
    Category category_;
    bool enabled_;
    int key_;
    std::vector<Setting> settings_;
};
