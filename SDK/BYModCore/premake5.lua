project "BYModCore"
	kind "SharedLib"
	language "C++"
	staticruntime "on"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"
	
	links
	{
		"Toshi",
		"BYardSDK",
		"detours.lib",
		"libcrypto_static.lib",
		"libssl_static.lib"
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.bink}",
		"%{LibDir.dx8}",
		"%{LibDir.detours}",
		"%{LibDir.libssl}",
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
		"%{IncludeDir.toshi}",
		"%{IncludeDir.openbarnyard}",
		"%{IncludeDir.byardsdk}",
		"%{IncludeDir.detours}",
		"%{IncludeDir.libssl}",
	}
	
	externalincludedirs
	{
		"%{IncludeDir.dx8}"
	}
	
	-- Modloader specific
	debugdir ("%{wks.location}/../Game")
	debugcommand ("%{wks.location}/../Game/BYardModLoader.exe")
	
	postbuildcommands
	{
		"{COPYDIR} \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\" %{wks.location}../Game/",
	}

	filter "system:windows"
		defines
		{
			"TOSHI_MODLOADER",
			"TOSHI_SDK"
		}
