#pragma once
#include "imgui.h"

#include "Renderer/Renderer.h"

struct UserInterfaceDesc
{
	void* pRenderer = NULL;
	void* pGraphicsQueue = NULL;
	void* pSwapChain = NULL;
};

//To be Called at application initialization time by the App Layer;
void initUserInterface(UserInterfaceDesc* pDesc);

//Draw Imgui components;
void cmdDrawUserInterface(void* /* Cmd* */ pCmd, uint32_t imageIndex, uint32_t currentFrame, VkFence fence);

void createImGuiCommandBuffers(std::vector<Texture> pTextures);