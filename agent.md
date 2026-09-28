# ThemeToggle Lite

## 1. 项目定位

ThemeToggle Lite 是一个面向 Windows 10 / Windows 11 的极简主题切换工具。

核心目标：

> 用尽可能少的资源，实现 Windows 深色 / 浅色模式的一键切换。

第一版采用：

- C++
- Win32 API
- CMake
- VSCode
- Codex 辅助开发

最终发布为：

```text
ThemeToggle.exe
```

不依赖：

- Python
- PowerShell
- .NET Runtime
- Electron
- Qt

目标是做成真正轻量的 Windows 原生程序。

---

# 2. 第一版功能

版本：

```text
v0.1.0
```

必须实现：

1. 启动后不显示主窗口
2. 常驻系统托盘
3. 左键托盘图标切换主题
4. 深色 → 浅色
5. 浅色 → 深色
6. 同时修改：
   - AppsUseLightTheme
   - SystemUsesLightTheme
7. 修改后通知 Windows 刷新主题
8. 托盘图标跟随当前主题变化
9. 右键菜单：
   - 切换主题
   - 关于
   - 退出
10. 单实例运行
11. 无管理员权限运行

---

# 3. 暂时不做

v0.1 不加入：

```text
自动日出日落
定时主题切换
复杂设置页面
联网
自动更新
云同步
多语言系统
Windows Service
安装程序
```

保持：

```text
small
fast
native
portable
```

---

# 4. 技术栈

## C++

建议：

```text
C++20
```

编译器可以使用：

```text
MinGW-w64 GCC
```

或者：

```text
MSVC
```

如果你目前主要用 Scoop 管理开发环境，推荐先使用：

```text
MinGW-w64 + CMake + Ninja
```

VSCode：

```text
VSCode
├── C/C++
├── CMake Tools
└── Codex
```

构建系统：

```text
CMake
```

项目不引入第三方 GUI 库。

全部使用 Windows API。

---

# 5. 使用到的 Windows API

## Registry

主题配置位置：

```text
HKEY_CURRENT_USER
\Software
\Microsoft
\Windows
\CurrentVersion
\Themes
\Personalize
```

主要键：

```text
AppsUseLightTheme
SystemUsesLightTheme
```

含义：

```text
1 = Light
0 = Dark
```

因为使用：

```text
HKEY_CURRENT_USER
```

所以不需要管理员权限。

---

## Registry API

使用：

```cpp
RegOpenKeyExW()
RegQueryValueExW()
RegSetValueExW()
RegCloseKey()
```

建议封装为：

```cpp
bool IsLightTheme();

bool SetTheme(bool light);

bool ToggleTheme();
```

---

# 6. 通知 Windows 主题改变

只修改 Registry 不够。

修改完成以后需要通知 Windows：

```cpp
SendMessageTimeoutW(
    HWND_BROADCAST,
    WM_SETTINGCHANGE,
    0,
    reinterpret_cast<LPARAM>(L"ImmersiveColorSet"),
    SMTO_ABORTIFHUNG,
    100,
    nullptr
);
```

这样：

```text
Explorer
Taskbar
Start Menu
支持系统主题的应用
```

可以更快响应主题变化。

---

# 7. 托盘程序

使用：

```cpp
Shell_NotifyIconW()
```

数据结构：

```cpp
NOTIFYICONDATAW
```

需要支持：

```text
NIM_ADD
NIM_MODIFY
NIM_DELETE
```

托盘消息建议：

```cpp
#define WM_TRAYICON (WM_USER + 1)
```

---

# 8. 托盘交互

## 左键

```text
Left Click
    ↓
ToggleTheme()
```

例如：

```cpp
case WM_LBUTTONUP:
    ToggleTheme();
    UpdateTrayIcon();
    break;
```

---

## 右键

弹出菜单：

```text
ThemeToggle Lite
────────────────
切换主题
关于
退出
```

对应：

```cpp
CreatePopupMenu()
AppendMenuW()
TrackPopupMenu()
DestroyMenu()
```

---

# 9. 图标设计

准备两个图标：

```text
light.ico
dark.ico
```

建议视觉：

```text
Light Mode
☀

Dark Mode
☾
```

主题状态：

```text
当前为 Light
→ 显示太阳

当前为 Dark
→ 显示月亮
```

目录：

```text
resources/
├── light.ico
└── dark.ico
```

通过：

```text
resource.rc
```

编译进入 exe。

---

# 10. 单实例

防止用户连续启动：

```text
ThemeToggle.exe
ThemeToggle.exe
ThemeToggle.exe
```

