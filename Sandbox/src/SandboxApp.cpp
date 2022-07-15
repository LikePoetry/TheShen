#include <TheShen.h>
#include "Core/App.h"
#include "TestLayer.h"
#include "Renderer/Renderer.h"


Renderer* pRenderer = NULL;
Queue* pGraphicsQueue = NULL;

SwapChain* pSwapChain = NULL;


class Sandbox :public App
{
public:
	bool Init() override
	{
		//初始化Instance 到 LogicalDevice
		RendererDesc settings;
		memset(&settings, 0, sizeof(settings));
		initRenderer(GetName(), &settings, &pRenderer);
		if (!pRenderer)
		{
			return false;
		}

		//添加图形队列
		QueueDesc queueDesc = {};
		queueDesc.mType = QUEUE_TYPE_GRAPHICS;
		queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
		addQueue(pRenderer, &queueDesc, &pGraphicsQueue);



		//Application::Get().PushLayer(new TestLayer());
		return 0;
	}

	bool Load() override
	{
		if (!addSwapChain())
			return false;
	}

	bool addSwapChain()
	{
		SwapChainDesc swapChainDesc = {};
		swapChainDesc.mWindow = Application::Get().GetNativeWindow();
		::addSwapChain(pRenderer, &swapChainDesc, &pSwapChain);

		return true;
	}

	const char* GetName() { return "TheShen"; }

private:

};


DEFINE_APPLICATION_MAIN(Sandbox)