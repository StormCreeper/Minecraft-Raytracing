#include "ImguiWindows.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

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
    
    ImGui::SliderInt3("Scene dimension", R.vt.dim, 16, 1024);

    ImGui::Text("Water");

    ImGui::SliderFloat("Intensity", &R.wt.intensity, 0.0f, 2.0f);
    ImGui::SliderFloat2("Speed", R.wt.speed, -10.0f, 10.0f);
    ImGui::SliderFloat("Diffuse", &R.wt.diffuse, 0.0f, 2.0f);
    ImGui::SliderFloat("Reflection", &R.wt.reflection, 0.0f, 1.0f);
    ImGui::SliderFloat("Refraction", &R.wt.refraction, 0.0f, 1.0f);
    ImGui::SliderFloat("IOR", &R.wt.ior, 1.0f, 2.0f);

    ImGui::SliderFloat("Air absorbance", &R.air_absorbance, 0.0f, 5.0f, nullptr, 3);
    ImGui::SliderFloat("Water absorbance", &R.water_absorbance, 0.0f, 5.0f, nullptr, 1);

    ImGui::End();

}

void DebugWindowDebugInfo(Renderer& R) {

    ImGui::Begin("Debug Info");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    ImGui::Begin("Shader");
    ImGui::Checkbox("Fragment shader", &R.rfp);
    ImGui::Checkbox("Compute shader", &R.rcp);
    if (ImGui::Button("Reload Shaders")) {
        if (R.rcp) {
            R.vt.init();
            R.vt.generateTextureComputed();
        }
        if(R.rfp) R.reloadShaders();

        std::cout << "Shaders reloaded" << std::endl;
    }
    if (ImGui::Button("Regenerate voxel map")) {
        R.vt.generateTextureComputed();
    }

    ImGui::Text(R.shader_error);
    ImGui::End();
}