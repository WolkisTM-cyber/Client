#pragma once
#include <string>
#include <array>
#include <jni.h>

// Compile-time XOR string obfuscation to prevent memory string scanning
template <size_t N, char K = 0x5A>
class XorString {
public:
    constexpr XorString(const char* str) {
        for (size_t i = 0; i < N; ++i) {
            data_[i] = str[i] ^ K;
        }
    }

    std::string decrypt() const {
        std::string result;
        result.reserve(N);
        for (size_t i = 0; i < N; ++i) {
            result.push_back(data_[i] ^ K);
        }
        return result;
    }

    const char* c_str() const {
        static thread_local char buffer[N + 1];
        for (size_t i = 0; i < N; ++i) {
            buffer[i] = data_[i] ^ K;
        }
        buffer[N] = '\0';
        return buffer;
    }

private:
    std::array<char, N> data_{};
};

#define XOR_STR(str) (XorString<sizeof(str) - 1>(str).c_str())

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
