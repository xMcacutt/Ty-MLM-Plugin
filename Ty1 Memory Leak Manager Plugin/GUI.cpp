#include "pch.h"
#include "GUI.h"
#include "framework.h"
#include <filesystem>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "TygerFrameworkAPI.hpp"
#include <Windows.h>
#include <iostream>
#include <Psapi.h>
#include <sstream>
#include <iomanip>
#define STB_IMAGE_IMPLEMENTATION

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool GUI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_F4) {
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            GUI::isShown = !GUI::isShown;
        else if (GUI::isShown)
            GUI::extraInfoShown = !GUI::extraInfoShown;
    }
    if (API::DrawingGUI())
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;
    return false;
}

void GUI::Initialize() {

    ImGui::CreateContext();

    //ImFontAtlas* fonts = ImGui::GetIO().Fonts;
    //fonts->Clear();

    //ImFontConfig custom_icons{};
    //custom_icons.FontDataOwnedByAtlas = false;

    //fonts->AddFontFromMemoryCompressedTTF(TyNumberFont_compressed_data, TyNumberFont_compressed_size, 40);
    //fonts->Build();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    if (API::GetTyWindowHandle() == 0) {
        API::LogPluginMessage("Error");
        return;
    }

    ImGui_ImplWin32_InitForOpenGL(API::GetTyWindowHandle());
    ImGui_ImplOpenGL3_Init();

    API::LogPluginMessage("Initialized ImGui");
    GUI::init = true;
}

//ImGui Drawing code
void GUI::DrawUI() {
    if (!isShown)
        return;

    if (!GUI::init)
        Initialize();


    HWND hwnd = (HWND)API::GetTyWindowHandle();
    RECT rect;
    if (!GetClientRect(hwnd, &rect)) {
        API::LogPluginMessage("Failed to get window size.");
        return;
    }
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    const int baseWidth = 1920;
    const int baseHeight = 1080;
    float widthScale = static_cast<float>(windowWidth) / baseWidth;
    float heightScale = static_cast<float>(windowHeight) / baseHeight;
    float uiScale = min(widthScale, heightScale);
    int iconScale = static_cast<int>(uiScale * 64);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5.0f, 5.0f });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });

    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin((API::PluginName + " Overlay").c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);

    // Get the memory usage in bytes
    long memoryUsage = GUI::GetMemoryUsage();

    // Convert memory usage to gigabytes for display (or use other units as preferred)
    float memoryUsageGB = static_cast<double>(memoryUsage) / (1024 * 1024 * 1024);
    float maxMemoryGB = 1.5f;
    ImVec4 color;

    if (memoryUsageGB < 0.7)
        color = ImVec4(0.0f, 0.6f, 0.0f, 1.0f);
    else if (memoryUsageGB < 1.2)
        color = ImVec4(0.8f, 0.6f, 0.0f, 1.0f);
    else
        color = ImVec4(0.6f, 0.0f, 0.0f, 1.0f);

    float memoryUsageFraction = memoryUsageGB / maxMemoryGB;
    ImGui::Text("Memory Usage");
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << memoryUsageGB;
    std::string memoryUsageStr = oss.str() + " GB / 1.5 GB";
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.6f)); // Dark background
    ImGui::ProgressBar(memoryUsageFraction, ImVec2(250.0f, 24.0f), memoryUsageStr.c_str());
    ImGui::PopStyleColor(2);

    if (GUI::extraInfoShown) {
        ImGui::Text("Loads Remaining: ~%d", static_cast<int>(std::round((maxMemoryGB - memoryUsageGB) / 0.035)));
    }

    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

long GUI::GetMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    return pmc.WorkingSetSize; // in gigabytes
}

bool GUI::ImGuiWantCaptureMouse() {
    return GUI::init && ImGui::GetIO().WantCaptureMouse;
}