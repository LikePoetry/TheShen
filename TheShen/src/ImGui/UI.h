#pragma once
#include "imgui.h"

struct UserInterfaceDesc
{
	void* pRenderer = NULL;
	void* pGraphicsQueue = NULL;
	void* pSwapChain = NULL;
};

//To be Called at application initialization time by the App Layer;
void initUserInterface(UserInterfaceDesc* pDesc);