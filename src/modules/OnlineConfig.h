#pragma once
#include "Module.h"
#include "JNIHelper.h"
#include <string>
#include <fstream>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

class OnlineConfig : public Module {
public:
    OnlineConfig() : Module("OnlineConfig", "Online Config", Category::Misc, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Download", "Upload", "Sync"}, 0));
        AddSetting(Setting::IntSetting("Interval", "Interval (s)", 60, 10, 600));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        clock_t now = clock();
        int interval = GetSetting("Interval")->iVal * CLOCKS_PER_SEC;
        if (now - lastSync_ < interval) return;
        lastSync_ = now;

        switch (GetSetting("Mode")->modeVal) {
        case 0: DownloadConfig(); break;
        case 1: UploadConfig(); break;
        case 2:
            if (DownloadConfig()) {
                // Reload config
                ConfigManager::Get().Load(g_moduleManager);
            }
            break;
        }
    }

private:
    bool DownloadConfig() {
        std::string url = "https://gist.githubusercontent.com/WolkisTM-cyber/client-config/raw/config.txt";
        std::string data = HttpGet(url);
        if (data.empty()) return false;

        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) return false;

        std::string path = std::string(appData, appData + wcslen(appData)) + "\\Client\\online_config.txt";
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << data;
        return true;
    }

    bool UploadConfig() {
        // Simplified - would need GitHub API for gist upload
        // For now, just save locally
        ConfigManager::Get().Save(g_moduleManager);
        return true;
    }

    std::string HttpGet(const std::string& url) {
        std::string result;
        HINTERNET session = WinHttpOpen(L"Client/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                         nullptr, nullptr, 0);
        if (!session) return result;

        HINTERNET connect = WinHttpConnect(session, L"gist.githubusercontent.com",
                                            INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (connect) {
            HINTERNET request = WinHttpOpenRequest(connect, L"GET",
                L"/WolkisTM-cyber/client-config/raw/config.txt", nullptr,
                nullptr, nullptr, WINHTTP_FLAG_SECURE);
            if (request) {
                if (WinHttpSendRequest(request, nullptr, 0, nullptr, 0, 0, 0)) {
                    WinHttpReceiveResponse(request, nullptr);
                    DWORD bytesAvail = 0;
                    while (WinHttpQueryDataAvailable(request, &bytesAvail) && bytesAvail > 0) {
                        std::vector<char> buf(bytesAvail + 1, 0);
                        DWORD bytesRead = 0;
                        WinHttpReadData(request, buf.data(), bytesAvail, &bytesRead);
                        result.append(buf.data(), bytesRead);
                    }
                }
                WinHttpCloseHandle(request);
            }
            WinHttpCloseHandle(connect);
        }
        WinHttpCloseHandle(session);
        return result;
    }

    clock_t lastSync_ = 0;
};
