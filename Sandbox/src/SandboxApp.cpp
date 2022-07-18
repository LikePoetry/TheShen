#include <TheShen.h>
#include "Core/App.h"
#include "TestLayer.h"
#include "Renderer/Renderer.h"


Renderer* pRenderer = NULL;
Queue* pGraphicsQueue = NULL;

SwapChain* pSwapChain = NULL;

std::vector<Texture> pTextures;

RenderPass* pRenderPass = NULL;

class Sandbox :public App
{
public:
	bool Init() override
	{
		//初始化Instance 到 LogicalDevice
		RendererDesc settings;
		memset(&settings, 0, sizeof(settings));

		SwapChainDesc swapChainDesc = {};
		swapChainDesc.mWindow = Application::Get().GetNativeWindow();
		swapChainDesc.mHeight = mSettings.mHeight;
		swapChainDesc.mWidth = mSettings.mWidth;

		initRenderer(GetName(), &settings, &pRenderer, &swapChainDesc, &pSwapChain, pTextures);
		if (!pRenderer)
		{
			return false;
		}

		//添加图形队列
		QueueDesc queueDesc = {};
		queueDesc.mType = QUEUE_TYPE_GRAPHICS;
		queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
		addQueue(pRenderer, &queueDesc, &pGraphicsQueue);

		//添加渲染通道
		RenderPassDesc renderPassDesc = {};
		renderPassDesc.pColorFormats = pSwapChain->pDesc->mImageFormat;
		addRenderPass(pRenderer, &renderPassDesc, &pRenderPass);

		//Application::Get().PushLayer(new TestLayer());
		return 0;
	}

	bool Load() override
	{
		return true;
	}

	const char* GetName() { return "TheShen"; }

private:

};


DEFINE_APPLICATION_MAIN(Sandbox)