出现多个托盘图标。

使用：

```cpp
CreateMutexW()
```

例如：

```cpp
CreateMutexW(
    nullptr,
    TRUE,
    L"ThemeToggleLite.Singleton"
);
```

如果：

```cpp
GetLastError() == ERROR_ALREADY_EXISTS
```

直接退出。

---

# 11. 推荐项目结构

```text
ThemeToggle/
│
├── CMakeLists.txt
│
├── README.md
│
├── LICENSE
│
├── AGENTS.md
│
├── .gitignore
│
├── src/
│   ├── main.cpp
│   ├── theme.cpp
│   ├── theme.h
│   ├── tray.cpp
│   └── tray.h
│
├── resources/
│   ├── resource.rc
│   ├── resource.h
│   ├── light.ico
│   └── dark.ico
│
└── build/
```

不要把所有代码塞进：

```text
main.cpp
```

推荐职责：

```text
main.cpp
    程序入口
    消息循环
    单实例

theme.cpp
    Registry
    获取主题
    设置主题
    Toggle

tray.cpp
    Tray Icon
    Tray Menu
    Tray Update
```

---

# 12. 类 / 模块设计

建议不急着上复杂 OOP。

第一版用简单模块即可。

## theme.h

```cpp
#pragma once

enum class Theme {
    Light,
    Dark
};

Theme GetCurrentTheme();

bool SetTheme(Theme theme);

bool ToggleTheme();
```

---

## tray.h

```cpp
#pragma once

#include <windows.h>

bool CreateTrayIcon(HWND hwnd);

void UpdateTrayIcon(HWND hwnd);

void RemoveTrayIcon();

void ShowTrayMenu(HWND hwnd);
```

---

# 13. main.cpp 职责

main.cpp 只处理：

```text
WinMain
↓
Single Instance
↓
Register Window Class
↓
Create Hidden Window
↓
Create Tray Icon
↓
Message Loop
```

程序架构：

```text
WinMain
 │
 ├── CreateMutex
 │
 ├── RegisterClass
 │
 ├── CreateWindow
 │
 ├── CreateTrayIcon
 │
 └── MessageLoop
          │
          ↓
       WndProc
          │
          ├── Tray Left Click
          │        ↓
          │    ToggleTheme
          │
          ├── Tray Right Click
          │        ↓
          │    Popup Menu
          │
          └── Exit
```

---

# 14. CMake

基础：

```cmake
cmake_minimum_required(VERSION 3.20)

project(
    ThemeToggle
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    ThemeToggle
    WIN32

    src/main.cpp
    src/theme.cpp
    src/tray.cpp

    resources/resource.rc
)

target_include_directories(
    ThemeToggle
    PRIVATE
    src
)

target_link_libraries(
    ThemeToggle
    PRIVATE
    user32
    shell32
    advapi32
)
```

其中：

```text
WIN32
```

非常重要。

这样程序启动时不会弹出：

```text
cmd.exe
```

控制台窗口。

---

# 15. VSCode 工作方式

打开：

```powershell
code ThemeToggle
```

推荐目录中保留：

```text
.vscode/
```

但尽量不要写大量 IDE 特定配置。

核心构建还是：

```powershell
cmake -S . -B build -G Ninja
```

然后：

```powershell
cmake --build build
```

运行：

```powershell
.\build\ThemeToggle.exe
```

---

# 16. Codex 开发方式

不要直接对 Codex 说：

```text
帮我写一个 Windows 软件
```

这样很容易一次生成太多代码。

采用阶段式开发。

---

# Phase 1

让 Codex 初始化工程。

Prompt：

```text
We are building ThemeToggle Lite, a minimal native Windows
dark/light theme switcher.

Tech stack:

- C++20
- Win32 API
- CMake
- No Qt
- No .NET
- No third-party GUI framework

Create the initial project structure:

src/
resources/

Implement only:

1. WinMain
2. hidden Win32 window
3. Windows message loop
4. single-instance protection using CreateMutexW

Do not implement theme switching or tray icons yet.

Keep the code simple and readable.
```

验收：

```text
编译成功
运行无窗口
运行第二次不会产生第二个进程
```

---

# Phase 2

Prompt：

```text
Implement the Windows theme module.

Create:

src/theme.h
src/theme.cpp

Requirements:

Read:

HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize

Values:

AppsUseLightTheme
SystemUsesLightTheme

Implement:

Theme GetCurrentTheme();
bool SetTheme(Theme theme);
bool ToggleTheme();

After changing the registry, broadcast WM_SETTINGCHANGE
with "ImmersiveColorSet".

Do not modify tray-related code.
```

