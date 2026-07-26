#pragma once
#include <string>
#include <vector>
#include <functional>
#include <jni.h>

class Command {
public:
    Command(const std::string& name, const std::string& description,
            std::function<bool(JNIEnv*, const std::vector<std::string>&)> handler)
        : name_(name), description_(description), handler_(handler) {}

    const std::string& GetName() const { return name_; }
    const std::string& GetDescription() const { return description_; }
    bool Execute(JNIEnv* env, const std::vector<std::string>& args) {
        return handler_(env, args);
    }

private:
    std::string name_;
    std::string description_;
    std::function<bool(JNIEnv*, const std::vector<std::string>&)> handler_;
};

class CommandManager {
public:
    CommandManager();
    ~CommandManager();

    bool Execute(JNIEnv* env, const std::string& input);
    void AddCommand(Command cmd);

private:
    std::vector<Command> commands_;
};
