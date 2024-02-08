project "OpenBarnyard"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	characterset "ASCII"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		"Toshi",
		"binkw32.lib"
	}
	
	linkoptions
	{
		"%{wks.location}/bin-int/" .. outputdir .. "/Toshi/Toshi.res",
		"/SAFESEH:NO"
	}

	files
	{
		"Source/**.h",
		"Source/**.cpp",
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.bink}",
		"%{LibDir.dx8}"
	}

	includedirs
	{
		"Source",
		"%{wks.location}/Toshi/Source",
		"%{wks.location}/Shared/Source",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.bink}",
		"%{IncludeDir.libtheora}",
		"%{IncludeDir.libogg}",
		"%{IncludeDir.libvorbis}",
		"%{IncludeDir.theoraplay}",
		"%{IncludeDir.stb}"
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
	
	prebuildcommands
	{
		"\"%{wks.location}/DevTools/PooledStrings.exe\" \"%{prj.location}Source\\APooledStrings.txt\" \"%{prj.location}Source\\APooledStrings.cpp\""
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
