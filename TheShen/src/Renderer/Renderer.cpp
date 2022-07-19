#include "Renderer.h"
#include "Core/Log.h"




const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkDebugUtilsMessengerEXT debugMessenger;

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

/// <summary>
/// 检查验证层支持
/// </summary>
/// <returns></returns>
bool checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

/// <summary>
/// 获取扩展信息
/// </summary>
/// <returns></returns>
std::vector<const char*> getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

/// <summary>
/// 验证层消息回调
/// </summary>
/// <param name="messageSeverity"></param>
/// <param name="messageType"></param>
/// <param name="pCallbackData"></param>
/// <param name="pUserData"></param>
/// <returns></returns>
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

/// <summary>
/// Debug 消息回调
/// </summary>
/// <param name="createInfo"></param>
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

/// <summary>
/// 检查可用队列家族和它们的属性
/// </summary>
/// <param name="physical_device"></param>
/// <param name="queue_families"></param>
/// <returns></returns>
bool CheckAvailableQueueFamiliesAndTheirProperties(VkPhysicalDevice                       physical_device,
	std::vector<VkQueueFamilyProperties>& queue_families) {
	//第一个阶段，获取指定物理设备上可用队列家族的总数。通过将最后一个参数设置为nullptr。调用vkGetPhysicalDeviceQueueFamilyProperties()
	uint32_t queue_families_count = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, nullptr);
	if (queue_families_count == 0) {
		SHEN_CORE_ERROR("Could not get the number of queue families.");
		return false;
	}
	//第二阶段，在获取指定物理设备含有的所有队列家族的数量后，就可以为队列家族的属性准备存储空间。
	//再次调用vkGetPhysicalDeivceQueueFamilyProperties()函数，获取所有可用队列家族的属性
	queue_families.resize(queue_families_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_families.data());
	if (queue_families_count == 0) {
		SHEN_CORE_ERROR("Could not acquire properties of queue families.");
		return false;
	}

	return true;
}

/// <summary>
///  根据功能选择队列家族的索引
/// </summary>
/// <param name="physical_device"></param>
/// <param name="desired_capabilities"></param>
/// <param name="queue_family_index"></param>
/// <returns></returns>
bool SelectIndexOfQueueFamilyWithDesiredCapabilities(VkPhysicalDevice   physical_device,
	VkQueueFlags       desired_capabilities,
	uint32_t& queue_family_index) {
	//先获取指定物理设备上可用队列家族的属性，将通过执行检查操作获得的数据存储在queue_families变量中，
	//queue_families变量中存储的是元素类型为VkQueueFamilyProperties的std::vector 容器
	std::vector<VkQueueFamilyProperties> queue_families;
	if (!CheckAvailableQueueFamiliesAndTheirProperties(physical_device, queue_families)) {
		return false;
	}

	//然后检查vector容器queue_families 变量中的所有元素
	//queue_families 变量中的每个元素，都代表一个独立的队列家族。每个元素中的queueCount成员都含有该元素代表的队列家族中可用队列的数量。
	//queueFlages成员使用位域数据结构，在该数据结构中每个二进制位代表一种操作类型，对指定的位进行设置，则代表其对应类型的操作由指定的队列家族来支持。
	//我们可以尝试组合使用队列家族支持的任何操作，但是需要为每种类型的操作寻找独立的队列，这完全取决于硬件支持和Vulkan驱动程序。
	//为了确保我们获得了正确的数据，还应该检查每个家族是否至少含有一个队列。
	//在更高级的现实处理情况下，需要存储每个家族含有队列的总数，这是因为我们可以使用一个以上的队列，但无法使用超过指定家族中可用队列数量的队列。
	//在简单的情况下，使用指定家族中的一个队列就足够了。
	for (uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size()); ++index) {
		if ((queue_families[index].queueCount > 0) &&
			((queue_families[index].queueFlags & desired_capabilities) == desired_capabilities)) {
			queue_family_index = index;
			return true;
		}
	}
	return false;
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> transferFamily;


	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() && transferFamily.has_value();
	}
};

/// <summary>
/// 交换链支持的细节信息
/// </summary>
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			indices.computeFamily = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			indices.transferFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

