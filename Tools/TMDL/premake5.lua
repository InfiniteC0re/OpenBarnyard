project "TMDL"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	characterset "ASCII"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"
	
	ignoredefaultlibraries
	{
		"LIBCI"
	}

	links
	{
		"Toshi"
	}
	
	linkoptions
	{
		"/SAFESEH:NO"
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
		"%{IncludeDir.spdlog}"
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
	
	postbuildcommands
	{
		"{COPYDIR} Content \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}\""
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TOSHI_CONSOLE",
			"TOSHI_SKU_WINDOWS"
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "TOSHI_DEBUG"
		symbols "On"
		
		links
		{
			"assimp-vc143-mtd.lib"
		}

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"
		
		links
		{
			"assimp-vc143-mt.lib"
		}

	filter "configurations:Dist"
		kind "ConsoleApp"
		runtime "Release"
		defines "TOSHI_DIST"
		optimize "On"
		
		links
		{
			"assimp-vc143-mt.lib"
		}
		