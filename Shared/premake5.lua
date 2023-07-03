project "Shared"
	kind "SharedItems"
	language "C++"
	
	includedirs
	{
		"Source",
		"%{wks.location}/Toshi/Source",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.libtheora}",
		"%{IncludeDir.libogg}",
		"%{IncludeDir.libvorbis}",
		"%{IncludeDir.theoraplay}",
		"%{IncludeDir.stb}"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		files
		{
			"Source/Platform/Windows/**.h",
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.c",
			"Source/**.h",
			"Source/**.cpp",
			"Source/**.c"
		}
