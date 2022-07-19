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
	VkDescriptorSetLayout				pEmptyDescriptorSetLayout;
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
	VkQueue	pVkQueue;
	uint32_t mVkQueueIndex : 5;
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

/// <summary>
/// 渲染通道描述符
/// </summary>
typedef struct RenderPassDesc
{
	VkFormat			pColorFormats;
	uint32_t            mRenderTargetCount;
} RenderPassDesc;

/// <summary>
/// 渲染通道
/// </summary>
typedef struct RenderPass
{
	VkRenderPass   pRenderPass;
	RenderPassDesc mDesc;
} RenderPass;

/// <summary>
/// 管线类型
/// </summary>
typedef enum PipelineType
{
	PIPELINE_TYPE_UNDEFINED = 0,
	PIPELINE_TYPE_COMPUTE,
	PIPELINE_TYPE_GRAPHICS,
	PIPELINE_TYPE_RAYTRACING,
	PIPELINE_TYPE_COUNT,
} PipelineType;

typedef enum ShaderStage
{
	SHADER_STAGE_NONE = 0,
	SHADER_STAGE_VERT = 0X00000001,
	SHADER_STAGE_TESC = 0X00000002,
	SHADER_STAGE_TESE = 0X00000004,
	SHADER_STAGE_GEOM = 0X00000008,
	SHADER_STAGE_FRAG = 0X00000010,
	SHADER_STAGE_COMP = 0X00000020,
	SHADER_STAGE_RAYTRACING = 0X00000040,
	SHADER_STAGE_ALL_GRAPHICS =
	((uint32_t)SHADER_STAGE_VERT | (uint32_t)SHADER_STAGE_TESC | (uint32_t)SHADER_STAGE_TESE | (uint32_t)SHADER_STAGE_GEOM |
		(uint32_t)SHADER_STAGE_FRAG),
	SHADER_STAGE_HULL = SHADER_STAGE_TESC,
	SHADER_STAGE_DOMN = SHADER_STAGE_TESE,
	SHADER_STAGE_COUNT = 7,
} ShaderStage;

typedef struct ShaderDesc
{
	const char* pFileName;
	ShaderStage	mStages : 31;
};

typedef struct Shader
{
	VkShaderModule pShaderModule;
	ShaderStage		mStages : 31;
}Shader;

/// <summary>
/// 图形管线说明
/// </summary>
typedef struct GraphicsPipelineDesc
{
	Shader* pShaders[SHADER_STAGE_COUNT];
	VkFormat pColorFormats;
	int32_t	pShaderCount;
} GraphicsPipelineDesc;

/// <summary>
/// 管线描述
/// </summary>
typedef struct PipelineDesc
{
	GraphicsPipelineDesc   mGraphicsDesc;
	PipelineType   mType;
};

/// <summary>
/// 管线
/// </summary>
typedef struct Pipeline
{
	VkPipeline   pVkPipeline;
	PipelineType mType;
	RenderPass mRenderPass;
	VkPipelineLayout mVkPipelineLayout;
} Pipeline;

/// <summary>
/// 帧缓存描述
/// </summary>
typedef struct FrameBufferDesc
{
	RenderPass* pRenderPass;
	Texture* pTexture;
	uint32_t        mWidth : 16;
	uint32_t        mHeight : 16;
} FrameBufferDesc;

/// <summary>
/// 帧缓存
/// </summary>
typedef struct FrameBuffer
{
	VkFramebuffer pFramebuffer;
	uint32_t      mWidth;
	uint32_t      mHeight;
} FrameBuffer;

/// <summary>
/// 命令池描述
/// </summary>
typedef struct CmdPoolDesc
{
	Queue* pQueue;
	bool mTransient;
} CmdPoolDesc;

/// <summary>
/// 命令池
/// </summary>
typedef struct CmdPool
{
	VkCommandPool pVkCmdPool;
	Queue* pQueue;
} CmdPool;

typedef struct CmdDesc
{
	CmdPool* pPool;
	bool mSecondary;
} CmdDesc;

/// <summary>
/// 命令
/// </summary>
typedef struct Cmd
{
	VkCommandBuffer  pVkCmdBuf;
	VkRenderPass     pVkActiveRenderPass;
	VkPipelineLayout pBoundPipelineLayout;
	CmdPool* pCmdPool;

	Renderer* pRenderer;
	Queue* pQueue;
} Cmd;

/// <summary>
/// 信号量
/// </summary>
typedef struct Semaphore
{
	VkSemaphore pVkSemaphore;
	uint32_t    mSignaled : 1;
} Semaphore;


typedef struct Fence
{
	VkFence  pVkFence;
	uint32_t mSubmitted : 1;
};

// 初始化绘图设备,包含交换链的创建
void initRenderer(const char* appName, const RendererDesc* pSettings, Renderer** ppRenderer, SwapChainDesc* p_desc, SwapChain** p_swap_chain, std::vector<Texture>& pTextures);
// 添加队列
void addQueue(Renderer* pRenderer, QueueDesc* pQDesc, Queue** pQueue);
// 添加渲染通道
void addRenderPass(Renderer* pRenderer, const RenderPassDesc* pDesc, RenderPass** ppRenderPass);
// 添加渲染管线
void addPipeline(Renderer* pRenderer, const PipelineDesc* pDesc, Pipeline** ppPipeline);
// 添加着色器
void addShader(Renderer* pRenderer, const ShaderDesc* pDesc, Shader** ppShader);
// 添加帧缓存
void addFrameBuffer(Renderer* pRenderer, const FrameBufferDesc* pDesc, FrameBuffer** ppFrameBuffer);
// 添加命令池
void addCmdPool(Renderer* pRenderer, const CmdPoolDesc* pDesc, CmdPool** ppCmdPool);
// 添加命令
void addCmd(Renderer* pRenderer, const CmdDesc* pDesc, Cmd** ppCmd);
// 添加信号量
void addSemaphore(Renderer* pRenderer, Semaphore** ppSemaphore);
// 增加栅栏
void addFence(Renderer* pRenderer, Fence** ppFence);


/*********  绘制图形部分函数 ***********/
/***************************************/
// 等待栅栏
void waitForFences(Renderer* pRenderer, int32_t fenceCount, Fence** ppFences);
// 获取下一帧图片
void acquireNextImage(Renderer* pRenderer, SwapChain* pSwapChain, Semaphore* pSignalSemaphore, Fence* pFence, uint32_t* pImageIndex);
// 开启指令录制
void beginCmd(Cmd* pCmd);
// 指令绑定到管线
void cmdBindPipeline(Cmd* pCmd, Pipeline* pPipeline);
// 指令视口设置
void cmdSetViewport(Cmd* pCmd, float x, float y, float width, float height, float minDepth, float maxDepth);
//设置指令视口裁切
void cmdSetScissor(Cmd* pCmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
// 指令绘制
void cmdDraw(Cmd* pCmd, uint32_t vertex_count, uint32_t first_vertex);