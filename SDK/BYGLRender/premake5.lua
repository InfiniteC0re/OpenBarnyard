project "BYGLRender"
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
		"Toshi",
		"BYardSDK",
		"BYModCore",
		"SDL2.lib",
		"opengl32.lib",
		"glew32s.lib"
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
		"%{wks.location}/SDK/BYGLRender/Vendor/glew/lib",
		"%{wks.location}/SDK/BYGLRender/Vendor/sdl2/lib",
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
		"%{wks.location}/SDK/BYModCore/Include",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.detours}",
		"%{wks.location}/SDK/BYGLRender/Vendor/sdl2/include",
		"%{wks.location}/SDK/BYGLRender/Vendor/glm/include",
		"%{wks.location}/SDK/BYGLRender/Vendor/glew/include"
		
	}
	
	externalincludedirs 
	{
		"%{IncludeDir.dx8}"
	}
	
	defines
	{
		"TOSHI_USER_CLIENT",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT",
		"GLEW_STATIC"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TOSHI_SDK",
			"TOSHI_MODLOADER_CLIENT",
			"TOSHI_SKU_WINDOWS"
		}

	filter "configurations:Debug"
		kind "SharedLib"
		runtime "Debug"
		defines "TOSHI_DEBUG"
		symbols "On"

	filter "configurations:Release"
		kind "SharedLib"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		kind "SharedLib"
		runtime "Release"
		defines "TOSHI_DIST"
		optimize "On"