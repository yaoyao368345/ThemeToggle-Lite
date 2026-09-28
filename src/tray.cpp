#include "tray.h"

#include "theme.h"

#include <shellapi.h>

namespace {

constexpr UINT kTrayIconId = 1;
constexpr UINT kToggleCommand = 1001;
constexpr UINT kAboutCommand = 1002;
constexpr UINT kExitCommand = 1003;

NOTIFYICONDATAW MakeTrayData(HWND window) {
    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = window;
    data.uID = kTrayIconId;
    return data;
}

}  // namespace

bool CreateTrayIcon(HWND window) {
    NOTIFYICONDATAW data = MakeTrayData(window);
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    data.uCallbackMessage = kTrayIconMessage;
    data.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    lstrcpynW(data.szTip, L"ThemeToggle Lite", ARRAYSIZE(data.szTip));

    return data.hIcon != nullptr && Shell_NotifyIconW(NIM_ADD, &data) != FALSE;
}

void RemoveTrayIcon(HWND window) {
    NOTIFYICONDATAW data = MakeTrayData(window);
    Shell_NotifyIconW(NIM_DELETE, &data);
}

void HandleTrayNotification(HWND window, LPARAM event) {
    switch (static_cast<UINT>(event)) {
        case WM_LBUTTONUP:
            ToggleTheme();
            break;
        case WM_RBUTTONUP:
            ShowTrayMenu(window);
            break;
        default:
            break;
    }
}

void ShowTrayMenu(HWND window) {
    HMENU menu = CreatePopupMenu();
    if (menu == nullptr) {
        return;
    }

    AppendMenuW(menu, MF_STRING, kToggleCommand, L"切换主题");
    AppendMenuW(menu, MF_STRING, kAboutCommand, L"关于");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kExitCommand, L"退出");

    POINT cursor{};
    if (GetCursorPos(&cursor)) {
        SetForegroundWindow(window);
        const UINT command = TrackPopupMenu(
            menu, TPM_RIGHTBUTTON | TPM_RETURNCMD,
            cursor.x, cursor.y, 0, window, nullptr);
        PostMessageW(window, WM_NULL, 0, 0);

        switch (command) {
            case kToggleCommand:
                ToggleTheme();
                break;
            case kAboutCommand:
                MessageBoxW(window, L"ThemeToggle Lite v0.1.0",
                            L"关于 ThemeToggle Lite", MB_OK | MB_ICONINFORMATION);
                break;
            case kExitCommand:
                DestroyWindow(window);
                break;
            default:
                break;
        }
    }

    DestroyMenu(menu);
}
