#include "GUI.h"
#include <utility>

GUI::GUI()
    : module_(nullptr)
    , hwnd_(nullptr)
    , targetWnd_(nullptr)
    , thread_(nullptr)
    , visible_(true)
    , sprintMode_(1)
    , threadRunning_(false)
    , width_(185)
    , height_(38)
{
}

GUI::~GUI() {
    Destroy();
}

bool GUI::Create(HMODULE module) {
    module_ = module;
    threadRunning_ = true;
    thread_ = CreateThread(nullptr, 0, ThreadProc, this, 0, nullptr);
    return thread_ != nullptr;
}

void GUI::Destroy() {
    threadRunning_ = false;
    if (thread_) {
        if (hwnd_) {
            PostMessageW(hwnd_, WM_CLOSE, 0, 0);
        }
        WaitForSingleObject(thread_, 2000);
        CloseHandle(thread_);
        thread_ = nullptr;
    }
    hwnd_ = nullptr;
}

void GUI::SetVisible(bool visible) {
    visible_ = visible;
    if (hwnd_) {
        ShowWindow(hwnd_, visible ? SW_SHOW : SW_HIDE);
    }
}

bool GUI::IsVisible() const {
    return visible_;
}

void GUI::SetSprintMode(int mode) {
    sprintMode_ = mode;
    if (hwnd_) {
        InvalidateRect(hwnd_, nullptr, TRUE);
    }
}

const wchar_t* GUI::ModeName(int mode) const {
    switch (mode) {
    case 0: return L"Off";
    case 1: return L"Forward";
    case 2: return L"Always";
    default: return L"Unknown";
    }
}

struct FindWindowData { DWORD pid; HWND* result; };

