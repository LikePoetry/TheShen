#include "WindowsWindow.h"

#include "Events/MouseEvent.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Core/Log.h"
#include "Core/Base.h"


static bool s_GLFWInitialized = false;

Window* Window::Create(const WindowProps& props) {
	return new WindowsWindow(props);
}

WindowsWindow::WindowsWindow(const WindowProps& props) {
	Init(props);
}

void WindowsWindow::Init(const WindowProps& props) {
	m_Data.Title = props.Title;
	m_Data.Width = props.Width;
	m_Data.Height = props.Height;

	SHEN_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

	if (!s_GLFWInitialized)
	{
		int success = glfwInit();

		SHEN_CORE_ASSERT(success, "Could not intialize GLFW!");

		s_GLFWInitialized = true;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_Window, &m_Data);
	SetVSync(true);

	glfwSetWindowSizeCallback(m_Window, ki_windowresizefun);
	glfwSetWindowCloseCallback(m_Window, ki_windowclosefun);

	glfwSetMouseButtonCallback(m_Window, ki_mousebuttonfun);
	glfwSetScrollCallback(m_Window, ki_mousescrolledfun);
	glfwSetCursorPosCallback(m_Window, ki_mousesmovefun);

	glfwSetKeyCallback(m_Window, ki_keyfun);
}

void WindowsWindow::ki_mousebuttonfun(GLFWwindow* glfwwin, int button, int action, int mods)
{
	WindowData& data = *(WindowData*)glfwGetWindowUserPointer(glfwwin);

	switch (action)
	{
	case GLFW_PRESS: {
		MouseButtonPressedEvent event(button);
		data.EventCallback(event);
		break;
	}
	case GLFW_RELEASE:
	{
		MouseButtonReleasedEvent event(button);
		data.EventCallback(event);
		break;
	}
	}
}

void WindowsWindow::ki_windowclosefun(GLFWwindow* glfwwin)
{
	WindowData& data = *(WindowData*)glfwGetWindowUserPointer(glfwwin);
	WindowCloseEvent event;
	data.EventCallback(event);
}

void WindowsWindow::ki_mousescrolledfun(GLFWwindow* glfwwin, double xOffset, double yOffset)
{
	WindowData& data = *(WindowData*)glfwGetWindowUserPointer(glfwwin);

	MouseScrolledEvent event((float)xOffset, (float)yOffset);
	data.EventCallback(event);
}

void WindowsWindow::ki_mousesmovefun(GLFWwindow* glfwwin, double xPos, double yPos)
{
	WindowData& data = *(WindowData*)glfwGetWindowUserPointer(glfwwin);

	MouseMovedEvent event((float)xPos, (float)yPos);
	data.EventCallback(event);
}

void WindowsWindow::ki_keyfun(GLFWwindow* glfwwin, int key, int scancode, int action, int mods)
{
	WindowData& data = *(WindowData*)glfwGetWindowUserPointer(glfwwin);

	switch (action)
	{
	case GLFW_PRESS:
	{
		KeyPressedEvent event((KeyCode)key, 0);
		data.EventCallback(event);
		break;
	}
	case GLFW_RELEASE:
	{
		KeyReleasedEvent event((KeyCode)key);
		data.EventCallback(event);
		break;
	}
	case GLFW_REPEAT:
	{
		KeyPressedEvent event((KeyCode)key, 1);
		data.EventCallback(event);
		break;
	}
	}
}

void WindowsWindow::ki_windowresizefun(GLFWwindow* glfwwin, int width, int height)
{
	WindowData& data = *(WindowData*)glfwGetWindowUserPointer(glfwwin);
	data.Width = width;
	data.Height = height;

	WindowResizeEvent event(width, height);
	data.EventCallback(event);
}


void WindowsWindow::OnUpdate()
{
	glfwPollEvents();
}

void WindowsWindow::SetVSync(bool enabled)
{
	if (enabled)
		glfwSwapInterval(1);
	else
		glfwSwapInterval(0);

	m_Data.VSync = enabled;
}

bool WindowsWindow::IsVSync() const
{
	return m_Data.VSync;
}