# ThemeToggle Lite

ThemeToggle Lite 是一个适用于 Windows 10 / 11 x64 的轻量级深色、浅色主题切换工具。程序常驻系统托盘，不显示主窗口，也不需要管理员权限。

当前版本：**v0.3.0** · [下载 EXE](https://github.com/yaoyao368345/ThemeToggle-Lite/releases/download/v0.3.0/ThemeToggle.exe) · [发布说明](https://github.com/yaoyao368345/ThemeToggle-Lite/releases/tag/v0.3.0)

## 功能

- 左键单击托盘图标，在深色和浅色主题之间切换。
- 同时更新 Windows 的应用主题和系统主题，并通知系统刷新。
- 托盘图标显示当前主题；右键菜单提供切换、自启登记、Windows 启动应用设置、关于和退出。
- 单实例运行。开机自启由用户在托盘菜单中主动开启，仅作用于当前 Windows 用户。
- Windows 资源管理器重启后自动恢复托盘图标；托盘尚未就绪时每秒重试一次，最多重试 10 次。

## 使用

从 [Releases](../../releases) 直接下载 `ThemeToggle.exe` 并运行。程序启动后可在任务栏通知区域找到太阳或月亮图标；图标也可能位于隐藏图标菜单中。

升级时先通过托盘菜单退出旧版本，再用新 EXE 替换原文件并运行。

若启用了开机自启，移动 EXE 后可在右键菜单选择“修复开机自启登记”，将启动项更新为当前位置。退出程序请使用托盘右键菜单中的“退出”。

“开机自启登记”的勾选仅表示当前 EXE 已登记到当前用户的 Run 启动项，不代表 Windows 一定允许它启动。若在任务管理器或 Windows 设置中禁用了启动项，登记仍会保留。可通过菜单中的“Windows 启动应用设置”查看并手动启用 `ThemeToggleLite`；取消勾选会删除本程序的登记。

## 从源码构建

需要 CMake 3.20+、Ninja，以及 MinGW-w64 GCC 或 MSVC。使用 MinGW-w64 时，在 PowerShell 中运行：

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

生成的程序位于 `build\ThemeToggle.exe`。重新构建前，请先从托盘菜单退出正在运行的程序。

版本号统一在 `CMakeLists.txt` 的 `project(... VERSION ...)` 中维护，构建时同步到“关于”窗口和 EXE 文件属性。

## 设计

使用 C++20、Win32 API 和 CMake，不依赖 Qt、Electron 或 .NET Runtime。开机自启通过当前用户的 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` 启用，无需管理员权限。

## 许可证

[MIT](LICENSE)
