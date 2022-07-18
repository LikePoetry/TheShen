#include <TheShen.h>
#include "Core/App.h"
#include "TestLayer.h"
#include "Renderer/Renderer.h"


Renderer* pRenderer = NULL;
Queue* pGraphicsQueue = NULL;
SwapChain* pSwapChain = NULL;

std::vector<Texture> pTextures;

class Sandbox :public App
{
public:

	/// <summary>
	/// 读取文件
	/// </summary>
	/// <param name="filename"></param>
	/// <returns></returns>
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	bool Init() override
	{
		//初始化Instance 到 LogicalDevice
		RendererDesc settings;
		memset(&settings, 0, sizeof(settings));
		SwapChainDesc swapChainDesc = {};
		swapChainDesc.mWindow = Application::Get().GetNativeWindow();
		swapChainDesc.mHeight = mSettings.mHeight;
		swapChainDesc.mWidth = mSettings.mWidth;

		initRenderer(GetName(), &settings, &pRenderer, &swapChainDesc, &pSwapChain, pTextures);
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
		{
			GraphicsPipelineDesc graphicsPipelineDesc = {};

			auto vertShaderCode = readFile("F:/VulkanWorkarea/TheShenProject_github/TheShen/bin/Debug-windows-x86_64/Sandbox/shaders/vert.spv");
			auto fragShaderCode = readFile("F:/VulkanWorkarea/TheShenProject_github/TheShen/bin/Debug-windows-x86_64/Sandbox/shaders/frag.spv");

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = vertShaderCode.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());
			VkShaderModule shaderModule;
			if (vkCreateShaderModule(pRenderer->pVkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}

			Shader shader = {};
			shader.pShaderModules = &shaderModule;

			graphicsPipelineDesc.pShaders[0] = shader;
		}


		return true;
	}

	const char* GetName() { return "TheShen"; }

private:

};


DEFINE_APPLICATION_MAIN(Sandbox)