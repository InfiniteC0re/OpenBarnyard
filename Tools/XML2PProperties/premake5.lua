project "XML2PProperties"
	kind "StaticLib"
	language "C++"
	staticruntime "on"
	
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
