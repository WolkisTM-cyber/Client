#include "ModuleManager.h"
#include <algorithm>

ModuleManager::ModuleManager() = default;
ModuleManager::~ModuleManager() = default;

void ModuleManager::Init(JNIEnv* env) {
    std::lock_guard<std::mutex> lock(moduleMutex_);
    for (auto& m : modules_) {
        if (m->IsEnabled()) m->OnEnable(env);
    }
}

void ModuleManager::OnTick(JNIEnv* env) {
    std::lock_guard<std::mutex> lock(moduleMutex_);
    for (auto& m : modules_) {
        if (m->IsEnabled()) m->OnTick(env);
    }
}

void ModuleManager::OnKeyPress(JNIEnv* env, int key) {
    std::lock_guard<std::mutex> lock(moduleMutex_);
    for (auto& m : modules_) {
        if (m->GetKey() == key) {
            m->Toggle(env);
        }
        if (m->IsEnabled()) m->OnKeyPress(env, key);
    }
}

Module* ModuleManager::Find(const std::string& name) {
    std::lock_guard<std::mutex> lock(moduleMutex_);
    for (auto& m : modules_) {
        if (m->GetName() == name) return m.get();
    }
    return nullptr;
}

std::vector<Module*> ModuleManager::GetAll() {
    std::lock_guard<std::mutex> lock(moduleMutex_);
    if (allCache_.empty()) {
        for (auto& m : modules_) allCache_.push_back(m.get());
    }
    return allCache_;
}

std::vector<Module*> ModuleManager::GetByCategory(Category cat) {
    std::lock_guard<std::mutex> lock(moduleMutex_);
    std::vector<Module*> result;
    for (auto& m : modules_) {
        if (m->GetCategory() == cat) result.push_back(m.get());
    }
    return result;
}
