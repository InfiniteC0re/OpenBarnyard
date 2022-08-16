project "Toshi"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	files
	{
		"Source/**.h",
		"Source/**.cpp"
	}

	includedirs
	{
		"Source",
		"%{IncludeDir.spdlog}"
	}
	
	defines
	{
		"TOSHI_USER_ENGINE"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TOSHI_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "TOSHI_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "TOSHI_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "TOSHI_DIST"
		runtime "Release"
		optimize "On"