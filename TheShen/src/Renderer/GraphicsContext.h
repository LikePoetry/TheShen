#pragma once
#include "Core/Base.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
class GraphicsContext
{
public:
	virtual ~GraphicsContext() = default;

	virtual bool Init() = 0;
	virtual void SwapBuffers() = 0;

	static Scope<GraphicsContext> Create(void* window);
};