#pragma once

enum class Theme {
    Light,
    Dark,
};

// Missing Windows theme settings use the default Light mode.
// Other registry errors return false without changing theme.
bool GetCurrentTheme(Theme& theme);

bool SetTheme(Theme theme);

bool ToggleTheme();
