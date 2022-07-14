#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

class IRenderer
{
public:
	static void Init();


private:

};