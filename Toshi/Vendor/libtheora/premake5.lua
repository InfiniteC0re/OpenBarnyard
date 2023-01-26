project "libtheora"
	kind "StaticLib"
	language "C"
	staticruntime "on"

	links 
	{
		"libogg",
		"libvorbis"
	}

	includedirs
	{
		"include",
		"%{IncludeDir.libogg}",
		"%{IncludeDir.libvorbis}"
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