#include "Renderer.h"
#include "Core/Log.h"
#include <vector>
#include <stdexcept>
#include <iostream>

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

/// <summary>
/// 初始化渲染配置信息
/// </summary>
/// <param name="appName"></param>
/// <param name="pSettings"></param>
/// <param name="ppRenderer"></param>
void initRenderer(const char* appName, const RendererDesc* pSettings, Renderer** ppRenderer)
{
	//初始化ppRenderer
	Renderer pRenderer;
	memset(&pRenderer, 0, sizeof(pRenderer));
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		SHEN_CORE_ERROR("validation layers requested, but not available!");
		throw std::runtime_error("validation layers requested, but not available!");
	}
	{
		//应用信息
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

		if (vkCreateInstance(&createInfo, nullptr, &pRenderer.pVkInstance) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to create instance!");
			throw std::runtime_error("failed to create instance!");
		}
	}

	//验证层信息回调
	if (enableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(pRenderer.pVkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to set up debug messenger!");
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//选取设备
	{
		//获取设备的数量
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(pRenderer.pVkInstance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			SHEN_CORE_ERROR("failed to find GPUs with Vulkan support!");
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		//枚举所有的设备
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(pRenderer.pVkInstance, &deviceCount, devices.data());

		//选取支持 图像，计算和演示功能的显卡
		for (const auto& device : devices)
		{
			if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(device, VK_QUEUE_GRAPHICS_BIT, pRenderer.pVkGraphicsQueueFamilyIndex)) {
				continue;
			}
			if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(device, VK_QUEUE_COMPUTE_BIT, pRenderer.pVkComputeQueueFamilyIndex)) {
				continue;
			}
			if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(device, VK_QUEUE_TRANSFER_BIT, pRenderer.pVkTransferQueueFamilyIndex)) {
				continue;
			}
			pRenderer.pVkActiveGPU = device;
			break;
		}
	}

	//创建逻辑设备
	{
		//获取设备队列族信息
		uint32_t queueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(pRenderer.pVkActiveGPU, &queueFamiliesCount, NULL);
		std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(pRenderer.pVkActiveGPU, &queueFamiliesCount, queueFamiliesProperties.data());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { pRenderer.pVkGraphicsQueueFamilyIndex, pRenderer.pVkComputeQueueFamilyIndex,pRenderer.pVkTransferQueueFamilyIndex };
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

		if (vkCreateDevice(pRenderer.pVkActiveGPU, &createInfo, nullptr, &pRenderer.pVkDevice) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to create logical device!");
			throw std::runtime_error("failed to create logical device!");
		}
	}
	*ppRenderer = &pRenderer;
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
	Queue pQueue;
	memset(&pQueue, 0, sizeof(pQueue));

	uint32_t queueFamilyIndex = UINT32_MAX;
	uitil_find_queue_family_index(pRenderer, pDesc->mType, &queueFamilyIndex);

	vkGetDeviceQueue(pRenderer->pVkDevice, queueFamilyIndex, 0, &pQueue.pVkQueue);
	*ppQueue = &pQueue;
}

/// <summary>
/// 创建交换链
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="p_desc"></param>
/// <param name="p_swap_chain"></param>
void addSwapChain(Renderer* pRenderer, const SwapChainDesc* pDesc, SwapChain** ppSwapChain)
{
	//创建surface
	SwapChain pSwapChain;
	memset(&pSwapChain, 0, sizeof(pSwapChain));
	if (glfwCreateWindowSurface(pRenderer->pVkInstance, (GLFWwindow*)pDesc->mWindow, nullptr, &pSwapChain.pVkSurface) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create window surface!");
		throw std::runtime_error("failed to create window surface!");
	}

	//创建交换链

}