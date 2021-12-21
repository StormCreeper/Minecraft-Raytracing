#pragma once
#include "Renderer.h"

void DebugWindowCubeDimensions(Renderer& R);
void DebugWindowDebugInfo(Renderer& R);

class ImguiWindowsManager {
public:
    static void ImguiInit(GLFWwindow* window);
    static void ImguiRender();
    static void ImguiCleanup();
    static void ImguiCreateWindows(Renderer& renderer);
};