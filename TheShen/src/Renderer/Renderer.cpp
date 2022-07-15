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
/// �����֤��֧��
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
/// ��ȡ��չ��Ϣ
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
/// ��֤����Ϣ�ص�
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
/// Debug ��Ϣ�ص�
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
/// �����ö��м�������ǵ�����
/// </summary>
/// <param name="physical_device"></param>
/// <param name="queue_families"></param>
/// <returns></returns>
bool CheckAvailableQueueFamiliesAndTheirProperties(VkPhysicalDevice                       physical_device,
	std::vector<VkQueueFamilyProperties>& queue_families) {
	//��һ���׶Σ���ȡָ�������豸�Ͽ��ö��м����������ͨ�������һ����������Ϊnullptr������vkGetPhysicalDeviceQueueFamilyProperties()
	uint32_t queue_families_count = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, nullptr);
	if (queue_families_count == 0) {
		SHEN_CORE_ERROR("Could not get the number of queue families.");
		return false;
	}
	//�ڶ��׶Σ��ڻ�ȡָ�������豸���е����ж��м���������󣬾Ϳ���Ϊ���м��������׼���洢�ռ䡣
	//�ٴε���vkGetPhysicalDeivceQueueFamilyProperties()��������ȡ���п��ö��м��������
	queue_families.resize(queue_families_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_families.data());
	if (queue_families_count == 0) {
		SHEN_CORE_ERROR("Could not acquire properties of queue families.");
		return false;
	}

	return true;
}

/// <summary>
///  ���ݹ���ѡ����м��������
/// </summary>
/// <param name="physical_device"></param>
/// <param name="desired_capabilities"></param>
/// <param name="queue_family_index"></param>
/// <returns></returns>
bool SelectIndexOfQueueFamilyWithDesiredCapabilities(VkPhysicalDevice   physical_device,
	VkQueueFlags       desired_capabilities,
	uint32_t& queue_family_index) {
	//�Ȼ�ȡָ�������豸�Ͽ��ö��м�������ԣ���ͨ��ִ�м�������õ����ݴ洢��queue_families�����У�
	//queue_families�����д洢����Ԫ������ΪVkQueueFamilyProperties��std::vector ����
	std::vector<VkQueueFamilyProperties> queue_families;
	if (!CheckAvailableQueueFamiliesAndTheirProperties(physical_device, queue_families)) {
		return false;
	}

	//Ȼ����vector����queue_families �����е�����Ԫ��
	//queue_families �����е�ÿ��Ԫ�أ�������һ�������Ķ��м��塣ÿ��Ԫ���е�queueCount��Ա�����и�Ԫ�ش����Ķ��м����п��ö��е�������
	//queueFlages��Աʹ��λ�����ݽṹ���ڸ����ݽṹ��ÿ��������λ����һ�ֲ������ͣ���ָ����λ�������ã���������Ӧ���͵Ĳ�����ָ���Ķ��м�����֧�֡�
	//���ǿ��Գ������ʹ�ö��м���֧�ֵ��κβ�����������ҪΪÿ�����͵Ĳ���Ѱ�Ҷ����Ķ��У�����ȫȡ����Ӳ��֧�ֺ�Vulkan��������
	//Ϊ��ȷ�����ǻ������ȷ�����ݣ���Ӧ�ü��ÿ�������Ƿ����ٺ���һ�����С�
	//�ڸ��߼�����ʵ��������£���Ҫ�洢ÿ�����庬�ж��е�������������Ϊ���ǿ���ʹ��һ�����ϵĶ��У����޷�ʹ�ó���ָ�������п��ö��������Ķ��С�
	//�ڼ򵥵�����£�ʹ��ָ�������е�һ�����о��㹻�ˡ�
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
/// ��ʼ����Ⱦ������Ϣ
/// </summary>
/// <param name="appName"></param>
/// <param name="pSettings"></param>
/// <param name="ppRenderer"></param>
void initRenderer(const char* appName, const RendererDesc* pSettings, Renderer** ppRenderer)
{
	//��ʼ��ppRenderer
	Renderer pRenderer;
	memset(&pRenderer, 0, sizeof(pRenderer));
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		SHEN_CORE_ERROR("validation layers requested, but not available!");
		throw std::runtime_error("validation layers requested, but not available!");
	}
	{
		//Ӧ����Ϣ
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//������Ϣ
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		//��ȡ��չ��Ϣ
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

	//��֤����Ϣ�ص�
	if (enableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(pRenderer.pVkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			SHEN_CORE_ERROR("failed to set up debug messenger!");
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//ѡȡ�豸
	{
		//��ȡ�豸������
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(pRenderer.pVkInstance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			SHEN_CORE_ERROR("failed to find GPUs with Vulkan support!");
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		//ö�����е��豸
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(pRenderer.pVkInstance, &deviceCount, devices.data());

		//ѡȡ֧�� ͼ�񣬼������ʾ���ܵ��Կ�
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

	//�����߼��豸
	{
		//��ȡ�豸��������Ϣ
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
	//ͼ�ζ���
	if (queueType == QUEUE_TYPE_GRAPHICS)
	{
		*pOutFamilyIndex = pRenderer->pVkGraphicsQueueFamilyIndex;
	}
}

/// <summary>
/// ���Ӷ�����
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
/// ����������
/// </summary>
/// <param name="pRenderer"></param>
/// <param name="p_desc"></param>
/// <param name="p_swap_chain"></param>
void addSwapChain(Renderer* pRenderer, const SwapChainDesc* pDesc, SwapChain** ppSwapChain)
{
	//����surface
	SwapChain pSwapChain;
	memset(&pSwapChain, 0, sizeof(pSwapChain));
	if (glfwCreateWindowSurface(pRenderer->pVkInstance, (GLFWwindow*)pDesc->mWindow, nullptr, &pSwapChain.pVkSurface) != VK_SUCCESS) {
		SHEN_CORE_ERROR("failed to create window surface!");
		throw std::runtime_error("failed to create window surface!");
	}

	//����������

}