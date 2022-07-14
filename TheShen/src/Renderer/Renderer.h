#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/// <summary>
/// 渲染初始化参数描述
/// </summary>
typedef struct RendererDesc
{

}RendererDesc;

/// <summary>
/// 渲染初始化内容
/// </summary>
typedef struct Renderer
{
	VkInstance							pVkInstance;
	VkPhysicalDevice					pVkActiveGPU;
	VkDevice							pVkDevice;
} Renderer;

typedef struct SwapChain
{
	VkSurfaceKHR   pVkSurface;
} SwapChain;

void initRenderer(const char* appName, const RendererDesc* pSettings, Renderer** ppRenderer);