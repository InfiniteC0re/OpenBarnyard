project "BYardSDK"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

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
		"%{wks.location}/Toshi/Source",
		"%{IncludeDir.detours}"
	}
	
	filter "system:windows"
		defines
		{
			"TOSHI_SDK"
		}
