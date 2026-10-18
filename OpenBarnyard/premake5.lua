project "OpenBarnyard"
	language "C++"
	staticruntime "off"
	
	-- Set debug dir to the Game folder
	debugdir ("%{wks.location}/../Game")

	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		"Toshi",
		"OPCODE",
		"MinHook",
		"binkw32.lib"
	}
	
	linkoptions
	{
		"%{wks.location}/bin-int/" .. outputdir .. "/Toshi/Toshi.res",
	}
	
	defines
	{
		"MODAPI_HOST"
	}

	files
	{
		"Source/*.h",
		"Source/*.cpp",
		"Source/APooledStrings.cpp",
		"Source/Animation/**.h",
		"Source/Animation/**.cpp",
		"Source/Assets/**.h",
		"Source/Assets/**.cpp",
		"Source/Cameras/**.h",
		"Source/Cameras/**.cpp",
		"Source/Helpers/**.h",
		"Source/Helpers/**.cpp",
		"Source/GameInterface/**.h",
		"Source/GameInterface/**.cpp",
		"Source/MiniGame/**.h",
		"Source/MiniGame/**.cpp",
		"Source/GUI/**.h",
		"Source/GUI/**.cpp",
		"Source/HUD/**.h",
		"Source/HUD/**.cpp",
		"Source/Input/**.h",
		"Source/Input/**.cpp",
		"Source/Locale/**.h",
		"Source/Locale/**.cpp",
		"Source/Memory/**.h",
		"Source/Memory/**.cpp",
		"Source/Movie/**.h",
		"Source/Movie/**.cpp",
		"Source/Render/**.h",
		"Source/Render/**.cpp",
		"Source/Sound/**.h",
		"Source/Sound/**.cpp",
		"Source/SoundBank/**.h",
		"Source/SoundBank/**.cpp",
		"Source/Tasks/**.h",
		"Source/Tasks/**.cpp",
		"Source/UnitMovement/**.h",
		"Source/UnitMovement/**.cpp",
		"Source/FX/**.h",
		"Source/FX/**.cpp",
		"Source/Terrain/**.h",
		"Source/Terrain/**.cpp",
		"Source/World/**.h",
		"Source/World/**.cpp",
		"Source/Collision/**.h",
		"Source/Collision/**.cpp",
		"Source/Physics/**.h",
		"Source/Physics/**.cpp",
		"Source/FSM/**.h",
		"Source/FSM/**.cpp",
		"Source/CVar/**.h",
		"Source/CVar/**.cpp",
		"Source/Player/**.h",
		"Source/Player/**.cpp",
		"Source/Units/**.h",
		"Source/Units/**.cpp",
		"Source/Quests/**.h",
		"Source/Quests/**.cpp",
		"Source/Mod/**.h",
		"Source/Mod/**.cpp",

		-- Dear ImGui, exported to mods
		"../Toshi/Vendor/imgui/*.h",
		"../Toshi/Vendor/imgui/*.cpp",
		"../Toshi/Vendor/imgui/backends/imgui_impl_win32.*"
	}
	
	libdirs
	{
		"%{LibDir.fmod}",
		"%{LibDir.bink}"
	}
	
	includedirs
	{
		"Source",
		"%{wks.location}/Toshi/Resources",
		"%{wks.location}/Shared/Source",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.bink}",
		"%{IncludeDir.opcode}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.minhook}",
		"%{IncludeDir.imgui}"
	}
	
	prebuildcommands
	{
		"\"%{wks.location}/DevTools/PooledStrings/PooledStrings.exe\" \"%{prj.location}Source\\APooledStrings.txt\" \"%{prj.location}Source\\APooledStrings.cpp\""
	}
	
	postbuildcommands
	{
		"{COPYDIR} \"" .. ClientContentCommon  .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
		"{COPYDIR} \"" .. ClientContentArch    .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
	}

	filter "files:**/Toshi/Vendor/imgui/**.cpp"
		enablepch "Off"

	filter "configurations:Debug"
		kind "ConsoleApp"

	filter "configurations:Release"
		kind "ConsoleApp"

	filter "configurations:Final"
		kind "WindowedApp"
	
	filter "system:windows"
		files
		{
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.h"
		}
		
		filter "options:renderer=DX8"
			libdirs
			{
				"%{LibDir.dx8}"
			}
			
			files
			{
				"Source/Platform/DX8/**.cpp",
				"Source/Platform/DX8/**.h",
				"../Toshi/Vendor/imgui/backends/imgui_impl_dx8.*"
			}
			
		filter "options:renderer=GL"
			libdirs
			{
				"%{LibDir.sdl2}"
			}
			
			files
			{
				"Source/Platform/GL/**.cpp",
				"Source/Platform/GL/**.h",
				"Source/Platform/SDL/**.cpp",
				"Source/Platform/SDL/**.h"
			}
