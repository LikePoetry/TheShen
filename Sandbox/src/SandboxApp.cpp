#include <TheShen.h>
#include "Core/App.h"
#include "TestLayer.h"
#include "Renderer/Renderer.h"
#include "ImGui/UI.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

Renderer* pRenderer = NULL;
Queue* pGraphicsQueue = NULL;
SwapChain* pSwapChain = NULL;
RenderPass* pRenderPass = NULL;
Pipeline* pPipeline = NULL;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
std::vector<Texture> pTextures;
std::vector<FrameBuffer*> pFrameBuffers;
CmdPool* pCmdPool = NULL;
Cmd* pCmds[MAX_FRAMES_IN_FLIGHT] = { NULL };
Semaphore* pImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT] = { NULL };
Semaphore* pRenderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT] = { NULL };
Fence* pInFlightFences[MAX_FRAMES_IN_FLIGHT] = { NULL };
uint32_t currentFrame = 0;


class Sandbox :public App
{
public:
	bool Init() override
	{
		//初始化Instance 到 LogicalDevice
		RendererDesc settings;
		memset(&settings, 0, sizeof(settings));
		SwapChainDesc* swapChainDesc = (SwapChainDesc*)malloc(sizeof(swapChainDesc));
		swapChainDesc->mWindow = Application::Get().GetNativeWindow();
		swapChainDesc->mHeight = mSettings.mHeight;
		swapChainDesc->mWidth = mSettings.mWidth;

		initRenderer(GetName(), &settings, &pRenderer, swapChainDesc, &pSwapChain, pTextures);
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

	void createGraphicsPipeline()
	{
		ShaderDesc shaderDesc = {};
		shaderDesc.mStages = SHADER_STAGE_VERT;
		shaderDesc.pFileName = "F:/VulkanWorkarea/TheShenProject_github/TheShen/bin/Debug-windows-x86_64/Sandbox/shaders/vert.spv";
		//shaderDesc.pFileName = "E:/workarea/TheShen_github/TheShen/Sandbox/shaders/vert.spv";
		Shader* pVertShader;
		addShader(pRenderer, &shaderDesc, &pVertShader);
		shaderDesc.mStages = SHADER_STAGE_FRAG;
		shaderDesc.pFileName = "F:/VulkanWorkarea/TheShenProject_github/TheShen/bin/Debug-windows-x86_64/Sandbox/shaders/frag.spv";
		//shaderDesc.pFileName = "E:/workarea/TheShen_github/TheShen/Sandbox/shaders/frag.spv";
		Shader* pFragShader;
		addShader(pRenderer, &shaderDesc, &pFragShader);

		GraphicsPipelineDesc graphicsPinelineDesc = {};
		graphicsPinelineDesc.pColorFormats = pSwapChain->pDesc->mImageFormat;
		graphicsPinelineDesc.pShaderCount = 2;
		graphicsPinelineDesc.pShaders[0] = pVertShader;
		graphicsPinelineDesc.pShaders[1] = pFragShader;

		PipelineDesc pipelineDesc = {};
		pipelineDesc.mGraphicsDesc = graphicsPinelineDesc;
		pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
		addPipeline(pRenderer, &pipelineDesc, &pPipeline);

		pipelineLayout = pPipeline->mVkPipelineLayout;
		pRenderPass = &pPipeline->mRenderPass;
		graphicsPipeline = pPipeline->pVkPipeline;

	}

	bool Load() override
	{
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();

		//初始化UI 接口
		UserInterfaceDesc uiRenderDesc = {};
		uiRenderDesc.pGraphicsQueue = pGraphicsQueue;
		uiRenderDesc.pSwapChain = pSwapChain;
		uiRenderDesc.pRenderer = pRenderer;
		initUserInterface(&uiRenderDesc);
		createImGuiCommandBuffers(pTextures);
		return true;
	}

	void createFramebuffers() {
		FrameBufferDesc frameBufferDesc = {};
		frameBufferDesc.mHeight = pSwapChain->pDesc->mHeight;
		frameBufferDesc.mWidth = pSwapChain->pDesc->mWidth;
		frameBufferDesc.pRenderPass = pRenderPass;
		pFrameBuffers.resize(pTextures.size());
		FrameBuffer* pFrameBuffer = NULL;
		for (size_t i = 0; i < pTextures.size(); i++)
		{
			frameBufferDesc.pTexture = &pTextures[i];
			addFrameBuffer(pRenderer, &frameBufferDesc, &pFrameBuffers[i]);
		}
	}

	void createCommandPool()
	{
		CmdPoolDesc cmdPoolDesc = {};
		cmdPoolDesc.pQueue = pGraphicsQueue;
		addCmdPool(pRenderer, &cmdPoolDesc, &pCmdPool);
	}

	void createCommandBuffers() {
		CmdDesc cmdDesc = {};
		cmdDesc.pPool = pCmdPool;

		addCmd(pRenderer, &cmdDesc, &pCmds[0]);
		addCmd(pRenderer, &cmdDesc, &pCmds[1]);
	}

	void createSyncObjects() {
		addSemaphore(pRenderer, &pImageAvailableSemaphores[0]);
		addSemaphore(pRenderer, &pImageAvailableSemaphores[1]);
		addSemaphore(pRenderer, &pRenderFinishedSemaphores[0]);
		addSemaphore(pRenderer, &pRenderFinishedSemaphores[1]);

		addFence(pRenderer, &pInFlightFences[0]);
		addFence(pRenderer, &pInFlightFences[1]);
	}

	void Draw()
	{
		//SHEN_CLIENT_INFO("Main loop");
		waitForFences(pRenderer, 1, &pInFlightFences[currentFrame]);
		uint32_t imageIndex;
		acquireNextImage(pRenderer, pSwapChain, pImageAvailableSemaphores[currentFrame], pInFlightFences[currentFrame], &imageIndex);
		//开始指令录制
		Cmd* cmd = pCmds[currentFrame];

		beginCmd(cmd);
		//绑定到渲染子通道
		cmdBindRenderPass(cmd, pRenderPass, pFrameBuffers[imageIndex]);
		//指令绑定到管线
		cmdBindPipeline(cmd, pPipeline);
		// 设置指令视口尺寸
		cmdSetViewport(cmd, 0.0f, 0.0f,
			(float)pSwapChain->pDesc->mWidth,
			(float)pSwapChain->pDesc->mHeight, 0.0f, 1.0f);
		//设置视口裁切
		cmdSetScissor(cmd, 0, 0, (float)pSwapChain->pDesc->mWidth,
			(float)pSwapChain->pDesc->mHeight);
		//指令绘制
		cmdDraw(cmd, 3, 0);

		//绘制UI
		cmdDrawUserInterface(cmd,imageIndex,currentFrame, pInFlightFences[currentFrame]->pVkFence);
		// 结束绘制
		endCmd(cmd);
		//图像队列提交

		QueueSubmitDesc submitDesc = {};
		submitDesc.mCmdCount = 1;
		submitDesc.mSignalSemaphoreCount = 1;
		submitDesc.mWaitSemaphoreCount = 1;
		submitDesc.ppCmds = &cmd;
		submitDesc.ppSignalSemaphores = &pRenderFinishedSemaphores[currentFrame];
		submitDesc.ppWaitSemaphores = &pImageAvailableSemaphores[currentFrame];
		submitDesc.pSignalFence = pInFlightFences[currentFrame];
		queueSubmit(pGraphicsQueue, &submitDesc);

		QueuePresentDesc presentDesc = {};
		presentDesc.mIndex = imageIndex;
		presentDesc.mWaitSemaphoreCount = 1;
		presentDesc.pSwapChain = pSwapChain;
		presentDesc.ppWaitSemaphores = &pRenderFinishedSemaphores[currentFrame];
		presentDesc.mSubmitDone = true;
		queuePresent(pGraphicsQueue, &presentDesc);
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	const char* GetName() { return "TheShen"; }

private:

};


DEFINE_APPLICATION_MAIN(Sandbox)