project "BYSpeedrunHelper"
	kind "SharedLib"
	language "C++"
	staticruntime "on"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		"Toshi",
		"BYardSDK",
		"BYModCore"
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
		"%{wks.location}/SDK/BYardSDK/Source",
		"%{wks.location}/SDK/BYModCore/Include",
		"%{IncludeDir.detours}"
	}

	filter "system:windows"
		defines
		{
			"TOSHI_SDK",
			"TOSHI_MODLOADER_CLIENT"
		}
