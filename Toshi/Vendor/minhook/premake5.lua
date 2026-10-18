project "MinHook"
	kind "StaticLib"
	language "C"
	staticruntime "off"

	includedirs
	{
		"include",
		"src"
	}

	files
	{
		"include/**.h",
		"src/**.h",
		"src/buffer.c",
		"src/hook.c",
		"src/trampoline.c",
		"src/hde/hde32.c",
		"src/hde/hde64.c"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

	filter "configurations:Final"
		runtime "Release"
		optimize "On"
