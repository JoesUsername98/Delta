#include "ApplicationOpenGL.h"
// Use system OpenGL headers directly on Windows
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <algorithm>

extern bool g_ApplicationRunning;

namespace Walnut {

	static Application* s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		s_Instance = this;
	}

	Application::~Application()
	{
		Shutdown();
	}

	Application& Application::Get()
	{
		return *s_Instance;
	}

	void Application::Run()
	{
		Init();

		while (!glfwWindowShouldClose(m_WindowHandle))
		{
			// Poll events
			glfwPollEvents();

			// Update layers
			for (auto& layer : m_LayerStack)
				layer->OnUpdate(0.0f); // Simple fixed timestep for now

			// Render
			int display_w, display_h;
			glfwGetFramebufferSize(m_WindowHandle, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			// Start the ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// Render UI layers
			for (auto& layer : m_LayerStack)
				layer->OnUIRender();

			// Render ImGui
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(m_WindowHandle);
		}

		g_ApplicationRunning = false;
	}

	void Application::Init()
	{
		// Initialize GLFW
		if (!glfwInit())
		{
			std::cerr << "Failed to initialize GLFW" << std::endl;
			return;
		}

		// GL 3.3 + GLSL 330
		const char* glsl_version = "#version 330";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Create window
		m_WindowHandle = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), NULL, NULL);
		if (m_WindowHandle == NULL)
		{
			std::cerr << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return;
		}
		glfwMakeContextCurrent(m_WindowHandle);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_WindowHandle, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		// Attach layers
		for (auto& layer : m_LayerStack)
			layer->OnAttach();
	}

	void Application::Shutdown()
	{
		for (auto& layer : m_LayerStack)
			layer->OnDetach();

		m_LayerStack.clear();

		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	float Application::GetTime()
	{
		return (float)glfwGetTime();
	}

}