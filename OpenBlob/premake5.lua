project ("OpenBlob (" .. _OPTIONS["renderer"] .. ")")
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	characterset "ASCII"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		TOSHI_PROJECT_NAME,
		SHARED_PROJECT_NAME,
		"libtheora",
		"theoraplay"
	}
	
	linkoptions
	{
		"%{wks.location}/bin-int/" .. outputdir .. "/Toshi/Toshi.res"
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
		"%{wks.location}/Shared/Source",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.libtheora}",
		"%{IncludeDir.libogg}",
		"%{IncludeDir.libvorbis}",
		"%{IncludeDir.theoraplay}",
		"%{IncludeDir.stb}"
	}
	
	defines
	{
		"TOSHI_USER_CLIENT",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
	}
	
	postbuildcommands
	{
		"{COPYDIR} Data \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/Data\"",
		"{COPYDIR} \"" .. ClientContentCommon  .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
		"{COPYDIR} \"" .. ClientContentArch    .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TOSHI_SKU_WINDOWS"
		}
	
	filter "options:renderer=DX11"
		defines
		{
			"TOSHI_RENDERER_DX11"
		}
		
	filter "options:renderer=OpenGL"
		defines
		{
			"TOSHI_RENDERER_OPENGL",
			"GLEW_STATIC"
		}
		
		libdirs
		{
			"%{LibDir.sdl2}",
			"%{LibDir.glew}"
		}
		
		links
		{
			"SDL2.lib",
			"opengl32.lib",
			"glew32s.lib"
		}
		
		includedirs
		{
			"%{IncludeDir.sdl2}",
			"%{IncludeDir.glm}",
			"%{IncludeDir.glew}"
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "TOSHI_DEBUG"
		symbols "On"

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		kind "WindowedApp"
		runtime "Release"
		defines "TOSHI_DIST"
		optimize "On"
