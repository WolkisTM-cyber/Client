#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>

#pragma comment(lib, "advapi32.lib")

DWORD FindProcess(const wchar_t* name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W entry = { sizeof(PROCESSENTRY32W) };
    DWORD pid = 0;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, name) == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return pid;
}

bool Inject(DWORD pid, const wchar_t* dllPath) {
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        std::wcerr << L"OpenProcess failed: " << GetLastError() << std::endl;
        return false;
    }

    size_t pathSize = (wcslen(dllPath) + 1) * sizeof(wchar_t);
    void* remoteMem = VirtualAllocEx(process, nullptr, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMem) {
        std::wcerr << L"VirtualAllocEx failed: " << GetLastError() << std::endl;
        CloseHandle(process);
        return false;
    }

    if (!WriteProcessMemory(process, remoteMem, dllPath, pathSize, nullptr)) {
        std::wcerr << L"WriteProcessMemory failed: " << GetLastError() << std::endl;
        VirtualFreeEx(process, remoteMem, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    LPTHREAD_START_ROUTINE loadLib = (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryW");

    HANDLE thread = CreateRemoteThread(process, nullptr, 0, loadLib, remoteMem, 0, nullptr);
    if (!thread) {
        std::wcerr << L"CreateRemoteThread failed: " << GetLastError() << std::endl;
        VirtualFreeEx(process, remoteMem, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    WaitForSingleObject(thread, INFINITE);

    DWORD exitCode;
    GetExitCodeThread(thread, &exitCode);

    CloseHandle(thread);
    VirtualFreeEx(process, remoteMem, 0, MEM_RELEASE);
    CloseHandle(process);

    return exitCode != 0;
}

int wmain(int argc, wchar_t* argv[]) {
    std::wcout << L"=== Client Injector ===" << std::endl;

    std::wstring processName = L"javaw.exe";
    std::wstring dllPath;

    if (argc >= 2) dllPath = argv[1];
    else {
        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        std::wstring exePath(buf);
        auto pos = exePath.find_last_of(L"\\");
        dllPath = exePath.substr(0, pos) + L"\\Client.dll";
    }

    if (argc >= 3) processName = argv[2];

    std::wcout << L"Target: " << processName << std::endl;
    std::wcout << L"DLL: " << dllPath << std::endl;

    DWORD pid = FindProcess(processName.c_str());
    if (!pid) {
        std::wcerr << L"Process not found: " << processName << std::endl;
        return 1;
    }

    std::wcout << L"Found PID: " << pid << std::endl;

    if (Inject(pid, dllPath.c_str())) {
        std::wcout << L"Injected successfully!" << std::endl;
        return 0;
    } else {
        std::wcerr << L"Injection failed!" << std::endl;
        return 1;
    }
}
