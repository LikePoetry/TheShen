#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>


/// <summary>
/// 渲染初始化参数描述
/// </summary>
typedef struct RendererDesc
{

}RendererDesc;

/// <summary>
/// 渲染初始化内容
/// </summary>
typedef struct Renderer
{
	VkInstance							pVkInstance;
	VkPhysicalDevice					pVkActiveGPU;
	VkDevice							pVkDevice;
	uint32_t							pVkGraphicsQueueFamilyIndex;
	uint32_t							pVkComputeQueueFamilyIndex;
	uint32_t							pVkTransferQueueFamilyIndex;
	//uint32_t							pVkPresentQueueFamilyIndex;

} Renderer;

typedef enum QueueType
{
	QUEUE_TYPE_GRAPHICS = 0,
	QUEUE_TYPE_TRANSFER,
	QUEUE_TYPE_COMPUTE,
	MAX_QUEUE_TYPE
};

typedef enum QueueFlag
{
	QUEUE_FLAG_NONE = 0x0,
	QUEUE_FLAG_DISABLE_GPU_TIMEOUT = 0x1,
	QUEUE_FLAG_INIT_MICROPROFILE = 0x2,
	MAX_QUEUE_FLAG = 0xFFFFFFFF
} QueueFlag;

/// <summary>
/// 队列描述
/// </summary>
typedef struct QueueDesc
{
	QueueType							mType;
	QueueFlag							mFlag;
} QueueDesc;

/// <summary>
/// 队列
/// </summary>
typedef struct Queue
{
	VkQueue								pVkQueue;
} Queue;

typedef struct Texture
{
	VkImageView pVkSRVDescriptor;
	VkImage pVkImage;
}Texture;

/// <summary>
/// 创建交换链描述符时的Flag 信息;
/// </summary>
typedef enum SwapChainCreationFlags
{
	SWAP_CHAIN_CREATION_FLAG_NONE = 0x0,
	SWAP_CHAIN_CREATION_FLAG_ENABLE_FOVEATED_RENDERING_VR = 0x1,
} SwapChainCreationFlags;

/// <summary>
/// 交互链描述
/// </summary>
typedef struct SwapChainDesc
{
	/// Window handle
	void* mWindow;
	/// Number of backbuffers in this swapchain
	uint32_t mImageCount;
	/// Width of the swapchain
	uint32_t mWidth;
	/// Height of the swapchain
	uint32_t mHeight;
	VkFormat mImageFormat;
	VkExtent2D mExtend2D;
	/// Swapchain creation flags
	SwapChainCreationFlags mFlags;
	/// Set whether swap chain will be presented using vsync
	bool mEnableVsync;

} SwapChainDesc;

/// <summary>
/// 交换链
/// </summary>
typedef struct SwapChain
{
	VkSwapchainKHR pSwapChain;
	VkSurfaceKHR   pVkSurface;
	VkQueue			pPresentQueue;
	uint32_t       mPresentQueueFamilyIndex : 5;
	SwapChainDesc* pDesc;
} SwapChain;

typedef struct RenderPassDesc
{
	VkFormat			pColorFormats;
	uint32_t            mRenderTargetCount;
} RenderPassDesc;

typedef struct RenderPass
{
	VkRenderPass   pRenderPass;
	RenderPassDesc mDesc;
} RenderPass;

/// <summary>
/// 初始化绘图设备,包含交换链的创建
/// </summary>
/// <param name="appName"></param>
/// <param name="pSettings"></param>
/// <param name="ppRenderer"></param>
void initRenderer(const char* appName, const RendererDesc* pSettings, Renderer** ppRenderer, SwapChainDesc* p_desc, SwapChain** p_swap_chain, std::vector<Texture> pTextures);
// 添加队列
void addQueue(Renderer* pRenderer, QueueDesc* pQDesc, Queue** pQueue);
//添加渲染通道
void addRenderPass(Renderer* pRenderer, const RenderPassDesc* pDesc, RenderPass** ppRenderPass);