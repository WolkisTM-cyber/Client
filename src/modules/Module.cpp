#include "Module.h"

Module::Module(const std::string& name, const std::string& displayName, Category category, int key)
    : name_(name), displayName_(displayName), category_(category), enabled_(false), key_(key) {}

void Module::Toggle(JNIEnv* env) {
    std::lock_guard<std::recursive_mutex> lock(toggleMutex_);
    bool was = enabled_.exchange(!enabled_.load());
    if (was) {
        if (env) OnDisable(env);
    } else {
        if (env) OnEnable(env);
    }
}

bool Module::ToggleNoJNI() {
    std::lock_guard<std::recursive_mutex> lock(toggleMutex_);
    bool was = enabled_.exchange(!enabled_.load());
    return !was; // return new state
}

Setting* Module::GetSetting(const std::string& name) {
    for (auto& s : settings_) {
        if (s.name == name) return &s;
    }
    return nullptr;
}
