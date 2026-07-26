#pragma once
#include <jni.h>
#include <vector>
#include <functional>
#include <string>

enum class PacketDirection {
    Inbound,
    Outbound
};

struct PacketEvent {
    JNIEnv* env;
    jobject packet;
    std::string packetClassName;
    PacketDirection direction;
    bool cancelled = false;

    void Cancel() { cancelled = true; }
};

class PacketListener {
public:
    static PacketListener& Get() {
        static PacketListener instance;
        return instance;
    }

    using ListenerFunc = std::function<void(PacketEvent&)>;

    void Register(ListenerFunc listener) {
        listeners_.push_back(listener);
    }

    bool Dispatch(JNIEnv* env, jobject packet, PacketDirection dir) {
        if (!env || !packet) return false;

        jclass pClass = env->GetObjectClass(packet);
        if (!pClass) return false;

        jmethodID getClass = env->GetMethodID(pClass, "getClass", "()Ljava/lang/Class;");
        std::string className = "Packet";
        if (getClass) {
            jobject clazzObj = env->CallObjectMethod(packet, getClass);
            if (clazzObj) {
                jmethodID getName = env->GetMethodID(env->GetObjectClass(clazzObj), "getName", "()Ljava/lang/String;");
                if (getName) {
                    jstring nameStr = (jstring)env->CallObjectMethod(clazzObj, getName);
                    if (nameStr) {
                        const char* str = env->GetStringUTFChars(nameStr, nullptr);
                        if (str) className = str;
                        env->ReleaseStringUTFChars(nameStr, str);
                        env->DeleteLocalRef(nameStr);
                    }
                }
                env->DeleteLocalRef(clazzObj);
            }
        }
        env->DeleteLocalRef(pClass);

        PacketEvent event{ env, packet, className, dir, false };
        for (auto& listener : listeners_) {
            listener(event);
            if (event.cancelled) break;
        }

        return event.cancelled;
    }

private:
    std::vector<ListenerFunc> listeners_;
};
