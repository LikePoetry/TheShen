#include "ImGuiLayer.h"

#include "Core/Application.h"

#include "imgui.h"
#include "example/imgui_impl_glfw.h"
#include "example/imgui_impl_vulkan.h"

ImGuiLayer::ImGuiLayer()
	: Layer("ImGuiLayer")
{

}

void ImGuiLayer::OnAttach()
{

}

void ImGuiLayer::OnDetach()
{
}

void ImGuiLayer::OnEvent(Event& e)
{
	if (m_BlockEvents)
	{
	}
}


void ImGuiLayer::Begin()
{

}

void ImGuiLayer::End()
{

}

