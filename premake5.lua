workspace "TheShen"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir="%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir={}
IncludeDir["GLFW"]="vendor/GLFW/include"
IncludeDir["imgui"]="vendor/imgui"
IncludeDir["glm"] = "vendor/glm"
IncludeDir["stb"] = "vendor/stb"
IncludeDir["tinyobjloader"] = "vendor/tinyobjloader"

group "Dependencies"
	include "vendor/GLFW"
	include "vendor/imgui"

group ""



project "TheShen"
	location "TheShen"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	characterset ("MBCS")

	targetdir("bin/" ..outputdir.. "/%{prj.name}")
	objdir("bin-int/" ..outputdir.. "/%{prj.name}")

	-- pchheader "hzpch.h"
	-- pchsource "Shen/src/hzpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"Vendor/stb/stb_image.h",
		"Vendor/tinyobjloader/tiny_obj_loader.h",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"VK_USE_PLATFORM_WIN32_KHR"
	}

	includedirs
	{
		"%{prj.name}/src",
		"vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinyobjloader}",
		"%VULKAN_SDK%/include"
	}

	libdirs 
	{ 
		"%VULKAN_SDK%/lib" 
	}

	links
	{
		"GLFW",
		"ImGui",
		"vulkan-1.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			GLFW_INCLUDE_NONE
		}


	filter "configurations:Debug"
		defines ""
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines ""
		runtime "Release"
		optimize "on"

project "Sandbox"

	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	characterset ("MBCS")



	targetdir("bin/" ..outputdir.. "/%{prj.name}")
	objdir("bin-int/" ..outputdir.. "/%{prj.name}")
	

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"vendor/spdlog/include",
		"TheShen/src",
		"%{IncludeDir.GLFW}",
		"%VULKAN_SDK%/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.imgui}"
	}

	links
	{
		"TheShen",
		"GLFW",
		"ImGui",
		"vulkan-1.lib"
	}

	libdirs 
	{ 
		"%VULKAN_SDK%/lib" 
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			GLFW_INCLUDE_NONE
		}


	filter "configurations:Debug"
		defines ""
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines ""
		runtime "Release"
		optimize "on"