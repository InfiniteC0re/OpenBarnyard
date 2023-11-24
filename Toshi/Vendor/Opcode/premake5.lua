project "OPCODE"
	kind "StaticLib"
	language "C"
	staticruntime "on"
	
	pchheader "StdAfx.h"
	pchsource "Source/StdAfx.cpp"

	includedirs
	{
		"Source",
	}

	files
	{
		"Source/**.cpp",
		"Source/**.c",
		"Source/**.h",
	}
	
	defines
	{
		"ICE_NO_DLL"
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