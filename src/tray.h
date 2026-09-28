#pragma once

#include <windows.h>

constexpr UINT kTrayIconMessage = WM_APP + 1;

bool CreateTrayIcon(HWND window);

bool UpdateTrayIcon(HWND window);

void RemoveTrayIcon(HWND window);

void HandleTrayNotification(HWND window, LPARAM event);

void ShowTrayMenu(HWND window);
