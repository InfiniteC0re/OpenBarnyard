project "BYardSDK"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	characterset "ASCII"

	pchheader "pch.h"
	pchsource "Source/pch.cpp"
	
	ignoredefaultlibraries
	{
		"LIBCI",
	}

	links
	{
		"Toshi"
	}
	
	linkoptions
	{
		"/SAFESEH:NO"
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
	
	externalincludedirs 
	{
		"%{IncludeDir.dx8}"
	}
	
	defines
	{
		"TOSHI_USER_CLIENT",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TOSHI_SDK",
			"TOSHI_SKU_WINDOWS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "TOSHI_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		runtime "Release"
		defines "TOSHI_DIST"
		optimize "On"