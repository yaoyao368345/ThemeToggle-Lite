#include "tray.h"

#include "startup.h"
#include "theme.h"
#include "resource.h"

#include <shellapi.h>

namespace {

constexpr UINT kTrayIconId = 1;
constexpr UINT kToggleCommand = 1001;
constexpr UINT kAboutCommand = 1002;
constexpr UINT kExitCommand = 1003;
constexpr UINT kStartupCommand = 1004;
constexpr UINT kStartupSettingsCommand = 1005;

NOTIFYICONDATAW MakeTrayData(HWND window) {
    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = window;
    data.uID = kTrayIconId;
    return data;
}

HICON CurrentThemeIcon() {
    Theme theme;
    if (!GetCurrentTheme(theme)) {
        return nullptr;
    }

    const int resource = theme == Theme::Light ? IDI_LIGHT : IDI_DARK;
    return LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(resource));
}

void ShowError(HWND window, const wchar_t* message) {
    MessageBoxW(window, message, L"ThemeToggle Lite", MB_OK | MB_ICONERROR);
}

void ToggleFromTray(HWND window) {
    if (!ToggleTheme()) {
        UpdateTrayIcon(window);
        ShowError(window, L"无法切换主题。请检查当前用户的主题设置是否可读写。");
        return;
    }

    if (!UpdateTrayIcon(window)) {
        ShowError(window, L"主题已切换，但托盘图标未能更新。请重启程序。");
    }
}

}  // namespace

bool CreateTrayIcon(HWND window) {
    NOTIFYICONDATAW data = MakeTrayData(window);
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    data.uCallbackMessage = kTrayIconMessage;
    data.hIcon = CurrentThemeIcon();
    lstrcpynW(data.szTip, L"ThemeToggle Lite", ARRAYSIZE(data.szTip));

    return data.hIcon != nullptr && Shell_NotifyIconW(NIM_ADD, &data) != FALSE;
}

bool UpdateTrayIcon(HWND window) {
    NOTIFYICONDATAW data = MakeTrayData(window);
    data.uFlags = NIF_ICON;
    data.hIcon = CurrentThemeIcon();
    return data.hIcon != nullptr && Shell_NotifyIconW(NIM_MODIFY, &data) != FALSE;
}

void RemoveTrayIcon(HWND window) {
    NOTIFYICONDATAW data = MakeTrayData(window);
    Shell_NotifyIconW(NIM_DELETE, &data);
}

void HandleTrayNotification(HWND window, LPARAM event) {
    switch (static_cast<UINT>(event)) {
        case WM_LBUTTONUP:
            ToggleFromTray(window);
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
        ShowError(window, L"无法创建托盘菜单。");
        return;
    }

    const StartupStatus startup = GetStartupStatus();
    const UINT startup_flags = MF_STRING |
        (startup == StartupStatus::Registered ? MF_CHECKED : 0) |
        (startup == StartupStatus::Error ? MF_GRAYED : 0);
    const wchar_t* startup_label = startup == StartupStatus::StalePath
        ? L"修复开机自启登记"
        : startup == StartupStatus::Error ? L"开机自启登记（无法读取）" : L"开机自启登记";

    const bool menu_ready = AppendMenuW(menu, MF_STRING, kToggleCommand, L"切换主题") &&
                            AppendMenuW(menu, startup_flags, kStartupCommand, startup_label) &&
                            AppendMenuW(menu, MF_STRING, kStartupSettingsCommand, L"Windows 启动应用设置") &&
                            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr) &&
                            AppendMenuW(menu, MF_STRING, kAboutCommand, L"关于") &&
                            AppendMenuW(menu, MF_STRING, kExitCommand, L"退出");
    if (!menu_ready) {
        DestroyMenu(menu);
        ShowError(window, L"无法创建托盘菜单。");
        return;
    }

    POINT cursor{};
    if (GetCursorPos(&cursor)) {
        SetForegroundWindow(window);
        const UINT command = TrackPopupMenu(
            menu, TPM_RIGHTBUTTON | TPM_RETURNCMD,
            cursor.x, cursor.y, 0, window, nullptr);
        PostMessageW(window, WM_NULL, 0, 0);

        switch (command) {
            case kToggleCommand:
                ToggleFromTray(window);
                break;
            case kStartupCommand:
                if (!SetStartupEnabled(startup != StartupStatus::Registered)) {
                    ShowError(window, L"无法更改开机自启设置。请检查当前用户的启动项是否可写。");
                }
                break;
            case kStartupSettingsCommand:
                if (reinterpret_cast<INT_PTR>(ShellExecuteW(
                        window, L"open", L"ms-settings:startupapps",
                        nullptr, nullptr, SW_SHOWNORMAL)) <= 32) {
                    ShowError(window, L"无法打开 Windows 启动应用设置。请在任务管理器的启动应用中查看 ThemeToggleLite。");
                }
                break;
            case kAboutCommand:
                MessageBoxW(window, L"ThemeToggle Lite v0.2.0",
                            L"关于 ThemeToggle Lite", MB_OK | MB_ICONINFORMATION);
                break;
            case kExitCommand:
                DestroyWindow(window);
                break;
            default:
                break;
        }
    } else {
        ShowError(window, L"无法获取鼠标位置，托盘菜单未打开。");
    }

    DestroyMenu(menu);
}
