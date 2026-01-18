#include <stdio.h>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Global variables
GLFWwindow* g_window;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void loop() {
    glfwPollEvents();
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Simple test window
    ImGui::Begin("Delta++ WebAssembly Test");
    ImGui::Text("Hello from Delta++ WebAssembly!");
    ImGui::Text("If you can see this, the basic setup is working!");
    ImGui::ColorEdit3("Background Color", (float*)&clear_color);
    ImGui::End();
    
    // Render
    ImGui::Render();
    
    int display_w, display_h;
    glfwMakeContextCurrent(g_window);
    glfwGetFramebufferSize(g_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(g_window);
}

int init_gl() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }
    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    
    g_window = glfwCreateWindow(1, 1, "Delta++ WebAssembly Test", NULL, NULL);
    if (g_window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(g_window);
    
    return 0;
}

int init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(g_window, "#canvas");
#endif
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
    
    ImGui::StyleColorsDark();
    
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    
    return 0;
}

int init() {
    if (init_gl() != 0) return 1;
    if (init_imgui() != 0) return 1;
    return 0;
}

void quit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

int main(int argc, char** argv) {
    if (init() != 0) return 1;
    
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!glfwWindowShouldClose(g_window)) {
        loop();
        glfwSwapBuffers(g_window);
    }
#endif
    
    quit();
    return 0;
}