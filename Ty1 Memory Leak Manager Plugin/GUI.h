#pragma once
#include <string>

class GUI
{
public:
	static inline bool init;
	static inline bool isShown;
	static inline bool extraInfoShown;
	static void Initialize();
	static void DrawUI();
	static long GetMemoryUsage();
	static bool ImGuiWantCaptureMouse();
	static bool WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};