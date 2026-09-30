# ThemeToggle Lite

ThemeToggle Lite 是一个适用于 Windows 10 / 11 x64 的轻量级深色、浅色主题切换工具。程序常驻系统托盘，不显示主窗口，也不需要管理员权限。

## 功能

- 左键单击托盘图标，在深色和浅色主题之间切换。
- 同时更新 Windows 的应用主题和系统主题，并通知系统刷新。
- 托盘图标显示当前主题；右键菜单提供切换、开机自启、关于和退出。
- 单实例运行。开机自启由用户在托盘菜单中主动开启，仅作用于当前 Windows 用户。
- Windows 资源管理器重启后自动恢复托盘图标；托盘尚未就绪时每秒重试一次，最多重试 10 次。

## 使用

从 [Releases](../../releases) 直接下载 `ThemeToggle.exe` 并运行。程序启动后可在任务栏通知区域找到太阳或月亮图标；图标也可能位于隐藏图标菜单中。

若启用了开机自启，移动 EXE 后可在右键菜单选择“修复开机自启”，将启动项更新为当前位置。退出程序请使用托盘右键菜单中的“退出”。

## 从源码构建

需要 CMake 3.20+、Ninja，以及 MinGW-w64 GCC 或 MSVC。使用 MinGW-w64 时，在 PowerShell 中运行：

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

生成的程序位于 `build\ThemeToggle.exe`。重新构建前，请先从托盘菜单退出正在运行的程序。

## 设计

使用 C++20、Win32 API 和 CMake，不依赖 Qt、Electron 或 .NET Runtime。开机自启通过当前用户的 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` 启用，无需管理员权限。

## 许可证

[MIT](LICENSE)
