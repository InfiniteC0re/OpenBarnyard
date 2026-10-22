-- Common settings of OpenBarnyard mods. Use it instead of project in a mod's premake5.lua:
--
--   ModProject "MyMod"
--       files { "Source/**.h", "Source/**.cpp" }
--
-- The DLL is copied to Game/Mods/Native, where the game loads mods from.
-- The OpenBarnyard folder, this file is in OpenBarnyard/Mods
local ModProjectRoot = path.getabsolute( path.join( _SCRIPT_DIR, ".." ) )

function ModProject( name )
	project( name )
	kind "SharedLib"
	language "C++"
	staticruntime "off"

	-- Link against the game's import library, never against Toshi
	dependson { "OpenBarnyard" }
	links { "OpenBarnyard.lib" }
	libdirs { "%{wks.location}/bin/" .. outputdir .. "/OpenBarnyard" }

	defines { "MODAPI_MOD" }

	files
	{
		-- Routes the mod's new/delete to the game's allocator
		path.join( ModProjectRoot, "Toshi/Source/Mod/ModMemory.cpp" )
	}

	includedirs
	{
		path.join( ModProjectRoot, "OpenBarnyard/Source" ),
		path.join( ModProjectRoot, "Toshi/Resources" ),
		"%{IncludeDir.fmod}",
		"%{IncludeDir.bink}",
		"%{IncludeDir.opcode}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.imgui}"
	}

	debugdir ( "%{wks.location}/../Game" )
	debugcommand ( "%{wks.location}/bin/" .. outputdir .. "/OpenBarnyard/OpenBarnyard.exe" )

	postbuildcommands
	{
		"{MKDIR} \"%{wks.location}../Game/Mods/Native\"",
		"{COPYFILE} \"%{cfg.buildtarget.abspath}\" \"%{wks.location}../Game/Mods/Native/\"",
	}

	filter "files:**/Toshi/Source/Mod/ModMemory.cpp"
		enablepch "Off"

	filter {}
end
