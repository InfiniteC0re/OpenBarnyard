project "OpenJPOG"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	characterset "ASCII"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		"Toshi",
		"libtheora",
		"theoraplay"
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
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT",
		"TOSHI_ENABLE_DEPRECATED"
	}
	
	postbuildcommands
	{
		"{COPYDIR} Data %{wks.location}bin/" .. outputdir .. "/%{prj.name}/Data",
		"{COPYDIR} " .. ClientContentCommon  .. " %{wks.location}bin/" .. outputdir .. "/%{prj.name}/",
		"{COPYDIR} " .. ClientContentArch    .. " %{wks.location}bin/" .. outputdir .. "/%{prj.name}/",
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TOSHI_PLATFORM_WINDOWS"
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