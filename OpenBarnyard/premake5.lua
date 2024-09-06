project "OpenBarnyard"
	language "C++"
	staticruntime "on"
	
	-- Set debug dir to the Game folder
	debugdir ("%{wks.location}/Game")

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
	}

	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Source/APooledStrings.cpp"
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
		"%{wks.location}/Toshi/Resources",
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
	
	prebuildcommands
	{
		"\"%{wks.location}/DevTools/PooledStrings/PooledStrings.exe\" \"%{prj.location}Source\\APooledStrings.txt\" \"%{prj.location}Source\\APooledStrings.cpp\""
	}
	
	postbuildcommands
	{
		-- "{COPYDIR} Data \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/Data\"",
		"{COPYDIR} \"" .. ClientContentCommon  .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
		"{COPYDIR} \"" .. ClientContentArch    .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
	}

	filter "configurations:Debug"
		kind "ConsoleApp"

	filter "configurations:Release"
		kind "ConsoleApp"

	filter "configurations:Final"
		kind "WindowedApp"
