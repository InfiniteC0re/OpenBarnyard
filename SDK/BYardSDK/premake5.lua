project "BYardSDK"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	pchheader "pch.h"
	pchsource "Source/pch.cpp"
	
	links
	{
		"Toshi"
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.bink}",
		"%{LibDir.dx8}",
		"%{LibDir.detours}",
	}

	files
	{
		"Source/**.h",
		"Source/**.cpp",
	}

	includedirs
	{
		"Source",
		"%{IncludeDir.toshi}",
		"%{IncludeDir.detours}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.sdl2}",
		"%{IncludeDir.glew}"
	}
	
	externalincludedirs
	{
		"%{IncludeDir.dx8}"
	}
	
	filter "system:windows"
		defines
		{
			"TOSHI_SDK"
		}