验收：

```text
ToggleTheme()
```

能够：

```text
Dark ↔ Light
```

---

# Phase 3

实现系统托盘。

Prompt：

```text
Implement the tray icon module.

Create:

src/tray.h
src/tray.cpp

Use:

Shell_NotifyIconW

Requirements:

- tray icon appears when application starts
- tray icon is removed on exit
- left click toggles Windows theme
- right click opens a popup menu

Menu:

Toggle Theme
About
Exit

Keep tray code separate from registry/theme code.
```

---

# Phase 4

动态图标。

Prompt：

```text
Add two Windows icon resources:

light.ico
dark.ico

Use a resource.rc file.

When Windows is currently using light mode,
show the light icon.

When Windows is currently using dark mode,
show the dark icon.

After ToggleTheme(), immediately update the tray icon.

Do not dynamically load icon files from disk.
Compile them into the executable.
```

---

# Phase 5

异常处理。

处理：

```text
Registry key missing
Registry value missing
Shell_NotifyIcon failure
CreateWindow failure
CreateMutex failure
```

原则：

普通用户错误不要：

```text
assert()
```

应该：

```text
return false
```

必要时：

```text
MessageBoxW()
```

---

# 17. Codex 项目规则

建议根目录创建：

```text
AGENTS.md
```

内容：

```markdown
# ThemeToggle Lite Development Rules

ThemeToggle Lite is a minimal Windows native utility.

## Stack

- C++20
- Win32 API
- CMake

Do not introduce:

- Qt
- Electron
- .NET
- Boost
- third-party GUI frameworks

unless explicitly requested.

## Architecture

Keep responsibilities separated:

main.cpp:
- application lifecycle
- message loop
- single-instance handling

theme.cpp:
- Windows registry theme state
- theme switching
- WM_SETTINGCHANGE notification

tray.cpp:
- notification area icon
- tray event handling
- context menu

## Coding style

Prefer:

- RAII where practical
- Unicode Win32 APIs
- W suffix APIs:
  - CreateWindowExW
  - RegOpenKeyExW
  - MessageBoxW

Avoid:

- global mutable state when unnecessary
- excessive abstraction
- unnecessary classes
- premature framework-style architecture

## Dependencies

Prefer Windows APIs.

Avoid adding dependencies for functionality that Win32 already provides.

## Changes

Before modifying multiple modules:

1. inspect existing code
2. identify affected files
3. make the smallest change necessary
4. build the project
5. report compiler errors if any

Never rewrite unrelated working code.

## Build

Use:

cmake -S . -B build -G Ninja

cmake --build build

Target:

Windows 10 / Windows 11 x64.
```

---

# 18. v0.1 验收标准

完成后逐项测试：

```text
[ ] 程序启动无 Console
[ ] 托盘出现图标
[ ] 左键切换 Light → Dark
[ ] 左键切换 Dark → Light
[ ] Taskbar 更新
[ ] Explorer 更新
[ ] 支持主题的软件响应
[ ] 图标自动更新
[ ] 右键菜单正常
[ ] About 正常
[ ] Exit 正常
[ ] 第二次启动不会出现两个托盘图标
[ ] 普通用户可运行
[ ] 无需管理员权限
[ ] EXE 可单独复制运行
```

---

# 19. v0.2

第一版稳定后，再增加：

```text
Global Hotkey
```

例如：

```text
Ctrl + Alt + D
```

使用：

```cpp
RegisterHotKey()
```

同时加入：

```text
开机启动
```

---

# 20. v0.3

增加：

```text
Settings
```

但是不要急着做完整 GUI。

可以先用：

```text
右键菜单
```

实现：

```text
✓ Switch Apps
✓ Switch System
✓ Start with Windows
```

---

# 21. v0.4

再考虑：

```text
Auto Theme
```

例如：

```text
07:00 Light
19:00 Dark
```

或者：

```text
Sunrise / Sunset
```

这时再考虑：

```text
配置文件
```

例如：

```text
config.json
```

---

# 22. 第一阶段最终目标

项目最终应该非常简单：

```text
ThemeToggle.exe
```

双击：

```text
Tray
```

左键：

```text
Light
  ↓
Dark
```

再次左键：

```text
Dark
  ↓
Light
```

整个程序核心代码尽量控制在：

```text
500～1000 lines C++
```

第一版重点不是功能多，而是：

```text
Native
Small
Fast
Stable
Maintainable
```

这也是 ThemeToggle Lite 与 Auto Dark Mode 这类大型工具最大的区别。