/// <summary>
/// 查询交换链支持的细节信息
/// </summary>
/// <param name="device">物理设备</param>
/// <param name="surface">显示表面</param>
/// <returns></returns>
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool isDeviceSuitable(Renderer pRenderer, VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices = findQueueFamilies(device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	pRenderer.pVkGraphicsQueueFamilyIndex = indices.graphicsFamily.value();
	pRenderer.pVkComputeQueueFamilyIndex = indices.computeFamily.value();
	pRenderer.pVkTransferQueueFamilyIndex = indices.transferFamily.value();
	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

/// <summary>
/// 初始化渲染配置信息
/// </summary>
/// <param name="appName"></param>
/// <param name="pSettings"></param>
/// <param name="ppRenderer"></param>
void initRenderer(const char* appName, const RendererDesc* pSettings, Renderer** ppRenderer, SwapChainDesc* pDesc, SwapChain** ppSwapChain, std::vector<Texture>& pTextures)
{
	//初始化ppRenderer
	Renderer* pRenderer = (Renderer*)malloc(sizeof(Renderer));
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		SHEN_CORE_ERROR("validation layers requested, but not available!");
		throw std::runtime_error("validation layers requested, but not available!");
	}
	//应用信息
	{

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//创建信息
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		//获取扩展信息
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &pRenderer->pVkInstance) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to create instance!");
			throw std::runtime_error("failed to create instance!");
		}
	}

	//验证层信息回调
	if (enableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(pRenderer->pVkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to set up debug messenger!");
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//创建surface
	SwapChain* pSwapChain = (SwapChain*)malloc(sizeof(SwapChain));
	{
		if (glfwCreateWindowSurface(pRenderer->pVkInstance, (GLFWwindow*)pDesc->mWindow, nullptr, &pSwapChain->pVkSurface) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to create window surface!");
			throw std::runtime_error("failed to create window surface!");
		}
	}

	//选取物理设备
	{
		//获取设备的数量
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(pRenderer->pVkInstance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			SHEN_CORE_ERROR("failed to find GPUs with Vulkan support!");
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		//枚举所有的设备
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(pRenderer->pVkInstance, &deviceCount, devices.data());

		//选取支持 图像，计算和演示功能的显卡
		for (const auto& device : devices)
		{
			if (isDeviceSuitable(*pRenderer, device, pSwapChain->pVkSurface)) {
				pRenderer->pVkActiveGPU = device;
				break;
			}
		}
	}

	//创建逻辑设备
	{
		QueueFamilyIndices indices = findQueueFamilies(pRenderer->pVkActiveGPU, pSwapChain->pVkSurface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		//图像队列索引
		pRenderer->pVkGraphicsQueueFamilyIndex = indices.graphicsFamily.value();

		//演示队列索引
		pSwapChain->mPresentQueueFamilyIndex = indices.presentFamily.value();

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(pRenderer->pVkActiveGPU, &createInfo, nullptr, &pRenderer->pVkDevice) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to create logical device!");
			throw std::runtime_error("failed to create logical device!");
		}
	}

	//创建交换链
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pRenderer->pVkActiveGPU, pSwapChain->pVkSurface);
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent((GLFWwindow*)pDesc->mWindow, swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = pSwapChain->pVkSurface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (swapChainSupport.capabilities.supportedTransforms & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		{
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		// Enable transfer destination on swap chain images if supported
		if (swapChainSupport.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		{
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		QueueFamilyIndices indices = findQueueFamilies(pRenderer->pVkActiveGPU, pSwapChain->pVkSurface);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(pRenderer->pVkDevice, &createInfo, nullptr, &pSwapChain->pSwapChain) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to create swap chain!");
			throw std::runtime_error("failed to create swap chain!");
		}

		std::vector<VkImage> swapChainImages;
		vkGetSwapchainImagesKHR(pRenderer->pVkDevice, pSwapChain->pSwapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(pRenderer->pVkDevice, pSwapChain->pSwapChain, &imageCount, swapChainImages.data());
		//交换链其他信息存储
		pTextures.resize(0);
		for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = swapChainImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = surfaceFormat.format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			VkImageView imageView;
			if (vkCreateImageView(pRenderer->pVkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
				SHEN_CORE_ERROR("failed to create texture image view!");
				throw std::runtime_error("failed to create texture image view!");
			}
			Texture pTexture;
			memset(&pTexture, 0, sizeof(pTexture));
			pTexture.pVkImage = swapChainImages[i];
			pTexture.pVkSRVDescriptor = imageView;

			pTextures.push_back(pTexture);
		}

		pDesc->mImageFormat = surfaceFormat.format;
		pDesc->mExtend2D = extent;
		pDesc->mImageCount = imageCount;
		pSwapChain->pDesc = pDesc;
		//创建演示队列
		vkGetDeviceQueue(pRenderer->pVkDevice, pSwapChain->mPresentQueueFamilyIndex, 0, &pSwapChain->pPresentQueue);
	}
	*ppRenderer = pRenderer;
	*ppSwapChain = pSwapChain;
}

void uitil_find_queue_family_index(const Renderer* pRenderer, QueueType queueType, uint32_t* pOutFamilyIndex)
{
	//图形队列
	if (queueType == QUEUE_TYPE_GRAPHICS)
	{
		*pOutFamilyIndex = pRenderer->pVkGraphicsQueueFamilyIndex;
	}
}

/// <summary>
/// 添加队列族
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="pQDesc"></param>
/// <param name="pQueue"></param>
void addQueue(Renderer* pRenderer, QueueDesc* pDesc, Queue** ppQueue)
{
	Queue* pQueue = (Queue*)malloc(sizeof(Queue));
	uint32_t queueFamilyIndex = UINT32_MAX;
	uitil_find_queue_family_index(pRenderer, pDesc->mType, &queueFamilyIndex);
	pQueue->mVkQueueIndex = queueFamilyIndex;
	vkGetDeviceQueue(pRenderer->pVkDevice, queueFamilyIndex, 0, &pQueue->pVkQueue);
	*ppQueue = pQueue;
}

/// <summary>
/// 添加渲染通道
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="pDesc"></param>
/// <param name="ppRenderPass"></param>
void addRenderPass(Renderer* pRenderer, const RenderPassDesc* pDesc, RenderPass** ppRenderPass)
{
	RenderPass* pRenderPass = (RenderPass*)malloc(sizeof(RenderPass));
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = pDesc->pColorFormats;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkRenderPass renderPass = {};
	if (vkCreateRenderPass(pRenderer->pVkDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create render pass!");
		throw std::runtime_error("failed to create render pass!");
	}

	pRenderPass->pRenderPass = renderPass;
	*ppRenderPass = pRenderPass;
}

void addGraphicsPipeline(Renderer* pRenderer, const PipelineDesc* pDesc, Pipeline** ppPipeline)
{
	Pipeline* pPipeline = (Pipeline*)malloc(sizeof(Pipeline));
	pPipeline->mType = pDesc->mType;
	int32_t shaderCount = pDesc->mGraphicsDesc.pShaderCount;
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = pDesc->mGraphicsDesc.pShaders[0]->pShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = pDesc->mGraphicsDesc.pShaders[1]->pShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(pRenderer->pVkDevice, &pipelineLayoutInfo, nullptr, &pPipeline->mVkPipelineLayout) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create pipeline layout!");
		throw std::runtime_error("failed to create pipeline layout!");
	}



	RenderPassDesc renderPassDesc = {};
	renderPassDesc.pColorFormats = pDesc->mGraphicsDesc.pColorFormats;
	RenderPass* pRenderPass;
	addRenderPass(pRenderer, &renderPassDesc, &pRenderPass);
	pPipeline->mRenderPass = *pRenderPass;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pPipeline->mVkPipelineLayout;
	pipelineInfo.renderPass = pRenderPass->pRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(pRenderer->pVkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pPipeline->pVkPipeline) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create graphics pipeline!");
		throw std::runtime_error("failed to create graphics pipeline!");
	}


	vkDestroyShaderModule(pRenderer->pVkDevice, pDesc->mGraphicsDesc.pShaders[0]->pShaderModule, nullptr);
	vkDestroyShaderModule(pRenderer->pVkDevice, pDesc->mGraphicsDesc.pShaders[1]->pShaderModule, nullptr);

	*ppPipeline = pPipeline;
}

void addPipeline(Renderer* pRenderer, const PipelineDesc* pDesc, Pipeline** ppPipeline)
{
	switch (pDesc->mType)
	{
	case PIPELINE_TYPE_GRAPHICS:
	{
		addGraphicsPipeline(pRenderer, pDesc, ppPipeline);
		break;
	}
	default:
		break;
	}


}

/// <summary>
	/// 读取文件
	/// </summary>
	/// <param name="filename"></param>
	/// <returns></returns>
static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		SHEN_CORE_ERROR("failed to open file!");
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void addShader(Renderer* pRenderer, const ShaderDesc* pDesc, Shader** ppShader)
{
	int32_t size = sizeof(Shader);
	Shader* pShader = (Shader*)malloc(sizeof(Shader));
	auto shaderCode = readFile(pDesc->pFileName);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
	if (vkCreateShaderModule(pRenderer->pVkDevice, &createInfo, nullptr, &pShader->pShaderModule) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create shader module!");
		throw std::runtime_error("failed to create shader module!");
	}

	pShader->mStages = pDesc->mStages;
	*ppShader = pShader;
}

/// <summary>
/// 添加帧缓存
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="pDesc"></param>
/// <param name="ppFrameBuffer"></param>
void addFrameBuffer(Renderer* pRenderer, const FrameBufferDesc* pDesc, FrameBuffer** ppFrameBuffer)
{
	FrameBuffer* pFrameBuffer = (FrameBuffer*)malloc(sizeof(FrameBuffer));

	VkImageView attachments[] = {
		pDesc->pTexture->pVkSRVDescriptor
	};

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = pDesc->pRenderPass->pRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = pDesc->mWidth;
	framebufferInfo.height = pDesc->mHeight;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(pRenderer->pVkDevice, &framebufferInfo, nullptr, &pFrameBuffer->pFramebuffer) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create framebuffer!");
		throw std::runtime_error("failed to create framebuffer!");
	}
	pFrameBuffer->mHeight = pDesc->mHeight;
	pFrameBuffer->mWidth = pDesc->mWidth;

	*ppFrameBuffer = pFrameBuffer;
}

/// <summary>
/// 添加命令缓冲池
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="pDesc"></param>
/// <param name="ppCmdPool"></param>
void addCmdPool(Renderer* pRenderer, const CmdPoolDesc* pDesc, CmdPool** ppCmdPool)
{
	CmdPool* pCmdPool = (CmdPool*)malloc(sizeof(CmdPool));
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = pDesc->pQueue->mVkQueueIndex;
	if (vkCreateCommandPool(pRenderer->pVkDevice, &poolInfo, nullptr, &pCmdPool->pVkCmdPool) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create command pool!");
		throw std::runtime_error("failed to create command pool!");
	}
	pCmdPool->pQueue = pDesc->pQueue;

	*ppCmdPool = pCmdPool;
}

/// <summary>
/// 添加命令
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="pDesc"></param>
/// <param name="ppCmd"></param>
void addCmd(Renderer* pRenderer, const CmdDesc* pDesc, Cmd** ppCmd)
{
	Cmd* pCmd = (Cmd*)malloc(sizeof(Cmd));

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pDesc->pPool->pVkCmdPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(pRenderer->pVkDevice, &allocInfo, &(pCmd->pVkCmdBuf)) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to allocate command buffers!");
		throw std::runtime_error("failed to allocate command buffers!");
	}

	*ppCmd = pCmd;
}

/// <summary>
/// 添加信号量
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="ppSemaphore"></param>
void addSemaphore(Renderer* pRenderer, Semaphore** ppSemaphore)
{
	Semaphore* pSemaphore = (Semaphore*)malloc(sizeof(Semaphore));
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = NULL;
	semaphoreInfo.flags = 0;
	if (vkCreateSemaphore(pRenderer->pVkDevice, &semaphoreInfo, nullptr, &pSemaphore->pVkSemaphore) != VK_SUCCESS)
	{
		SHEN_CORE_ERROR("failed to create synchronization objects for a frame!");
		throw std::runtime_error("failed to create synchronization objects for a frame!");
	}
	pSemaphore->mSignaled = 0;
	*ppSemaphore = pSemaphore;
}

/// <summary>
/// 添加栅栏
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="ppFence"></param>
void addFence(Renderer* pRenderer, Fence** ppFence)
{
	Fence* pFence = (Fence*)malloc(sizeof(Fence));

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	fenceInfo.pNext = NULL;

	if (vkCreateFence(pRenderer->pVkDevice, &fenceInfo, nullptr, &pFence->pVkFence) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create synchronization objects for a frame!");
		throw std::runtime_error("failed to create synchronization objects for a frame!");
	}
	pFence->mSubmitted = 0;
	*ppFence = pFence;
}

