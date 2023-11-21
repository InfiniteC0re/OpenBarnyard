project "Toshi"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	characterset "ASCII"

	pchheader "ToshiPCH.h"
	pchsource "Source/ToshiPCH.cpp"
	
	links
	{
		"d3d8.lib",
		"d3dx8.lib",
		"dxguid.lib",
		"dxgi.lib",
		"legacy_stdio_definitions.lib",
		"winmm.lib",
		"dinput8.lib"
	}

	includedirs
	{
		"Source",
		"Icons",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.stb}"
	}
	
	externalincludedirs 
	{
		"%{IncludeDir.dx8}"
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.dx8}"
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
		
		files
		{
			"Source/ToshiPCH.h",
			"Source/ToshiPCH.cpp",
			"Source/Platform/Windows/**.h",
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.c",
			"Source/Platform/DX8/**.h",
			"Source/Platform/DX8/**.cpp",
			"Source/Platform/DX8/**.c",
			"Source/Plugins**/**.h",
			"Source/Plugins**/**.cpp",
			"Source/Toshi**/**.h",
			"Source/Toshi**/**.cpp",
			"Source/Toshi**/**.c",
			"Icons/*.ico",
			"*.h",
			"*.aps",
			"*.rc"
		}

		defines
		{
			"TOSHI_SKU_WINDOWS"
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