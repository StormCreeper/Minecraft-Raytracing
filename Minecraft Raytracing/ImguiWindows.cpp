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

    if (renderer.drawWindows) {
        DebugWindowCubeDimensions(renderer);
        DebugWindowDebugInfo(renderer);
    }
}


void DebugWindowCubeDimensions(Renderer& R) {

    ImGui::Begin("Parameters");

    ImGui::Text("Scene parameters");
    
    ImGui::SliderInt3("Scene dimension", R.vt.dim, 16, 2048);
    ImGui::SliderInt("Block Scale", &R.blockScale, 16, 256);
    ImGui::SliderInt("Tool", &R.player.tool, 1, 15);


    ImGui::Text("Water");

    ImGui::SliderFloat("Intensity", &R.wt.intensity, 0.0f, 2.0f);
    ImGui::SliderFloat2("Speed", R.wt.speed, -1.0f, 1.0f);
    ImGui::SliderFloat("Diffuse", &R.wt.diffuse, 0.0f, 2.0f);
    ImGui::SliderFloat("Reflection", &R.wt.reflection, 0.0f, 1.0f);
    ImGui::SliderFloat("Refraction", &R.wt.refraction, 0.0f, 1.0f);
    ImGui::SliderFloat("IOR", &R.wt.ior, 1.0f, 2.0f);

    ImGui::SliderFloat("Air absorbance", &R.air_absorbance, 0.0f, 5.0f);
    ImGui::SliderFloat("Water absorbance", &R.water_absorbance, 0.0f, 5.0f);

    ImGui::Text("Colors");

    float green[3] = { R.color_green.r, R.color_green.g, R.color_green.b };
    float brown[3] = { R.color_brown.r, R.color_brown.g, R.color_brown.b };

    ImGui::ColorPicker3("Green", green);
    ImGui::ColorPicker3("Brown", brown);

    R.color_green = glm::vec3(green[0], green[1], green[2]);
    R.color_brown = glm::vec3(brown[0], brown[1], brown[2]);

    ImGui::End();

}

void DebugWindowDebugInfo(Renderer& R) {

    ImGui::Begin("Debug Info");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Position : (%.1f, %.1f, %.1f)", R.player.position.x, R.player.position.y, R.player.position.z);

    static char buf1[64] = "";
    ImGui::InputText("", buf1, 64);
    ImGui::SameLine();
    if (ImGui::Button("Save to the file")) {
        SaveManager::save("test.txt", *&R.player);
    }
    if (ImGui::Button("Load from file")) {
        SaveManager::load("test.txt", R.player, &R.vt);
    }
    ImGui::End();

    ImGui::Begin("Shader");
    ImGui::Checkbox("Fragment shader", &R.rfp);
    ImGui::Checkbox("Compute shader", &R.rcp);
    if (ImGui::Button("Reload Shaders")) {
        if (R.rcp) {
            //R.vt.init(R.vt.dim[0]);
            R.vt.generateTextureComputed();
        }
        if(R.rfp) R.reloadShaders();

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
        SettingsManager::load("default.stg");

    SettingsManager::ImGuiWindow();
    
    ImGui::End();
}