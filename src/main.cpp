#include <windows.h>

#include "tray.h"

namespace {

constexpr wchar_t kWindowClassName[] = L"ThemeToggleLite.HiddenWindow";
constexpr wchar_t kMutexName[] = L"Local\\ThemeToggleLite.Singleton";

void ShowStartupError(const wchar_t* message) {
    MessageBoxW(nullptr, message, L"ThemeToggle Lite", MB_OK | MB_ICONERROR);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == kTrayIconMessage) {
        HandleTrayNotification(window, lparam);
        return 0;
    }

    if (message == WM_SETTINGCHANGE || message == WM_THEMECHANGED) {
        UpdateTrayIcon(window);
    }

    if (message == WM_DESTROY) {
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
        nullptr, nullptr, instance, nullptr);
    if (window == nullptr) {
        ShowStartupError(L"无法创建程序窗口，程序未启动。");
        UnregisterClassW(kWindowClassName, instance);
        CloseHandle(mutex);
        return 1;
    }

    if (!CreateTrayIcon(window)) {
        ShowStartupError(L"无法创建托盘图标。请确认 Windows 资源管理器正在运行，且当前用户的主题设置可读。");
        DestroyWindow(window);
        UnregisterClassW(kWindowClassName, instance);
        CloseHandle(mutex);
        return 1;
    }

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
