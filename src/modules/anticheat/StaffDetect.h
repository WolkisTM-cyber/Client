#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../ModuleManager.h"
#include <string>
#include <vector>
#include <set>

class StaffDetect : public Module {
public:
    StaffDetect() : Module("StaffDetect", "Staff Detect", Category::Misc, 0) {
        AddSetting(Setting::BoolSetting("Alerts", "Chat Alerts", true));
        AddSetting(Setting::BoolSetting("AutoDisable", "Auto Disable", true));
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

        int prevCount = prevPlayers_;
        prevPlayers_ = size;

        for (int i = 0; i < size; i++) {
            jobject entity = env->CallObjectMethod(playerList, c.listGet, i);
            if (!entity || env->ExceptionCheck()) {
                if (entity) env->DeleteLocalRef(entity);
                env->ExceptionClear();
                continue;
            }
            if (env->IsSameObject(entity, player)) { env->DeleteLocalRef(entity); continue; }

            jmethodID getName = env->GetMethodID(
                env->GetObjectClass(entity), "getName", "()Ljava/lang/String;");
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

            // Hypixel staff: [GM] or [MOD] or [HELPER] prefix, or all-colored name
            std::string s(nameStr);
            bool isStaff = false;
            if (s.find("GM") != std::string::npos) isStaff = true;
            if (s.find("[MOD]") != std::string::npos) isStaff = true;
            if (s.find("[HELPER]") != std::string::npos) isStaff = true;
            if (s.find("[ADMIN]") != std::string::npos) isStaff = true;
            if (s.find("[OWNER]") != std::string::npos) isStaff = true;

            // Detect fake players (staff alts): 0 kills, 0 wins, specific name patterns
            if (s.length() <= 4) isStaff = true; // Very short names
            if (s.find("§") != std::string::npos && s.find("[") == std::string::npos) isStaff = true;

            if (isStaff && !seenStaff_.count(s)) {
                seenStaff_.insert(s);

                auto* alerts = GetSetting("Alerts");
                if (alerts && alerts->bVal) {
                    std::string msg = "[Client] Staff detected: " + s;
                    jstring jmsg = env->NewStringUTF(msg.c_str());
                    if (jmsg) {
                        env->CallVoidMethod(player, c.sendChatMessage, jmsg);
                        if (env->ExceptionCheck()) env->ExceptionClear();
                        env->DeleteLocalRef(jmsg);
                    }
                }

                auto* autoDisable = GetSetting("AutoDisable");
                if (autoDisable && autoDisable->bVal) {
                    for (auto* mod : g_moduleManager->GetAll()) {
                        if (mod->IsEnabled() && mod->GetCategory() != Category::Visual) {
                            mod->Toggle(env);
                        }
                    }
                }
            }

            env->ReleaseStringUTFChars(nameObj, nameStr);
            env->DeleteLocalRef(nameObj);
            env->DeleteLocalRef(entity);
        }

        env->DeleteLocalRef(playerList);
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }

private:
    int prevPlayers_ = 0;
    std::vector<std::string> names_;
    std::set<std::string> seenStaff_;
};
