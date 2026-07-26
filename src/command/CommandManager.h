#pragma once
#include <string>
#include <vector>
#include <functional>
#include <jni.h>

struct Command {
    std::string name;
    std::string description;
    std::string usage;
    std::function<void(JNIEnv*, const std::vector<std::string>&)> handler;
};

class CommandManager {
public:
    CommandManager();
    ~CommandManager() = default;

    void RegisterDefaults();
    bool Execute(JNIEnv* env, const std::string& input);

private:
    void Register(const std::string& name, const std::string& desc,
                  const std::string& usage,
                  std::function<void(JNIEnv*, const std::vector<std::string>&)> handler);

    std::vector<Command> commands_;
};
