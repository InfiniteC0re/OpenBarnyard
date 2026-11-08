-- The debug tooling of Prime (de Blob 2's engine) ported from OpenPrime
project "PrimeDebug"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	characterset "ASCII"

	pchheader "PrimeDebugPCH.h"
	pchsource "Source/PrimeDebugPCH.cpp"

	files
	{
		"Source/**.h",
		"Source/**.cpp"
	}

	includedirs
	{
		"Source"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	filter {}

	if os.target() == "emscripten" then
		enablepch "Off"
		forceincludes { "PrimeDebugPCH.h" }
	end
