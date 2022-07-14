#pragma once
#include "Renderer/RendererAPI.h"

class Renderer
{
public:
	static bool Init();

	static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

private:

};