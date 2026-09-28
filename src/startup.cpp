#include "startup.h"

#include <windows.h>

#include <string>

namespace {

constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kValueName[] = L"ThemeToggleLite";

bool GetCurrentCommand(std::wstring& command) {
    std::wstring path(MAX_PATH, L'\0');
    for (;;) {
        const DWORD length = GetModuleFileNameW(nullptr, path.data(),
                                                 static_cast<DWORD>(path.size()));
        if (length == 0) {
            return false;
        }
        if (length < path.size()) {
            path.resize(length);
            command = L"\"" + path + L"\"";
            return true;
        }
        if (path.size() >= 32768) {
            return false;
        }
        path.resize(path.size() * 2);
    }
}

}  // namespace

StartupStatus GetStartupStatus() {
    HKEY key = nullptr;
    const LONG opened = RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_QUERY_VALUE, &key);
    if (opened == ERROR_FILE_NOT_FOUND || opened == ERROR_PATH_NOT_FOUND) {
        return StartupStatus::Disabled;
    }
    if (opened != ERROR_SUCCESS) {
        return StartupStatus::Error;
    }

    DWORD type = 0;
    DWORD bytes = 0;
    LONG queried = RegQueryValueExW(key, kValueName, nullptr, &type, nullptr, &bytes);
    if (queried == ERROR_FILE_NOT_FOUND) {
        RegCloseKey(key);
        return StartupStatus::Disabled;
    }
    if (queried != ERROR_SUCCESS || type != REG_SZ || bytes < sizeof(wchar_t) ||
        bytes % sizeof(wchar_t) != 0) {
        RegCloseKey(key);
        return StartupStatus::Error;
    }

    std::wstring value(bytes / sizeof(wchar_t), L'\0');
    queried = RegQueryValueExW(key, kValueName, nullptr, &type,
                               reinterpret_cast<BYTE*>(value.data()), &bytes);
    RegCloseKey(key);
    if (queried != ERROR_SUCCESS || type != REG_SZ || value.back() != L'\0') {
        return StartupStatus::Error;
    }
    value.resize(value.find(L'\0'));

    std::wstring command;
    if (!GetCurrentCommand(command)) {
        return StartupStatus::Error;
    }

    return CompareStringOrdinal(value.c_str(), -1, command.c_str(), -1, TRUE) == CSTR_EQUAL
        ? StartupStatus::Enabled : StartupStatus::StalePath;
}

bool SetStartupEnabled(bool enabled) {
    HKEY key = nullptr;
    if (enabled) {
        std::wstring command;
        if (!GetCurrentCommand(command) ||
            RegCreateKeyExW(HKEY_CURRENT_USER, kRunKey, 0, nullptr,
                            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr,
                            &key, nullptr) != ERROR_SUCCESS) {
            return false;
        }

        const DWORD bytes = static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t));
        const LONG result = RegSetValueExW(key, kValueName, 0, REG_SZ,
                                            reinterpret_cast<const BYTE*>(command.c_str()),
                                            bytes);
        RegCloseKey(key);
        return result == ERROR_SUCCESS;
    }

    const LONG opened = RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_SET_VALUE, &key);
    if (opened == ERROR_FILE_NOT_FOUND || opened == ERROR_PATH_NOT_FOUND) {
        return true;
    }
    if (opened != ERROR_SUCCESS) {
        return false;
    }

    const LONG result = RegDeleteValueW(key, kValueName);
    RegCloseKey(key);
    return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
}
