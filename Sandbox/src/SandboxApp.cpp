#include <TheShen.h>
#include "Core/App.h"
#include "TestLayer.h"
#include "Renderer/Renderer.h"


Renderer* pRenderer = NULL;

class Sandbox :public App
{
public:
	bool Init() override 
	{
		Application::Get().PushLayer(new TestLayer());
		return 0;
	}

private:

};


DEFINE_APPLICATION_MAIN(Sandbox)