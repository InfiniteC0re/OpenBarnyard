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
	
	files
	{
		"Source/ToshiPCH.cpp",
		"Source/Profiler/TracyClient.cpp",
		"Source/Core/**.h",
		"Source/Core/**.cpp",
		"Source/Toshi/**.h",
		"Source/Toshi/**.cpp",
		"Source/ToshiTools/**.h",
		"Source/ToshiTools/**.cpp",
		"Source/File/**.h",
		"Source/File/**.cpp",
		"Source/Math/**.h",
		"Source/Math/**.cpp",
		"Source/Input/**.h",
		"Source/Input/**.cpp",
		"Source/Render/**.h",
		"Source/Render/**.cpp",
		"Source/T2Locale/**.h",
		"Source/T2Locale/**.cpp",
		"Source/Thread/**.h",
		"Source/Thread/**.cpp",
		"Source/Plugins/**.h",
		"Source/Plugins/**.cpp"
	}

	includedirs
	{
		"Source",
		"Icons",
		"Resources",
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
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	filter "files:**.c"
		flags { "NoPCH" }
		
	filter "files:Source/Profiler/**.*"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		
		files
		{
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.c",
			"Source/Platform/Windows/**.h",
			"Icons/*.ico",
			"Resources/*.h",
			"Resources/*.aps",
			"Resources/*.rc"
		}
		
		filter "options:renderer=DX8"
			files
			{
				"Source/Platform/DX8/**.cpp",
				"Source/Platform/DX8/**.c"
			}
			
		filter "options:renderer=GL"
			files
			{
				"Source/Platform/GL/**.cpp",
				"Source/Platform/GL/**.c"
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

	filter "configurations:Final"
		defines "TOSHI_FINAL"
		runtime "Release"
		optimize "On"