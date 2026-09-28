#include "theme.h"

#include <windows.h>

namespace {

constexpr wchar_t kPersonalizeKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
constexpr wchar_t kAppsValue[] = L"AppsUseLightTheme";
constexpr wchar_t kSystemValue[] = L"SystemUsesLightTheme";

bool ReadLightValue(HKEY key, const wchar_t* name, DWORD& value) {
    DWORD type = 0;
    DWORD size = sizeof(value);
    return RegQueryValueExW(key, name, nullptr, &type,
                            reinterpret_cast<BYTE*>(&value), &size) == ERROR_SUCCESS &&
           type == REG_DWORD && size == sizeof(value) && value <= 1;
}

bool ReadCurrentTheme(Theme& theme) {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kPersonalizeKey, 0, KEY_QUERY_VALUE, &key) !=
        ERROR_SUCCESS) {
        return false;
    }

    DWORD apps_light = 0;
    DWORD system_light = 0;
    const bool read_ok = ReadLightValue(key, kAppsValue, apps_light) &&
                         ReadLightValue(key, kSystemValue, system_light);
    RegCloseKey(key);
    if (!read_ok) {
        return false;
    }

    // Windows permits separate app and system settings. Use the app setting
    // as the current mode; SetTheme always sets both to the same mode.
    theme = apps_light == 1 ? Theme::Light : Theme::Dark;
    return true;
}

bool WriteLightValue(HKEY key, const wchar_t* name, DWORD value) {
    return RegSetValueExW(key, name, 0, REG_DWORD,
                          reinterpret_cast<const BYTE*>(&value), sizeof(value)) ==
           ERROR_SUCCESS;
}

}  // namespace

Theme GetCurrentTheme() {
    Theme theme = Theme::Light;
    ReadCurrentTheme(theme);
    return theme;
}

bool SetTheme(Theme theme) {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kPersonalizeKey, 0, KEY_SET_VALUE, &key) !=
        ERROR_SUCCESS) {
        return false;
    }

    const DWORD light = theme == Theme::Light ? 1 : 0;
    const bool write_ok = WriteLightValue(key, kAppsValue, light) &&
                          WriteLightValue(key, kSystemValue, light);
    RegCloseKey(key);
    if (!write_ok) {
        return false;
    }

    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                        reinterpret_cast<LPARAM>(L"ImmersiveColorSet"),
                        SMTO_ABORTIFHUNG, 100, nullptr);
    return true;
}

bool ToggleTheme() {
    Theme current;
    if (!ReadCurrentTheme(current)) {
        return false;
    }

    return SetTheme(current == Theme::Light ? Theme::Dark : Theme::Light);
}
