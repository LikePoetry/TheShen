#include <TheShen.h>
#include "Core/App.h"
#include "TestLayer.h"
#include "Renderer/Renderer.h"

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

std::vector<VkFramebuffer> swapChainFramebuffers;

CmdPool* pCmdPool = NULL;

Cmd* pCmds[MAX_FRAMES_IN_FLIGHT] = { NULL };

Semaphore* pImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT] = { NULL };
Semaphore* pRenderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT] = { NULL };

Fence* pInFlightFences[MAX_FRAMES_IN_FLIGHT] = { NULL };

VkCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
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
		Shader* pVertShader;
		addShader(pRenderer, &shaderDesc, &pVertShader);

		shaderDesc.mStages = SHADER_STAGE_FRAG;
		shaderDesc.pFileName = "F:/VulkanWorkarea/TheShenProject_github/TheShen/bin/Debug-windows-x86_64/Sandbox/shaders/frag.spv";
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
		return true;
	}

	void createFramebuffers() {
		FrameBufferDesc frameBufferDesc = {};
		frameBufferDesc.mHeight = pSwapChain->pDesc->mHeight;
		frameBufferDesc.mWidth = pSwapChain->pDesc->mWidth;
		frameBufferDesc.pRenderPass = pRenderPass;
		pFrameBuffers.resize(pTextures.size());
		swapChainFramebuffers.resize(pTextures.size());
		FrameBuffer* pFrameBuffer = NULL;
		for (size_t i = 0; i < pTextures.size(); i++)
		{
			frameBufferDesc.pTexture = &pTextures[i];
			addFrameBuffer(pRenderer, &frameBufferDesc, &pFrameBuffers[i]);
			swapChainFramebuffers[i] = pFrameBuffers[i]->pFramebuffer;
		}
	}

	void createCommandPool()
	{
		CmdPoolDesc cmdPoolDesc = {};
		cmdPoolDesc.pQueue = pGraphicsQueue;
		addCmdPool(pRenderer, &cmdPoolDesc, &pCmdPool);
		commandPool = pCmdPool->pVkCmdPool;
	}

	void createCommandBuffers() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		CmdDesc cmdDesc = {};
		cmdDesc.pPool = pCmdPool;

		addCmd(pRenderer, &cmdDesc, &pCmds[0]);
		addCmd(pRenderer, &cmdDesc, &pCmds[1]);
		commandBuffers[0] = pCmds[0]->pVkCmdBuf;
		commandBuffers[1] = pCmds[1]->pVkCmdBuf;
	}

	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		addSemaphore(pRenderer, &pImageAvailableSemaphores[0]);
		addSemaphore(pRenderer, &pImageAvailableSemaphores[1]);
		addSemaphore(pRenderer, &pRenderFinishedSemaphores[0]);
		addSemaphore(pRenderer, &pRenderFinishedSemaphores[1]);
		imageAvailableSemaphores[0] = pImageAvailableSemaphores[0]->pVkSemaphore;
		imageAvailableSemaphores[1] = pImageAvailableSemaphores[1]->pVkSemaphore;
		renderFinishedSemaphores[0] = pRenderFinishedSemaphores[0]->pVkSemaphore;
		renderFinishedSemaphores[1] = pRenderFinishedSemaphores[1]->pVkSemaphore;

		addFence(pRenderer, &pInFlightFences[0]);
		addFence(pRenderer, &pInFlightFences[1]);

		inFlightFences[0] = pInFlightFences[0]->pVkFence;
		inFlightFences[1] = pInFlightFences[1]->pVkFence;
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pRenderPass->pRenderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = pSwapChain->pDesc->mExtend2D;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)pSwapChain->pDesc->mWidth;
		viewport.height = (float)pSwapChain->pDesc->mHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = pSwapChain->pDesc->mExtend2D;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void Draw()
	{
		waitForFences(pRenderer, 1, &pInFlightFences[currentFrame]);
		uint32_t imageIndex;
		acquireNextImage(pRenderer, pSwapChain, pImageAvailableSemaphores[currentFrame], pInFlightFences[currentFrame], &imageIndex);

		vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(pGraphicsQueue->pVkQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { pSwapChain->pSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		VkResult result = vkQueuePresentKHR(pSwapChain->pPresentQueue, &presentInfo);


		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	const char* GetName() { return "TheShen"; }

private:

};


DEFINE_APPLICATION_MAIN(Sandbox)