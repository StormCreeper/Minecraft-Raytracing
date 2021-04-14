#include "ImguiWindows.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void ImguiWindowsManager::ImguiInit(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
    
}

void ImguiWindowsManager::ImguiRender() {

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void ImguiWindowsManager::ImguiCleanup() {

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}
void ImguiWindowsManager::ImguiCreateWindows(Renderer &renderer) {

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    DebugWindowCubeDimensions(renderer);
    DebugWindowDebugInfo(renderer);

}


void DebugWindowCubeDimensions(Renderer& R) {

    ImGui::Begin("Parameters");

    ImGui::Text("Scene parameters");

    ImGui::End();

}

void DebugWindowDebugInfo(Renderer& R) {

    ImGui::Begin("Debug Info");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    ImGui::Begin("Shader");
    if (ImGui::Button("Reload Shaders")) {
        R.reloadShaders();
    }

    ImGui::Text(R.shader_error);
    ImGui::End();
}