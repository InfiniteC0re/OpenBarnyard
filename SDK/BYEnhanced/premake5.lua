project "BYEnhanced"
	kind "None"
	language "C++"
	staticruntime "on"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"
	
	filter "options:renderer=GL"
		kind "SharedLib"

	links
	{
		"Toshi",
		"BYardSDK",
		"BYModCore",
		"SDL2.lib",
		"opengl32.lib",
		"glew32s.lib"
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.bink}",
		"%{LibDir.dx8}",
		"%{LibDir.detours}",
		"%{LibDir.glew}",
		"%{LibDir.sdl2}",
	}

	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Source/**.c",
	}

	includedirs
	{
		"Source",
		"Include",
		"%{wks.location}/Toshi/Source",
		"%{wks.location}/SDK/BYardSDK/Source",
		"%{wks.location}/SDK/BYModCore/Include",
		"%{IncludeDir.detours}",
		"%{IncludeDir.sdl2}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.glew}"
	}
	
	externalincludedirs
	{
		"%{IncludeDir.dx8}"
	}
	
	defines
	{
		"GLEW_STATIC"
	}
	
	-- Modloader specific
	debugdir ("%{wks.location}/../Game")
	debugcommand ("%{wks.location}/../Game/BYardModLoader.exe")
	
	postbuildcommands
	{
		"{COPYDIR} \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\" %{wks.location}../Game/Mods/",
	}
	
	filter "files:**.c"
		flags { "NoPCH" }

	filter "system:windows"
		defines
		{
			"TOSHI_SDK",
			"TOSHI_MODLOADER_CLIENT"
		}
