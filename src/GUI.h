#pragma once
#include <windows.h>
#include <atomic>

class GUI {
public:
    GUI();
    ~GUI();

    bool Create(HMODULE module);
    void Destroy();
    void SetVisible(bool visible);
    bool IsVisible() const;

    void SetSprintMode(int mode);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static DWORD WINAPI ThreadProc(LPVOID param);
    void Run();
    void Render();
    void UpdateWindowPosition();
    HWND FindMinecraftWindow();
    const wchar_t* ModeName(int mode) const;

    HMODULE module_;
    HWND hwnd_;
    HWND targetWnd_;
    HANDLE thread_;
    bool visible_;
    int sprintMode_;
    std::atomic<bool> threadRunning_;
    int width_;
    int height_;

    static const int ToggleWidth = 40;
    static const int ToggleHeight = 18;
};
