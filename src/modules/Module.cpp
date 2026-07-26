#include "Module.h"

Module::Module(const std::string& name, const std::string& displayName, Category category, int key)
    : name_(name), displayName_(displayName), category_(category), enabled_(false), key_(key) {}

void Module::Toggle(JNIEnv* env) {
    enabled_ = !enabled_;
    if (enabled_ && env) OnEnable(env);
    else if (!enabled_ && env) OnDisable(env);
}

Setting* Module::GetSetting(const std::string& name) {
    for (auto& s : settings_) {
        if (s.name == name) return &s;
    }
    return nullptr;
}
