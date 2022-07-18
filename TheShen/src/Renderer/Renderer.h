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
/// ��Ⱦ��ʼ����������
/// </summary>
typedef struct RendererDesc
{

}RendererDesc;

/// <summary>
/// ��Ⱦ��ʼ������
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
/// ��������
/// </summary>
typedef struct QueueDesc
{
	QueueType							mType;
	QueueFlag							mFlag;
} QueueDesc;

/// <summary>
/// ����
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
/// ����������������ʱ��Flag ��Ϣ;
/// </summary>
typedef enum SwapChainCreationFlags
{
	SWAP_CHAIN_CREATION_FLAG_NONE = 0x0,
	SWAP_CHAIN_CREATION_FLAG_ENABLE_FOVEATED_RENDERING_VR = 0x1,
} SwapChainCreationFlags;

/// <summary>
/// ����������
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
/// ������
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
/// ��Ⱦͨ��������
/// </summary>
typedef struct RenderPassDesc
{
	VkFormat			pColorFormats;
	uint32_t            mRenderTargetCount;
} RenderPassDesc;

/// <summary>
/// ��Ⱦͨ��
/// </summary>
typedef struct RenderPass
{
	VkRenderPass   pRenderPass;
	RenderPassDesc mDesc;
} RenderPass;

/// <summary>
/// ��������
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
	VkShaderModule* pShaderModules;
	ShaderStage		mStages : 31;
}Shader;

/// <summary>
/// ͼ�ι���˵��
/// </summary>
typedef struct GraphicsPipelineDesc
{
	Shader pShaders[SHADER_STAGE_COUNT];
} GraphicsPipelineDesc;

/// <summary>
/// ��������
/// </summary>
typedef struct PipelineDesc
{
	GraphicsPipelineDesc   mGraphicsDesc;
	PipelineType   mType;
};


typedef struct Pipeline
{
	VkPipeline   pVkPipeline;
	PipelineType mType;
} Pipeline;
/// <summary>
/// ��ʼ����ͼ�豸,�����������Ĵ���
/// </summary>
/// <param name="appName"></param>
/// <param name="pSettings"></param>
/// <param name="ppRenderer"></param>
void initRenderer(const char* appName, const RendererDesc* pSettings, Renderer** ppRenderer, SwapChainDesc* p_desc, SwapChain** p_swap_chain, std::vector<Texture>& pTextures);
// ��Ӷ���
void addQueue(Renderer* pRenderer, QueueDesc* pQDesc, Queue** pQueue);
////�����Ⱦͨ��
void addRenderPass(Renderer* pRenderer, const RenderPassDesc* pDesc, RenderPass** ppRenderPass);
//�����Ⱦ����
void addPipeline(Renderer* pRenderer, const PipelineDesc* pDesc, Pipeline** ppPipeline);
//�����ɫ��
void addShader(Renderer* pRenderer, const ShaderDesc* pDesc, Shader** ppShader);