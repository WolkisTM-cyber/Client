#pragma once
#include <windows.h>
#include <jni.h>

class GUI;

constexpr int CYCLE_KEY = VK_RSHIFT;

enum class SprintMode : int {
    Off = 0,
    Forward,
    Always,
    COUNT
};

class AutoSprint {
public:
    AutoSprint();
    ~AutoSprint();

    bool Start(GUI* gui);
    void Stop();
    bool IsRunning() const;

    SprintMode GetMode() const;
    void SetMode(SprintMode mode);

private:
    static DWORD WINAPI ThreadProc(LPVOID param);
    void Run();
    void CycleMode();

    GUI* gui_;
    volatile bool running_;
    volatile SprintMode mode_;
    volatile bool keyDown_;
    HANDLE thread_;
    mutable CRITICAL_SECTION cs_;
};
