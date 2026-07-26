#pragma once
#include <string>
#include <fstream>
#include <ctime>
#include <mutex>
#include <windows.h>

class LogSystem {
public:
    static LogSystem& Get() {
        static LogSystem inst;
        return inst;
    }

    void Init() {
        wchar_t appData[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) {
            logPath_ = std::string(appData, appData + wcslen(appData)) + "\\Client\\logs\\";
            SHCreateDirectoryExW(nullptr, std::wstring(logPath_.begin(), logPath_.end()).c_str(), nullptr);

            time_t t = time(nullptr);
            struct tm timeinfo;
            localtime_s(&timeinfo, &t);
            char buf[64];
            strftime(buf, sizeof(buf), "client_%Y%m%d_%H%M%S.log", &timeinfo);
            logPath_ += buf;

            logFile_.open(logPath_, std::ios::app);
            if (logFile_.is_open()) {
                Write("Log system initialized");
            }
        }
    }

    void Write(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!logFile_.is_open()) return;

        time_t t = time(nullptr);
        struct tm timeinfo;
        localtime_s(&timeinfo, &t);
        char buf[32];
        strftime(buf, sizeof(buf), "[%H:%M:%S]", &timeinfo);

        logFile_ << buf << " " << message << std::endl;
        logFile_.flush();
    }

    void WriteModule(const std::string& module, const std::string& message) {
        Write("[" + module + "] " + message);
    }

    void CrashLog(const std::string& context) {
        Write("!!! CRASH: " + context + " !!!");
        // Save and close
        if (logFile_.is_open()) {
            logFile_ << "=== CRASH LOG END ===" << std::endl;
            logFile_.close();
        }
    }

private:
    LogSystem() = default;
    ~LogSystem() {
        if (logFile_.is_open()) {
            Write("Log system shutdown");
            logFile_.close();
        }
    }

    std::string logPath_;
    std::ofstream logFile_;
    std::mutex mtx_;
};
