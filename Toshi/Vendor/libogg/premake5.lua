project "libogg"
	kind "StaticLib"
	language "C"
	staticruntime "on"

	includedirs
	{
		"include",
	}

	files
	{
		"src/bitwise.c",
		"src/framing.c",
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