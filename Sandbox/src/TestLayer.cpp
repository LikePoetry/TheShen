#include "TestLayer.h"

TestLayer::TestLayer()
	:Layer("Test Layer")
{

}

void TestLayer::OnAttach()
{
	SHEN_CLIENT_ERROR("Test Layer Attach");
}

void TestLayer::OnDetach()
{
}

void TestLayer::OnUpdate(Timestep ts)
{
}

void TestLayer::OnImGuiRender()
{

}

void TestLayer::OnEvent(Event& e)
{

}
