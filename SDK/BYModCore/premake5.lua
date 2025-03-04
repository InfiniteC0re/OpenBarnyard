project "BYModCore"
	kind "SharedLib"
	language "C++"
	staticruntime "on"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"
	
	links
	{
		"Toshi",
		"BYardSDK",
		"detours.lib",
		"libcrypto_static.lib",
		"libssl_static.lib"
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.bink}",
		"%{LibDir.dx8}",
		"%{LibDir.detours}",
		"%{LibDir.libssl}",
	}

	files
	{
		"Source/**.h",
		"Source/**.cpp",
	}

	includedirs
	{
		"Source",
		"Include",
		"%{wks.location}/Toshi/Source",
		"%{wks.location}/SDK/BYardSDK/Source",
		"%{IncludeDir.detours}",
		"%{IncludeDir.libssl}",
	}
	
	externalincludedirs
	{
		"%{IncludeDir.dx8}"
	}

	filter "system:windows"
		defines
		{
			"TOSHI_MODLOADER",
			"TOSHI_SDK"
		}
