project "libvorbis"
	kind "StaticLib"
	language "C"
	staticruntime "on"

	links 
	{
		"libogg"
	}

	includedirs
	{
		"include",
		"%{IncludeDir.libogg}"
	}

	files
	{
		"lib/*.c",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		runtime "Release"
		optimize "On"