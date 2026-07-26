#pragma once
#include <windows.h>
#include <vector>
#include <memory>
#include <mutex>
#include <jni.h>
#include "Module.h"

class ModuleManager {
public:
    ModuleManager();
    ~ModuleManager();

    void Init(JNIEnv* env);
    void OnTick(JNIEnv* env);
    void OnKeyPress(JNIEnv* env, int key);

    template<typename T, typename... Args>
    T* AddModule(Args&&... args) {
        std::lock_guard<std::mutex> lock(moduleMutex_);
        auto mod = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = mod.get();
        modules_.push_back(std::move(mod));
        allCache_.clear();
        return ptr;
    }

    Module* Find(const std::string& name);
    std::vector<Module*> GetAll();
    std::vector<Module*> GetByCategory(Category cat);

private:
    std::vector<std::unique_ptr<Module>> modules_;
    std::vector<Module*> allCache_;
    mutable std::mutex moduleMutex_;
};
