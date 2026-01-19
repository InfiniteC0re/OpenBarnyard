project "FSMCompiler"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	
	pchheader "pch.h"
	pchsource "Source/pch.cpp"
	
	links
	{
		"Toshi",
		"XML2PProperties"
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
		"%{IncludeDir.xml2pproperties}"
	}
	
	defines
	{
		"TOSHI_CONSOLE"
	}
