#pragma once

enum class Theme {
    Light,
    Dark,
};

// Returns Light if the theme settings cannot be read.
Theme GetCurrentTheme();

bool SetTheme(Theme theme);

bool ToggleTheme();
