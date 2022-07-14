#pragma once
#include "Renderer/RendererAPI.h"

class RenderCommand
{
public:
	static bool Init()
	{
		return s_RendererAPI->Init();
	}


private:
	static Scope<RendererAPI> s_RendererAPI;
};