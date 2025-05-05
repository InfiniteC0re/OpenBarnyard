project "TMDL"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"
	
	links
	{
		"Toshi"
	}
	
	libdirs
	{
		"Vendor/Assimp/lib"
	}

	files
	{
		"Source/**.h",
		"Source/**.cpp",
	}

	includedirs
	{
		"Source",
		"Vendor/Assimp/include",
		"%{wks.location}/Toshi/Source",
	}
	
	postbuildcommands
	{
		"{COPYDIR} Content \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}\""
	}

	defines
	{
		"TOSHI_CONSOLE",
		"NOMINMAX"
	}

	filter "configurations:Debug"
		links
		{
			"assimp-vc143-mtd.lib"
		}

	filter "configurations:Release"
		links
		{
			"assimp-vc143-mt.lib"
		}

	filter "configurations:Final"
		links
		{
			"assimp-vc143-mt.lib"
		}
