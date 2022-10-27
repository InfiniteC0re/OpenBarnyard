project "Toshi"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	pchheader "ToshiPCH.h"
	pchsource "Source/ToshiPCH.cpp"
	
	links
	{
		"fmod_vc.lib"
	}

	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Source/**.c"
	}

	includedirs
	{
		"Source",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.fmod}"
	}
	
	libdirs
	{
		"%{LibDir.fmod}"
	}

	defines
	{
		"TOSHI_USER_ENGINE",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT",
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	filter "files:**.c"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TOSHI_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "TOSHI_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "TOSHI_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "TOSHI_DIST"
		runtime "Release"
		optimize "On"