#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>
#include <vector>

struct AltAccount {
    std::string username;
    std::string uuid;
    std::string token;
    bool isOffline;
};

class AltManager : public Module {
public:
    AltManager() : Module("AltManager", "Alt Manager", Category::Quality, 0) {
        accounts_.push_back({"DefaultPlayer", "00000000-0000-0000-0000-000000000000", "0", true});
    }

    void AddAccount(const std::string& username, const std::string& token = "0", bool isOffline = true) {
        accounts_.push_back({username, "00000000-0000-0000-0000-000000000000", token, isOffline});
    }

    bool Login(JNIEnv* env, size_t index) {
        if (index >= accounts_.size() || !env) return false;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return false;

        auto& c = JNIHelper::Get();
        jfieldID sessionField = env->GetFieldID(c.minecraft, "session", "Lnet/minecraft/util/Session;");
        if (!sessionField) { env->DeleteLocalRef(mc); return false; }

        jobject session = env->GetObjectField(mc, sessionField);
        if (!session) { env->DeleteLocalRef(mc); return false; }

        jclass sessionClass = env->GetObjectClass(session);
        jfieldID usernameField = env->GetFieldID(sessionClass, "username", "Ljava/lang/String;");
        if (usernameField) {
            jstring nameStr = env->NewStringUTF(accounts_[index].username.c_str());
            env->SetObjectField(session, usernameField, nameStr);
            env->DeleteLocalRef(nameStr);
        }

        env->DeleteLocalRef(sessionClass);
        env->DeleteLocalRef(session);
        env->DeleteLocalRef(mc);
        currentIndex_ = index;
        return true;
    }

    const std::vector<AltAccount>& GetAccounts() const { return accounts_; }
    size_t GetCurrentIndex() const { return currentIndex_; }

private:
    std::vector<AltAccount> accounts_;
    size_t currentIndex_ = 0;
};