static BOOL CALLBACK FindMinecraftEnumProc(HWND hwnd, LPARAM lParam) {
    auto* data = reinterpret_cast<FindWindowData*>(lParam);
    DWORD wpid;
    GetWindowThreadProcessId(hwnd, &wpid);
    if (wpid == data->pid && IsWindowVisible(hwnd)) {
        wchar_t cls[64];
        GetClassNameW(hwnd, cls, 64);
        if (wcscmp(cls, L"LWJGL") == 0 || wcscmp(cls, L"GLFW30") == 0) {
            *data->result = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

static BOOL CALLBACK FindMinecraftFallbackEnumProc(HWND hwnd, LPARAM lParam) {
    auto* data = reinterpret_cast<FindWindowData*>(lParam);
    DWORD wpid;
    GetWindowThreadProcessId(hwnd, &wpid);
    if (wpid == data->pid && IsWindowVisible(hwnd)) {
        wchar_t title[128];
        GetWindowTextW(hwnd, title, 128);
        if (wcsstr(title, L"Minecraft") || wcsstr(title, L"minecraft")) {
            *data->result = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

HWND GUI::FindMinecraftWindow() {
    DWORD pid = GetCurrentProcessId();
    HWND result = nullptr;
    FindWindowData data = { pid, &result };
    EnumWindows(FindMinecraftEnumProc, reinterpret_cast<LPARAM>(&data));
    if (!result) {
        EnumWindows(FindMinecraftFallbackEnumProc, reinterpret_cast<LPARAM>(&data));
    }
    return result;
}

void GUI::UpdateWindowPosition() {
    if (!hwnd_) return;

    HWND mcWnd = FindMinecraftWindow();
    if (!mcWnd) {
        SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, width_, height_,
                     SWP_NOACTIVATE | SWP_HIDEWINDOW);
        return;
    }
    targetWnd_ = mcWnd;

    RECT mcRect;
    if (!GetClientRect(mcWnd, &mcRect)) return;
    MapWindowPoints(mcWnd, nullptr, (POINT*)&mcRect, 2);

    int x = mcRect.left + (mcRect.right - mcRect.left - width_) / 2;
    int y = mcRect.top + 10;

    SetWindowPos(hwnd_, HWND_TOPMOST, x, y, width_, height_,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

DWORD WINAPI GUI::ThreadProc(LPVOID param) {
    GUI* gui = static_cast<GUI*>(param);
    gui->Run();
    return 0;
}

void GUI::Run() {
    const wchar_t CLASS_NAME[] = L"AutoSprintOverlay";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = module_;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = nullptr;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    HDC screenDC = GetDC(nullptr);
    int dpiX = GetDeviceCaps(screenDC, LOGPIXELSX);
    ReleaseDC(nullptr, screenDC);
    float scale = max(1.0f, dpiX / 96.0f);
    int winW = static_cast<int>(width_ * scale);
    int winH = static_cast<int>(height_ * scale);

    hwnd_ = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        CLASS_NAME, L"AutoSprint",
        WS_POPUP,
        0, 0, winW, winH,
        nullptr, nullptr, module_, this
    );

    if (!hwnd_) {
        threadRunning_ = false;
        return;
    }

    SetLayeredWindowAttributes(hwnd_, 0, 210, LWA_ALPHA);

    targetWnd_ = FindMinecraftWindow();
    UpdateWindowPosition();

    SetTimer(hwnd_, 1, 50, nullptr);
    ShowWindow(hwnd_, SW_SHOW);

    MSG msg;
    while (threadRunning_ && GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    KillTimer(hwnd_, 1);
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
    UnregisterClassW(CLASS_NAME, module_);
}

void GUI::Render() {
    if (!hwnd_) return;

    RECT client;
    GetClientRect(hwnd_, &client);
    int w = client.right;
    int h = client.bottom;
    if (w <= 0 || h <= 0) return;

    bool active = sprintMode_ > 0;

    HDC hdc = GetDC(hwnd_);
    if (!hdc) return;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, w, h);
    HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

    SetBkMode(memDC, TRANSPARENT);

    HBRUSH bgBrush = CreateSolidBrush(RGB(14, 14, 24));
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(45, 45, 62));
    HGDIOBJ oldPen = SelectObject(memDC, borderPen);
    HGDIOBJ oldBrush = SelectObject(memDC, bgBrush);
    RoundRect(memDC, 0, 0, w, h, 12, 12);
    SelectObject(memDC, GetStockObject(NULL_BRUSH));
    SelectObject(memDC, oldBrush);
    DeleteObject(bgBrush);

    HPEN innerPen = CreatePen(PS_SOLID, 1, RGB(26, 26, 40));
    SelectObject(memDC, innerPen);
    SelectObject(memDC, GetStockObject(NULL_BRUSH));
    RoundRect(memDC, 1, 1, w - 1, h - 1, 10, 10);

    HFONT font = CreateFontW(-13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    SelectObject(memDC, font);

    int dotSize = 7;
    int dotX = 14;
    int dotY = (h - dotSize) / 2;

    COLORREF dotCol = active ? RGB(0, 215, 75) : RGB(70, 70, 85);
    HBRUSH dotBrush = CreateSolidBrush(dotCol);
    HPEN dotPen = CreatePen(PS_SOLID, 1, active ? RGB(0, 180, 60) : RGB(55, 55, 70));
    SelectObject(memDC, dotPen);
    SelectObject(memDC, dotBrush);
    Ellipse(memDC, dotX, dotY, dotX + dotSize, dotY + dotSize);
    DeleteObject(dotPen);
    DeleteObject(dotBrush);

    int textX = dotX + dotSize + 10;
    int toggleRightMargin = 60;

    if (active) {
        const wchar_t* modeName = ModeName(sprintMode_);
        SetTextColor(memDC, RGB(0, 215, 75));

        RECT modeRect = { textX, 0, textX + 60, h };
        DrawTextW(memDC, modeName, -1, &modeRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

        int modeW = 0;
        SIZE sz;
        if (GetTextExtentPoint32W(memDC, modeName, wcslen(modeName), &sz)) {
            modeW = sz.cx + 6;
        }

        SetTextColor(memDC, RGB(120, 120, 140));
        RECT labelRect = { textX + modeW, 0, w - toggleRightMargin, h };
        DrawTextW(memDC, L"Auto Sprint", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
    } else {
        SetTextColor(memDC, RGB(160, 160, 175));
        RECT labelRect = { textX, 0, w - toggleRightMargin, h };
        DrawTextW(memDC, L"Off", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
    }

    int toggleX = w - 12 - 40;
    int toggleY = (h - 18) / 2;

    COLORREF toggleCol = active ? RGB(0, 215, 75) : RGB(50, 50, 65);
    HBRUSH toggleBg = CreateSolidBrush(toggleCol);
    HPEN togglePen = CreatePen(PS_SOLID, 1, active ? RGB(0, 180, 60) : RGB(38, 38, 52));
    SelectObject(memDC, togglePen);
    SelectObject(memDC, toggleBg);
    RoundRect(memDC, toggleX, toggleY, toggleX + 40, toggleY + 18, 18, 18);
    DeleteObject(togglePen);
    DeleteObject(toggleBg);

    int thumbSize = 12;
    int thumbX = active ? toggleX + 25 : toggleX + 3;
    HBRUSH thumbBrush = CreateSolidBrush(RGB(240, 240, 248));
    HPEN thumbPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 215));
    SelectObject(memDC, thumbPen);
    SelectObject(memDC, thumbBrush);
    Ellipse(memDC, thumbX, toggleY + 3, thumbX + thumbSize, toggleY + 3 + thumbSize);
    DeleteObject(thumbPen);
    DeleteObject(thumbBrush);

    SelectObject(memDC, oldPen);
    DeleteObject(innerPen);
    DeleteObject(font);

    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
    ReleaseDC(hwnd_, hdc);
}

LRESULT CALLBACK GUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GUI* gui = reinterpret_cast<GUI*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_CREATE: {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }
    case WM_TIMER: {
        if (gui) {
            gui->UpdateWindowPosition();
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        if (gui) gui->Render();
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
