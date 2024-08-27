project "BYGLRender"
	kind "SharedLib"
	language "C++"
	staticruntime "on"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		"Toshi",
		"BYardSDK",
		"BYModCore",
		"SDL2.lib",
		"opengl32.lib",
		"glew32s.lib"
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.bink}",
		"%{LibDir.dx8}",
		"%{LibDir.detours}",
		"%{wks.location}/SDK/BYGLRender/Vendor/glew/lib",
		"%{wks.location}/SDK/BYGLRender/Vendor/sdl2/lib",
	}

	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Source/**.c",
	}

	includedirs
	{
		"Source",
		"Include",
		"%{wks.location}/Toshi/Source",
		"%{wks.location}/SDK/BYardSDK/Source",
		"%{wks.location}/SDK/BYModCore/Include",
		"%{IncludeDir.detours}",
		"%{wks.location}/SDK/BYGLRender/Vendor/sdl2/include",
		"%{wks.location}/SDK/BYGLRender/Vendor/glm/include",
		"%{wks.location}/SDK/BYGLRender/Vendor/glew/include"
	}
	
	defines
	{
		"GLEW_STATIC"
	}
	
	filter "files:**.c"
		flags { "NoPCH" }

	filter "system:windows"
		defines
		{
			"TOSHI_SDK",
			"TOSHI_MODLOADER_CLIENT"
		}
