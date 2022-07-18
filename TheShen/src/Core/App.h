#pragma once
#include "Core/Log.h"


class App
{
public:
	virtual bool Init() = 0;

	virtual bool Load() = 0;

	virtual void Draw() = 0;

	virtual const char* GetName() = 0;

	struct Settings
	{
		/// Window width
		int32_t  mWidth = -1;
		/// Window height
		int32_t  mHeight = -1;
	} mSettings;

	static int			argc;
	static const char** argv;
};


#define DEFINE_APPLICATION_MAIN(appClass)							\
	int App::argc;													\
	const char** App::argv;											\
	extern int CreateApplication(int argc,char** argv,App* app);	\
																	\
	int main(int argc,char** argv)									\
	{																\
		App::argc = argc;											\
		App::argv = (const char**)argv;								\
		appClass app;												\
		return CreateApplication(argc, argv, &app);					\
	}																\