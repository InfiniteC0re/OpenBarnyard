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
		"PrimeDebug",
		"OPCODE"
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
		"Source/Debug/**.h",
		"Source/Debug/**.cpp",
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
		OpenBarnyardDir .. "/PrimeDebug/Source",
		OpenBarnyardDir .. "/Toshi/Resources",
		OpenBarnyardDir .. "/Shared/Source",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.bink}",
		"%{IncludeDir.opcode}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.minhook}",
		"%{IncludeDir.imgui}"
	}

	if os.target() == "emscripten" then
		prebuildcommands
		{
			"./DevTools/PooledStrings/PooledStrings.exe OpenBarnyard/Source/APooledStrings.txt OpenBarnyard/Source/APooledStrings.cpp"
		}
	else
		prebuildcommands
		{
			"\"" .. OpenBarnyardDir .. "/DevTools/PooledStrings/PooledStrings.exe\" \"%{prj.location}Source\\APooledStrings.txt\" \"%{prj.location}Source\\APooledStrings.cpp\""
		}

		postbuildcommands
		{
			"{COPYDIR} \"" .. ClientContentCommon  .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
			"{COPYDIR} \"" .. ClientContentArch    .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
		}
	end

	filter "system:windows"
		links
		{
			"MinHook",
			"binkw32.lib"
		}

		linkoptions
		{
			"%{wks.location}/bin-int/" .. outputdir .. "/Toshi/Toshi.res",
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

		filter "options:renderer=DX11"
			files
			{
				"Source/Platform/DX11/**.cpp",
				"Source/Platform/DX11/**.h",
				"../Toshi/Vendor/imgui/backends/imgui_impl_dx11.*"
			}

		filter "options:renderer=GL"
			includedirs
			{
				"%{IncludeDir.glew}"
			}

			libdirs
			{
				"%{LibDir.glew}"
			}

			files
			{
				"Source/Platform/GL/**.cpp",
				"Source/Platform/GL/**.h"
			}

	-- The mods are DLLs, the movies and the sound go through the shims of Platform/Web
	filter {}

	if os.target() == "emscripten" then
		enablepch "Off"
		forceincludes { "pch.h" }
		removeincludedirs { "%{IncludeDir.bink}" }

		includedirs
		{
			OpenBarnyardDir .. "/Toshi/Source/Platform/WASM/Compat"
		}

		files
		{
			"Source/Platform/GL/**.cpp",
			"Source/Platform/GL/**.h",
			"Source/Platform/WASM/**.cpp",
			"Source/Platform/WASM/**.h",
			"Source/Platform/Windows/ARenderer_Win.cpp",
			"Source/Platform/Windows/ADisplayModes_Win.*",
			"Source/Platform/Windows/Input/AInputHandler_Win.cpp"
		}

		removefiles
		{
			"Source/Mod/AModLoader.*",
			"Source/Mod/AModHooks.*",
			"Source/Mod/AModExports.*",
			"Source/Mod/ImGui/**",
			"../Toshi/Vendor/imgui/**"
		}

		targetextension ".js"

		linkoptions
		{
			"-sPROXY_TO_PTHREAD",
			"-sEXIT_RUNTIME=0",
			"-sINITIAL_MEMORY=384MB",
			"-sMAXIMUM_MEMORY=1024MB",
			"-sALLOW_MEMORY_GROWTH",
			"-sDEFAULT_PTHREAD_STACK_SIZE=8MB",
			"-sSTACK_SIZE=16MB",
			"-sPTHREAD_POOL_SIZE=4",
			"-sENVIRONMENT=web,worker",
			"-sEXPORTED_RUNTIME_METHODS=wasmMemory,UTF8ToString,stringToUTF8,lengthBytesUTF8,HEAPU8,HEAP32,FS,IDBFS,addRunDependency,removeRunDependency",
			"-lidbfs.js",
			"-sFORCE_FILESYSTEM",
			"--profiling-funcs",
			"--pre-js OpenBarnyard/WASM/game_pre.js"
		}
	end
