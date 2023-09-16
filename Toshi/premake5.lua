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
		"fmod_vc.lib",
		"fmodstudio_vc.lib",
		"fsbank_vc.lib",
		"d3d11.lib",
		"d3dcompiler.lib",
		"dxguid.lib",
		"dxgi.lib",
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
		
		files
		{
			"Source/ToshiPCH.h",
			"Source/ToshiPCH.cpp",
			"Source/Platform/Windows/**.h",
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.c",
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
	
	filter "options:renderer=DX11"
		defines
		{
			"TOSHI_RENDERER_DX11"
		}
		
		files
		{
			"Source/Platform/DX11/**.h",
			"Source/Platform/DX11/**.cpp",
			"Source/Platform/DX11/**.c",
		}
		
	filter "options:renderer=OpenGL"
		defines
		{
			"TOSHI_RENDERER_OPENGL",
			"GLEW_STATIC"
		}
		
		libdirs
		{
			"%{LibDir.glew}"
		}
		
		files
		{
			"Source/Platform/SDL/**.h",
			"Source/Platform/SDL/**.cpp",
			"Source/Platform/SDL/**.c",
		}
		
		includedirs
		{
			"%{IncludeDir.sdl2}",
			"%{IncludeDir.glm}",
			"%{IncludeDir.glew}"
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