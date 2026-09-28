#include <windows.h>

#include "tray.h"

namespace {

constexpr wchar_t kWindowClassName[] = L"ThemeToggleLite.HiddenWindow";
constexpr wchar_t kMutexName[] = L"Local\\ThemeToggleLite.Singleton";

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == kTrayIconMessage) {
        HandleTrayNotification(window, lparam);
        return 0;
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
        CloseHandle(mutex);
        return 1;
    }

    HWND window = CreateWindowExW(
        0, kWindowClassName, L"ThemeToggle Lite", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        nullptr, nullptr, instance, nullptr);
    if (window == nullptr) {
        UnregisterClassW(kWindowClassName, instance);
        CloseHandle(mutex);
        return 1;
    }

    if (!CreateTrayIcon(window)) {
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
    return result == -1 ? 1 : static_cast<int>(message.wParam);
}
