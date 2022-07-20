#include "example/imgui_impl_glfw.h"
#include "example/imgui_impl_vulkan.h"
#include "UI.h"
#include "Renderer/Renderer.h"
#include "Core/Log.h"
#include "Core/Application.h"

typedef struct UserInterface
{
	Renderer* pRenderer = NULL;
	Queue* pGraphicsQueue = NULL;
	SwapChain* pSwapChain = NULL;
} UserInterface;

static UserInterface* pUserInterface = NULL;

VkDescriptorPool m_ImGuiDescriptorPool;
VkCommandPool m_ImGuiCommandPool;
VkRenderPass m_ImGuiRenderPass;


void createImGuiDescriptorPool()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
			{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
			{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000} };

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	if (vkCreateDescriptorPool(pUserInterface->pRenderer->pVkDevice, &pool_info, nullptr, &m_ImGuiDescriptorPool) != VK_SUCCESS)
	{
		SHEN_CORE_ERROR("Create DescriptorPool for m_ImGuiDescriptorPool failed!");
		throw std::runtime_error("Create DescriptorPool for m_ImGuiDescriptorPool failed!");
	}
}

void createCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = pUserInterface->pRenderer->pVkGraphicsQueueFamilyIndex;

	if (vkCreateCommandPool(pUserInterface->pRenderer->pVkDevice, &poolInfo, nullptr, &m_ImGuiCommandPool) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create graphics command pool!");
		throw std::runtime_error("failed to create graphics command pool!");
	}
}

void createImGuiRenderPass()
{
	VkAttachmentDescription attachment = {};
	attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0; // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;

	if (vkCreateRenderPass(pUserInterface->pRenderer->pVkDevice, &info, nullptr, &m_ImGuiRenderPass) != VK_SUCCESS)
	{
		SHEN_CORE_ERROR("failed to create render pass!");
		throw std::runtime_error("failed to create render pass!");
	}

}

VkCommandBuffer beginSingleTimeCommands(const VkCommandPool& cmdPool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = cmdPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(pUserInterface->pRenderer->pVkDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer, const VkCommandPool& cmdPool) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(pUserInterface->pGraphicsQueue->pVkQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(pUserInterface->pGraphicsQueue->pVkQueue);

	vkFreeCommandBuffers(pUserInterface->pRenderer->pVkDevice, cmdPool, 1, &commandBuffer);
}


/// <summary>
///	初始化用户接口
/// </summary>
/// <param name="pDesc"></param>
void initUserInterface(UserInterfaceDesc* pDesc)
{
	pUserInterface->pRenderer = (Renderer*)pDesc->pRenderer;
	pUserInterface->pGraphicsQueue = (Queue*)pDesc->pGraphicsQueue;
	pUserInterface->pSwapChain = (SwapChain*)pDesc->pSwapChain;
	createImGuiDescriptorPool();

	createCommandPool();
	createImGuiRenderPass();

	ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)(Application::Get().GetNativeWindow()), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = pUserInterface->pRenderer->pVkInstance;
	init_info.PhysicalDevice = pUserInterface->pRenderer->pVkActiveGPU;
	init_info.Device = pUserInterface->pRenderer->pVkDevice;
	init_info.QueueFamily = pUserInterface->pRenderer->pVkGraphicsQueueFamilyIndex;
	init_info.Queue = pUserInterface->pGraphicsQueue->pVkQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = m_ImGuiDescriptorPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 2;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info, m_ImGuiRenderPass);

	// Upload Fonts
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_ImGuiCommandPool);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		endSingleTimeCommands(commandBuffer, m_ImGuiCommandPool);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

bool platformInitUserInterface()
{
	UserInterface* pAppUI = (UserInterface*)malloc(sizeof(UserInterface));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	pUserInterface = pAppUI;
	return true;
}