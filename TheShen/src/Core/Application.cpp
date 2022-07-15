#include "Application.h"
#include "Renderer/Renderer.h"

Application* Application::s_Instance = nullptr;
Application::Application(int argc, char** argv, App* app) {
	SHEN_CORE_ASSERT(!s_Instance, "Application already exists!");
	s_Instance = this;


	m_Window = std::unique_ptr<Window>(Window::Create());

	m_Window->SetEventCallback(SHEN_BIND_EVENT_FN(OnEvent));

	app->Init();

	app->Load();

	m_ImGuiLayer = new ImGuiLayer();
	PushOverlay(m_ImGuiLayer);
}

Application::~Application() {

}

void Application::Run() {
	while (m_Running)
	{
		float time = (float)glfwGetTime();
		Timestep timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;

		if (!m_Minimized)
		{
			{

				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timestep);
			}

			m_ImGuiLayer->Begin();
			{

				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
		}

		m_Window->OnUpdate();
	}

}

void Application::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	SHEN_CORE_INFO("{0}", e);
	dispatcher.Dispatch<WindowCloseEvent>(SHEN_BIND_EVENT_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(SHEN_BIND_EVENT_FN(Application::OnWindowResize));
	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
	{
		if (e.Handled)
			break;
		(*it)->OnEvent(e);
	}
}

void Application::PushLayer(Layer* layer)
{
	m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

void Application::PushOverlay(Layer* layer)
{
	m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{

	return false;
}

void Application::Close()
{
	m_Running = false;
}

int CreateApplication(int argc, char** argv, App* app) {
	Log::Init();
	Application* application = new Application(argc, argv, app);
	application->Run();
	delete application;
	return 0;
}