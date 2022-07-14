#include "RendererAPI.h"

RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

Scope<RendererAPI> RendererAPI::Create()
{
	switch (s_API)
	{
	case RendererAPI::API::None:    SHEN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		break;
	}

	SHEN_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}
