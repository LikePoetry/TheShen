#pragma once
#include "Events/Event.h"

struct WindowProps
{
	std::string Title;
	uint32_t Width;
	uint32_t Height;

	WindowProps(const std::string& title = "TheShen",
		uint32_t width = 900,
		uint32_t height = 600)
		:Title(title), Width(width), Height(height)
	{

	}
};


class Window
{
public:
	using EventCallbackFn = std::function<void(Event&)>;

	virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

	static Window* Create(const WindowProps& props = WindowProps());

	virtual	~Window() {};

	virtual void OnUpdate() = 0;

	virtual void SetVSync(bool enabled) = 0;
	virtual bool IsVSync() const = 0;

	virtual void* GetNativeWindow() const = 0;

};
