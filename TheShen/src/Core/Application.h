#pragma once
#include "Window.h"
#include "Log.h"
#include "LayerStack.h"
#include "Timestep.h"
#include "Events/Event.h"

#include "Events/ApplicationEvent.h"

#include "ImGui/ImGuiLayer.h"
#include "LayerStack.h"
#include "Core/App.h"


class Application
{
public:
	Application(int argc, char** argv, App* app);
	virtual ~Application();
	void Close();


	void OnEvent(Event& e);

	void PushLayer(Layer* layer);
	void PushOverlay(Layer* layer);

	ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

	Window& GetWindow() { return *m_Window; }
	static Application& Get() { return *s_Instance; }

	void Run();
private:

	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);

	std::unique_ptr<Window> m_Window;

	bool m_Running = true;
	bool m_Minimized = false;
	float m_LastFrameTime = 0.0f;

private:
	static Application* s_Instance;
	ImGuiLayer* m_ImGuiLayer;
	LayerStack m_LayerStack;

};