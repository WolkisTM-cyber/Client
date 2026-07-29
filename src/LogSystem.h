#pragma once
#include <string>
#include <fstream>
#include <ctime>
#include <mutex>
#include <atomic>
#include <windows.h>
#include <shlobj.h>

class LogSystem {
public:
    static LogSystem& Get() {
        static LogSystem inst;
        return inst;
    }

    void Init() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (initialized_) return;

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
                initialized_ = true;
                WriteInternal("[INFO] Log system initialized for Minecraft 1.8.9 Client");
            }
        }
    }

    void Write(const std::string& message) {
        WriteInfo(message);
    }

    void WriteDebug(const std::string& message) {
        WriteInternal("[DEBUG] " + message);
    }

    void WriteInfo(const std::string& message) {
        WriteInternal("[INFO] " + message);
    }

    void WriteWarn(const std::string& message) {
        WriteInternal("[WARN] " + message);
    }

    void WriteError(const std::string& message) {
        WriteInternal("[ERROR] " + message);
    }

    void WriteModule(const std::string& module, const std::string& message) {
        WriteInternal("[MODULE][" + module + "] " + message);
    }

    void CrashLog(const std::string& context) {
        exceptionCount_++;
        WriteInternal("!!! SAFE SEH CAUGHT EXCEPTION: " + context + " !!! [MC Maintained Running]");
    }

    int GetExceptionCount() const { return exceptionCount_.load(); }
    void ResetExceptionCount() { exceptionCount_.store(0); }

    bool IsInitialized() const { return initialized_; }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (logFile_.is_open()) {
            WriteInternal("[INFO] Log system shutting down for DLL detach / re-injection");
            logFile_.close();
        }
        initialized_ = false;
    }

private:
    LogSystem() : initialized_(false), exceptionCount_(0) {}
    ~LogSystem() {
        Shutdown();
    }

    void WriteInternal(const std::string& message) {
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

    std::string logPath_;
    std::ofstream logFile_;
    std::mutex mtx_;
    bool initialized_;
    std::atomic<int> exceptionCount_;
};

