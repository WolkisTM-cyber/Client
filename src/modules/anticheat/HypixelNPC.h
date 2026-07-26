#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>
#include <vector>

class HypixelNPC : public Module {
public:
    HypixelNPC() : Module("HypixelNPC", "Hypixel NPC", Category::Misc, 0) {
        AddSetting(Setting::BoolSetting("ESP", "ESP Color", true));
        AddSetting(Setting::BoolSetting("Alerts", "Chat Alerts", true));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Name", "Scoreboard", "Both"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jobject playerList = env->GetObjectField(world, c.getPlayerEntitiesF);
        if (!playerList || env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(world);
            env->DeleteLocalRef(player);
            return;
        }

        jint size = env->CallIntMethod(playerList, c.listSize);
        if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(playerList); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        for (int i = 0; i < size; i++) {
            jobject entity = env->CallObjectMethod(playerList, c.listGet, i);
            if (!entity || env->ExceptionCheck()) {
                if (entity) env->DeleteLocalRef(entity);
                env->ExceptionClear();
                continue;
            }

            jmethodID getName = env->GetMethodID(
                env->GetObjectClass(entity), "getName",
                "()Ljava/lang/String;");
            if (!getName) { env->DeleteLocalRef(entity); continue; }

            jstring nameObj = (jstring)env->CallObjectMethod(entity, getName);
            if (!nameObj || env->ExceptionCheck()) {
                if (nameObj) env->DeleteLocalRef(nameObj);
                env->ExceptionClear();
                env->DeleteLocalRef(entity);
                continue;
            }

            const char* nameStr = env->GetStringUTFChars(nameObj, nullptr);
            if (!nameStr) { env->DeleteLocalRef(nameObj); env->DeleteLocalRef(entity); continue; }

            bool isNpc = IsHypixelNPC(nameStr);
            env->ReleaseStringUTFChars(nameObj, nameStr);
            env->DeleteLocalRef(nameObj);

            if (isNpc) {
                auto* alerts = GetSetting("Alerts");
                if (alerts && alerts->bVal) {
                    jstring msg = env->NewStringUTF("[Client] NPC detected.");
                    if (msg) {
                        env->CallVoidMethod(player, c.sendChatMessage, msg);
                        if (env->ExceptionCheck()) env->ExceptionClear();
                        env->DeleteLocalRef(msg);
                    }
                }
            }

            env->DeleteLocalRef(entity);
        }

        env->DeleteLocalRef(playerList);
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }

private:
    bool IsHypixelNPC(const std::string& name) {
        if (name.empty()) return true;

        // Hypixel NPC patterns
        if (name.find("\xC2\xA7") != std::string::npos) return true; // § color code
        if (name.find("[NPC]") != std::string::npos) return true;
        if (name.find("§") != std::string::npos) return true;

        // All lowercase + number suffix (e.g. "bot143")
        bool hasLetter = false;
        bool hasNumber = false;
        bool hasUpper = false;
        for (char c : name) {
            if (isupper(c)) hasUpper = true;
            if (islower(c)) hasLetter = true;
            if (isdigit(c)) hasNumber = true;
        }
        if (hasLetter && hasNumber && !hasUpper && name.length() > 4) return true;

        return false;
    }
};
