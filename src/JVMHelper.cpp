#include "JVMHelper.h"
#include <string>
#include <vector>

typedef jint(JNICALL* JNI_GetCreatedJavaVMs_t)(JavaVM**, jsize, jsize*);

namespace JVMHelper {

    static bool loaded = false;
    static HMODULE jvmModule = nullptr;
    static JNI_GetCreatedJavaVMs_t JNI_GetCreatedJavaVMs = nullptr;

    static bool EnsureLoaded() {
        if (loaded) return true;

        std::vector<std::wstring> candidates = {
            L"jvm.dll"
        };

        for (const auto& path : candidates) {
            jvmModule = GetModuleHandleW(path.c_str());
            if (jvmModule) break;
            jvmModule = LoadLibraryW(path.c_str());
            if (jvmModule) break;
        }

        if (!jvmModule) {
            wchar_t buf[MAX_PATH];
            HKEY hKey;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\JavaSoft\\Java Runtime Environment", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                wchar_t version[64] = {0};
                DWORD size = sizeof(version);
                if (RegQueryValueExW(hKey, L"CurrentVersion", nullptr, nullptr, (LPBYTE)version, &size) == ERROR_SUCCESS) {
                    std::wstring subKey = L"SOFTWARE\\JavaSoft\\Java Runtime Environment\\";
                    subKey += version;
                    HKEY hSubKey;
                    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subKey.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                        size = sizeof(buf);
                        if (RegQueryValueExW(hSubKey, L"RuntimeLib", nullptr, nullptr, (LPBYTE)buf, &size) == ERROR_SUCCESS) {
                            jvmModule = LoadLibraryW(buf);
                        }
                        RegCloseKey(hSubKey);
                    }
                }
                RegCloseKey(hKey);
            }
        }

        if (!jvmModule) {
            wchar_t buf[MAX_PATH];
            HKEY hKey;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\JavaSoft\\JDK", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                wchar_t version[64] = {0};
                DWORD size = sizeof(version);
                if (RegQueryValueExW(hKey, L"CurrentVersion", nullptr, nullptr, (LPBYTE)version, &size) == ERROR_SUCCESS) {
                    std::wstring subKey = L"SOFTWARE\\JavaSoft\\JDK\\";
                    subKey += version;
                    HKEY hSubKey;
                    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subKey.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                        size = sizeof(buf);
                        if (RegQueryValueExW(hSubKey, L"RuntimeLib", nullptr, nullptr, (LPBYTE)buf, &size) == ERROR_SUCCESS) {
                            jvmModule = LoadLibraryW(buf);
                        }
                        RegCloseKey(hSubKey);
                    }
                }
                RegCloseKey(hKey);
            }
        }

        if (!jvmModule) return false;

        JNI_GetCreatedJavaVMs = (JNI_GetCreatedJavaVMs_t)GetProcAddress(jvmModule, "JNI_GetCreatedJavaVMs");
        if (!JNI_GetCreatedJavaVMs) return false;

        loaded = true;
        return true;
    }

    bool FindAndAttach(JavaVM** vm, JNIEnv** env) {
        if (!EnsureLoaded()) return false;

        jsize count = 0;
        if (JNI_GetCreatedJavaVMs(vm, 1, &count) != JNI_OK || count == 0) {
            return false;
        }

        jint result = (*vm)->GetEnv((void**)env, JNI_VERSION_1_6);
        if (result == JNI_EDETACHED) {
            JavaVMAttachArgs args = { JNI_VERSION_1_6, "AutoSprint", nullptr };
            result = (*vm)->AttachCurrentThread((void**)env, &args);
        }

        return result == JNI_OK;
    }

    void Detach(JavaVM* vm) {
        if (vm) {
            vm->DetachCurrentThread();
        }
    }

    bool IsAttached(JavaVM* vm) {
        if (!vm) return false;
        JNIEnv* env = nullptr;
        return vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK;
    }

}
