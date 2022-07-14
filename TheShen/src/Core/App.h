#pragma once
#include "Core/Log.h"


class App
{
public:
	virtual bool Init() = 0;


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