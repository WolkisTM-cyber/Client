#pragma once
#include <string>
#include <array>
#include <jni.h>

// Compile-time XOR string obfuscation with dynamic key seed and thread_local evaluation
template <size_t N, char Key1 = 0x5A, char Key2 = 0x3F>
class XorString {
public:
    constexpr XorString(const char* str) {
        for (size_t i = 0; i < N; ++i) {
            data_[i] = str[i] ^ (Key1 + (i % 7) ^ Key2);
        }
    }

    const char* decrypt() const {
        static thread_local char buffer[N + 1];
        for (size_t i = 0; i < N; ++i) {
            buffer[i] = data_[i] ^ (Key1 + (i % 7) ^ Key2);
        }
        buffer[N] = '\0';
        return buffer;
    }

private:
    std::array<char, N> data_{};
};

#define XOR_STR(str) ([]() { \
    constexpr auto obfuscator = XorString<sizeof(str) - 1, (char)(__COUNTER__ * 0x13 + 0x5A), (char)(__LINE__ * 0x3F)>(str); \
    return obfuscator.decrypt(); \
}())

class SafeJNI {
public:
    static void ClearException(JNIEnv* env) {
        if (env && env->ExceptionCheck()) {
            env->ExceptionClear();
        }
    }

    static jclass FindClassSafe(JNIEnv* env, const char* className) {
        if (!env) return nullptr;
        jclass clazz = env->FindClass(className);
        ClearException(env);
        return clazz;
    }
};

