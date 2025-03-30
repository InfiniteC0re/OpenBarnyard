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
		"OPCODE",
		"binkw32.lib"
	}
	
	linkoptions
	{
		"%{wks.location}/bin-int/" .. outputdir .. "/Toshi/Toshi.res",
	}
	
	defines
	{
		"BAN_OPCODE_AUTOLINK"
	}

	files
	{
		"Source/*.h",
		"Source/*.cpp",
		"Source/APooledStrings.cpp",
		"Source/Animation/**.h",
		"Source/Animation/**.cpp",
		"Source/Assets/**.h",
		"Source/Assets/**.cpp",
		"Source/Cameras/**.h",
		"Source/Cameras/**.cpp",
		"Source/Helpers/**.h",
		"Source/Helpers/**.cpp",
		"Source/GameInterface/**.h",
		"Source/GameInterface/**.cpp",
		"Source/GUI/**.h",
		"Source/GUI/**.cpp",
		"Source/Input/**.h",
		"Source/Input/**.cpp",
		"Source/Locale/**.h",
		"Source/Locale/**.cpp",
		"Source/Memory/**.h",
		"Source/Memory/**.cpp",
		"Source/Movie/**.h",
		"Source/Movie/**.cpp",
		"Source/Render/**.h",
		"Source/Render/**.cpp",
		"Source/Sound/**.h",
		"Source/Sound/**.cpp",
		"Source/SoundBank/**.h",
		"Source/SoundBank/**.cpp",
		"Source/Tasks/**.h",
		"Source/Tasks/**.cpp",
		"Source/Terrain/**.h",
		"Source/Terrain/**.cpp",
		"Source/World/**.h",
		"Source/World/**.cpp"
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.bink}"
	}
	
	includedirs
	{
		"Source",
		"%{wks.location}/Toshi/Resources",
		"%{wks.location}/Toshi/Source",
		"%{wks.location}/Shared/Source",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.bink}",
		"%{IncludeDir.opcode}",
		"%{IncludeDir.stb}"
	}
	
	prebuildcommands
	{
		"\"%{wks.location}/DevTools/PooledStrings/PooledStrings.exe\" \"%{prj.location}Source\\APooledStrings.txt\" \"%{prj.location}Source\\APooledStrings.cpp\""
	}
	
	postbuildcommands
	{
		"{COPYDIR} \"" .. ClientContentCommon  .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
		"{COPYDIR} \"" .. ClientContentArch    .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
	}

	filter "configurations:Debug"
		kind "ConsoleApp"

	filter "configurations:Release"
		kind "ConsoleApp"

	filter "configurations:Final"
		kind "WindowedApp"
	
	filter "system:windows"
		files
		{
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.h"
		}
		
		filter "options:renderer=DX8"
			libdirs
			{
				"%{LibDir.dx8}"
			}
			
			files
			{
				"Source/Platform/DX8/**.cpp",
				"Source/Platform/DX8/**.h"
			}
			
		filter "options:renderer=GL"
			libdirs
			{
				"%{LibDir.sdl2}"
			}
			
			files
			{
				"Source/Platform/GL/**.cpp",
				"Source/Platform/GL/**.h",
				"Source/Platform/SDL/**.cpp",
				"Source/Platform/SDL/**.h"
			}
