#include <TheShen.h>
#include "Core/EntryPoint.h"
#include "TestLayer.h"


class Sandbox :public Application
{
public:
	Sandbox() {
		PushLayer(new TestLayer());
	};

	~Sandbox() {
	};

private:

};

Application* CreateApplication() {
	return new Sandbox();
}