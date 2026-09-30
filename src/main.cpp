#include <windows.h>

#include "tray.h"

namespace {

constexpr wchar_t kWindowClassName[] = L"ThemeToggleLite.HiddenWindow";
constexpr wchar_t kMutexName[] = L"Local\\ThemeToggleLite.Singleton";
constexpr UINT_PTR kTrayRetryTimer = 1;
constexpr UINT kTrayRetryIntervalMs = 1000;
constexpr unsigned kMaxTrayAttempts = 11;  // Initial attempt plus ten retries.

struct WindowState {
    UINT taskbar_created_message = 0;
    unsigned tray_attempts = 0;
    bool retrying_tray = false;
};

void ShowStartupError(const wchar_t* message) {
    MessageBoxW(nullptr, message, L"ThemeToggle Lite", MB_OK | MB_ICONERROR);
}

void TryCreateTrayIcon(HWND window, WindowState& state) {
    ++state.tray_attempts;
    if (CreateTrayIcon(window)) {
        KillTimer(window, kTrayRetryTimer);
        state.retrying_tray = false;
        return;
    }

    if (state.tray_attempts < kMaxTrayAttempts &&
        (state.retrying_tray ||
         SetTimer(window, kTrayRetryTimer, kTrayRetryIntervalMs, nullptr) != 0)) {
        state.retrying_tray = true;
        return;
    }

    KillTimer(window, kTrayRetryTimer);
    state.retrying_tray = false;
    ShowStartupError(L"无法创建或恢复托盘图标。请确认 Windows 资源管理器正在运行，且当前用户的主题设置可读，然后重新启动程序。");
    DestroyWindow(window);
    PostQuitMessage(1);
}

void BeginTrayCreation(HWND window, WindowState& state) {
    KillTimer(window, kTrayRetryTimer);
    state.tray_attempts = 0;
    state.retrying_tray = false;
    TryCreateTrayIcon(window, state);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_NCCREATE) {
        const auto* creation = reinterpret_cast<const CREATESTRUCTW*>(lparam);
        SetWindowLongPtrW(window, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(creation->lpCreateParams));
    }
    auto* state = reinterpret_cast<WindowState*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));

    if (state != nullptr && message == state->taskbar_created_message) {
        // Explorer discards notification icons when its taskbar is recreated.
        BeginTrayCreation(window, *state);
        return 0;
    }

    if (message == WM_TIMER && wparam == kTrayRetryTimer) {
        if (state != nullptr && state->retrying_tray) {
            TryCreateTrayIcon(window, *state);
        }
        return 0;
    }

    if (message == kTrayIconMessage) {
        HandleTrayNotification(window, lparam);
        return 0;
    }

    if (message == WM_SETTINGCHANGE || message == WM_THEMECHANGED) {
        UpdateTrayIcon(window);
    }

    if (message == WM_DESTROY) {
        KillTimer(window, kTrayRetryTimer);
        RemoveTrayIcon(window);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(window, message, wparam, lparam);
}

}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
    HANDLE mutex = CreateMutexW(nullptr, FALSE, kMutexName);
    if (mutex == nullptr) {
        ShowStartupError(L"无法创建单实例锁，程序未启动。");
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        return 0;
    }

    WindowState state;
    state.taskbar_created_message = RegisterWindowMessageW(L"TaskbarCreated");
    if (state.taskbar_created_message == 0) {
        ShowStartupError(L"无法注册托盘恢复通知，程序未启动。");
        CloseHandle(mutex);
        return 1;
    }

    WNDCLASSW window_class{};
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = instance;
    window_class.lpszClassName = kWindowClassName;

    if (RegisterClassW(&window_class) == 0) {
        ShowStartupError(L"无法注册程序窗口，程序未启动。");
        CloseHandle(mutex);
        return 1;
    }

    HWND window = CreateWindowExW(
        0, kWindowClassName, L"ThemeToggle Lite", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        nullptr, nullptr, instance, &state);
    if (window == nullptr) {
        ShowStartupError(L"无法创建程序窗口，程序未启动。");
        UnregisterClassW(kWindowClassName, instance);
        CloseHandle(mutex);
        return 1;
    }

    BeginTrayCreation(window, state);

    MSG message{};
    int result = 0;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    if (IsWindow(window)) {
        DestroyWindow(window);
    }
    UnregisterClassW(kWindowClassName, instance);
    CloseHandle(mutex);
    if (result == -1) {
        ShowStartupError(L"程序消息循环发生错误，已退出。");
    }
    return result == -1 ? 1 : static_cast<int>(message.wParam);
}
