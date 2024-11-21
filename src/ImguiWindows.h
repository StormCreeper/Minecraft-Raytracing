#pragma once
#include "Renderer.h"

void DrawWindows(Renderer& R);

class ImguiWindowsManager {
public:
    static void ImguiInit(GLFWwindow* window);
    static void ImguiRender();
    static void ImguiCleanup();
    static void ImguiCreateWindows(Renderer& renderer);
};