#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <string>

class MurdererFinder : public Module {
public:
    MurdererFinder() : Module("MurdererFinder", "Murderer Finder", Category::Combat, 0) {}

    void OnTick(JNIEnv* env) override {
        auto world = JNIHelper::GetWorld(env);
        if (!world) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) { env->DeleteLocalRef(world); return; }

        auto& c = JNIHelper::Get();
        jobject playerList = env->GetObjectField(world, c.getPlayerEntitiesF);
        if (!playerList) { env->DeleteLocalRef(player); env->DeleteLocalRef(world); return; }

        jint size = env->CallIntMethod(playerList, c.listSize);
        for (int i = 0; i < size; i++) {
            jobject entity = env->CallObjectMethod(playerList, c.listGet, i);
            if (!entity || env->IsSameObject(entity, player)) {
                if (entity) env->DeleteLocalRef(entity);
                continue;
            }

            jobject heldItem = env->CallObjectMethod(entity, c.getHeldItem);
            if (heldItem) {
                jobject item = env->CallObjectMethod(heldItem, c.getItemFromStack);
                if (item) {
                    jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
                    if (name) {
                        const char* str = env->GetStringUTFChars(name, nullptr);
                        if (str) {
                            std::string itemName(str);
                            if (itemName.find("sword") != std::string::npos ||
                                itemName.find("bow") != std::string::npos) {
                                jmethodID getName = env->GetMethodID(
                                    env->GetObjectClass(entity), "getName",
                                    "()Ljava/lang/String;");
                                if (getName) {
                                    jstring entityName = (jstring)env->CallObjectMethod(entity, getName);
                                    if (entityName) {
                                        const char* nameStr = env->GetStringUTFChars(entityName, nullptr);
                                        if (nameStr) {
                                            lastMurderer_ = nameStr;
                                            env->ReleaseStringUTFChars(entityName, nameStr);
                                        }
                                        env->DeleteLocalRef(entityName);
                                    }
                                }
                            }
                            env->ReleaseStringUTFChars(name, str);
                        }
                        env->DeleteLocalRef(name);
                    }
                    env->DeleteLocalRef(item);
                }
                env->DeleteLocalRef(heldItem);
            }
            env->DeleteLocalRef(entity);
        }

        env->DeleteLocalRef(playerList);
        env->DeleteLocalRef(player);
        env->DeleteLocalRef(world);
    }

    const std::string& GetMurderer() const { return lastMurderer_; }

private:
    std::string lastMurderer_;
};
