#pragma once
#include "Core/Base.h"
#include <memory>

class RendererAPI
{
public:
	enum class API
	{
		None = 0,
		Vulkan = 1
	};

public:
	virtual ~RendererAPI() = default;

	virtual bool Init() = 0;
	virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

	virtual void Clear() = 0;

	static API GetAPI() { return s_API; }
	static Scope<RendererAPI> Create();
private:
	static API s_API;
};