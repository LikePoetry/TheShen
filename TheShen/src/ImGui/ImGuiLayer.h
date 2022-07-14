#pragma once
#include "Core/Layer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class ImGuiLayer :public Layer {
public:
	ImGuiLayer();
	~ImGuiLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(Event& e) override;

	void Begin();
	void End();

	void BlockEvents(bool block) { m_BlockEvents = block; }
private:
	bool m_BlockEvents = true;

	GLFWwindow* window;
private:
};