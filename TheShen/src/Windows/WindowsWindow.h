#pragma once

#include "Core/Window.h"
#include <GLFW/glfw3.h>

class WindowsWindow :public Window
{
public:
	WindowsWindow(const WindowProps& props);
	virtual ~WindowsWindow() {};

	inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }

	void OnUpdate() override;


	virtual void* GetNativeWindow() const { return m_Window; }
private:
	virtual void Init(const WindowProps& props);

	static void ki_windowresizefun(GLFWwindow* glfwwin, int width, int height);
	static void ki_windowclosefun(GLFWwindow* glfwwin);

	static void ki_mousebuttonfun(GLFWwindow* glfwwin, int button, int action, int mods);
	static void ki_mousescrolledfun(GLFWwindow* glfwwin, double xOffset, double yOffset);
	static void ki_mousesmovefun(GLFWwindow* glfwwin, double xPos, double yPos);

	static void ki_keyfun(GLFWwindow* glfwwin, int key, int scancode, int action, int mods);

	void SetVSync(bool enabled) override;
	bool IsVSync() const override;



private:
	GLFWwindow* m_Window;

	struct WindowData
	{
		std::string Title;
		unsigned int Width, Height;
		bool VSync;
		EventCallbackFn EventCallback;
	};

	WindowData m_Data;
};