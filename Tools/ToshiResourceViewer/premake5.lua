project "Toshi Resource Viewer"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	characterset "ASCII"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		"Toshi"
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
		"%{IncludeDir.spdlog}"
	}
	
	defines
	{
		"TOSHI_USER_CLIENT",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
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
			"TOSHI_CONSOLE",
			"TOSHI_SKU_WINDOWS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "TOSHI_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		runtime "Release"
		defines "TOSHI_DIST"
		optimize "On"