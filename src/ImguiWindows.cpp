#include "ImguiWindows.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include "SaveManager.h"
#include "SettingsManager.h"

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

    DrawWindows(renderer);
}

void DrawWindows(Renderer& R) {

    if (!R.b_paused) return;

    ImGui::Begin("Debug Info");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Position : (%.1f, %.1f, %.1f)", R.player.position.x, R.player.position.y, R.player.position.z);

    static std::string buf = "test.txt";

    //ImGui::InputText("", &buf, );
    ImGui::SameLine();
    if (ImGui::Button("Save to the file")) {
        SaveManager::save("Data/test.txt", R.player);
    }
    if (ImGui::Button("Load from file")) {
        SaveManager::load("Data/test.txt", R.player, &R.vt);
    }
    ImGui::End();

    ImGui::Begin("Shader");

    static bool rfp = false;
    static bool rcp = false;

    ImGui::Checkbox("Fragment shader", &rfp);
    ImGui::Checkbox("Compute shader", &rcp);
    if (ImGui::Button("Reload Shaders")) {
        if (rcp) {
            //R.vt.init(R.vt.dim[0]);
            R.vt.generateTextureComputed();
        }
        if(rfp) R.reloadShaders();

        std::cout << "Shaders reloaded" << std::endl;
    }
    if (ImGui::Button("Regenerate voxel map")) {
        R.vt.generateTextureComputed();
    }
    ImGui::SliderInt("Speed mod", &R.player.speed_mod, -10, 10);

    ImGui::Text(R.shader_error);
    ImGui::End();

    ImGui::Begin("Settings");
    
    if (ImGui::Button("Load default settings"))
        SettingsManager::load("Data/default.stg");
    if (ImGui::Button("Save settings"))
        SettingsManager::save("Data/save.stg");

    SettingsManager::ImGuiWindow();
    
    ImGui::End();
}