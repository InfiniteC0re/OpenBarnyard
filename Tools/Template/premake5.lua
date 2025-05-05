project "Template Tool"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	
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
	}
	
	defines
	{
		"TOSHI_CONSOLE"
	}
