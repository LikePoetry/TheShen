#include "Renderer/GraphicsContext.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererAPI.h"


Scope<GraphicsContext> GraphicsContext::Create(void* window)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::None:    SHEN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	}

	SHEN_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}
