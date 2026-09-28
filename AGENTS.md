# ThemeToggle Lite Development Rules

ThemeToggle Lite is a minimal native Windows utility for Windows 10 and 11 x64.

## Stack

- C++20
- Win32 API
- CMake

Do not introduce Qt, Electron, .NET, Boost, or third-party GUI frameworks unless explicitly requested.

## Architecture

Keep responsibilities separated:

- `main.cpp`: application lifecycle, message loop, and single-instance handling.
- `theme.cpp`: Windows registry theme state, theme switching, and `WM_SETTINGCHANGE` notification.
- `tray.cpp`: notification area icon, tray events, and context menu.
- `startup.cpp`: optional current-user startup registration in the Windows Run key.

## Coding style

- Prefer RAII where practical.
- Use Unicode Win32 APIs and explicit `W` suffix functions such as `CreateWindowExW`, `RegOpenKeyExW`, and `MessageBoxW`.
- Avoid unnecessary global mutable state, excessive abstraction, unnecessary classes, and premature framework architecture.

## Dependencies

Prefer Windows APIs. Do not add dependencies for functionality that Win32 already provides.

## Changes

Before modifying multiple modules:

1. Inspect existing code.
2. Identify affected files.
3. Make the smallest change necessary.
4. Build the project.
5. Report compiler errors, if any.

Do not rewrite unrelated working code.

## Build

```powershell
cmake -S . -B build -G Ninja
cmake --build build
```

Keep startup registration optional and tied to the current executable path.
