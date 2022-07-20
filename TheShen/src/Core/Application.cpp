#include "Application.h"
#include "Renderer/Renderer.h"

static App* pApp = nullptr;

Application* Application::s_Instance = nullptr;

Application::Application(int argc, char** argv, App* app) {
	SHEN_CORE_ASSERT(!s_Instance, "Application already exists!");
	s_Instance = this;

	WindowProps windowProp;
	windowProp.Height = 600;
	windowProp.Width = 900;
	windowProp.Title = app->GetName();

	app->mSettings.mHeight = windowProp.Height;
	app->mSettings.mWidth = windowProp.Width;

	m_Window = std::unique_ptr<Window>(Window::Create(windowProp));

	m_Window->SetEventCallback(SHEN_BIND_EVENT_FN(OnEvent));

	// 初始化子系统
	InitBaseSubSystems();

	pApp = app;
	app->Init();

	app->Load();

	m_ImGuiLayer = new ImGuiLayer();
	PushOverlay(m_ImGuiLayer);
}

Application::~Application() 
{

}

/// <summary>
/// 初始化子系统
/// </summary>
/// <returns></returns>
bool Application::InitBaseSubSystems()
{
	extern bool platformInitUserInterface();
	if (!platformInitUserInterface())
	{
		return false;
	}
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

		pApp->Draw();
	}

}

void Application::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	//SHEN_CORE_INFO("{0}", e);
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
	pApp->mSettings.mWidth = e.GetWidth();
	pApp->mSettings.mHeight = e.GetHeight();
	return true;
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