include "Settings.lua"
include "Dependencies.lua"
include "Mods/ModProject.lua"
include "Workspace.lua"

workspace "OpenBarnyard"
	OpenBarnyardWorkspaceSettings()

-- Include the projects
group "Engine"
	include "Toshi"
	include "OpenBarnyard"

group "Tools"
	include "Tools/AssetPack"
	include "Tools/TMDL"
	include "Tools/XML2PProperties"
	include "Tools/TLocaleCompiler"
	include "Tools/PPropertyCompiler"
	include "Tools/QuestCompiler"
	include "Tools/FSMCompiler"
	include "Tools/Sounds2Enum"

group "Third-Party"
	include "Toshi/Vendor/Opcode"
	include "Toshi/Vendor/minhook"

if _OPTIONS["renderer"] ~= 'DX11' then
	group "SDK"
		include "SDK/BYardSDK"
		include "SDK/BYModCore"
		include "SDK/BYSpeedrunHelper"
		include "SDK/BYTexturePacks"
		include "SDK/BYWinterMod"
end

group "Mods"
	include "Mods/BYSpeedrunHelper"

if _OPTIONS["profiler"] == 'perf' or _OPTIONS["profiler"] == 'perfmem' then
	group "Utils"
		include "Utils/TracyProfiler"
end