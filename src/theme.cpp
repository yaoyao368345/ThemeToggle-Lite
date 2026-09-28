#include "theme.h"

#include <windows.h>

namespace {

constexpr wchar_t kPersonalizeKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
constexpr wchar_t kAppsValue[] = L"AppsUseLightTheme";
constexpr wchar_t kSystemValue[] = L"SystemUsesLightTheme";

struct RegistryValue {
    DWORD light = 1;
    bool exists = false;
};

bool ReadLightValue(HKEY key, const wchar_t* name, RegistryValue& value) {
    DWORD type = 0;
    DWORD size = sizeof(value.light);
    const LONG result = RegQueryValueExW(key, name, nullptr, &type,
                                         reinterpret_cast<BYTE*>(&value.light), &size);
    if (result == ERROR_FILE_NOT_FOUND) {
        value = {};
        return true;
    }

    value.exists = result == ERROR_SUCCESS;
    return value.exists && type == REG_DWORD && size == sizeof(value.light) &&
           value.light <= 1;
}

bool WriteLightValue(HKEY key, const wchar_t* name, DWORD value) {
    return RegSetValueExW(key, name, 0, REG_DWORD,
                          reinterpret_cast<const BYTE*>(&value), sizeof(value)) ==
           ERROR_SUCCESS;
}

void BroadcastThemeChange() {
    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                        reinterpret_cast<LPARAM>(L"ImmersiveColorSet"),
                        SMTO_ABORTIFHUNG, 100, nullptr);
}

}  // namespace

bool GetCurrentTheme(Theme& theme) {
    HKEY key = nullptr;
    const LONG opened = RegOpenKeyExW(HKEY_CURRENT_USER, kPersonalizeKey, 0,
                                      KEY_QUERY_VALUE, &key);
    if (opened == ERROR_FILE_NOT_FOUND || opened == ERROR_PATH_NOT_FOUND) {
        theme = Theme::Light;
        return true;
    }
    if (opened != ERROR_SUCCESS) {
        return false;
    }

    RegistryValue apps;
    RegistryValue system;
    const bool read_ok = ReadLightValue(key, kAppsValue, apps) &&
                         ReadLightValue(key, kSystemValue, system);
    RegCloseKey(key);
    if (!read_ok) {
        return false;
    }

    // Windows permits separate app and system settings. Use the app setting
    // as the current mode; SetTheme always sets both to the same mode.
    theme = apps.light == 1 ? Theme::Light : Theme::Dark;
    return true;
}

bool SetTheme(Theme theme) {
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kPersonalizeKey, 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE,
                        nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return false;
    }

    RegistryValue previous_apps;
    RegistryValue previous_system;
    if (!ReadLightValue(key, kAppsValue, previous_apps) ||
        !ReadLightValue(key, kSystemValue, previous_system)) {
        RegCloseKey(key);
        return false;
    }

    const DWORD light = theme == Theme::Light ? 1 : 0;
    if (!WriteLightValue(key, kAppsValue, light)) {
        RegCloseKey(key);
        return false;
    }

    if (!WriteLightValue(key, kSystemValue, light)) {
        const bool restored = previous_apps.exists
            ? WriteLightValue(key, kAppsValue, previous_apps.light)
            : RegDeleteValueW(key, kAppsValue) == ERROR_SUCCESS;
        if (!restored) {
            BroadcastThemeChange();
        }
        RegCloseKey(key);
        return false;
    }

    RegCloseKey(key);
    BroadcastThemeChange();
    return true;
}

bool ToggleTheme() {
    Theme current;
    if (!GetCurrentTheme(current)) {
        return false;
    }

    return SetTheme(current == Theme::Light ? Theme::Dark : Theme::Light);
}
