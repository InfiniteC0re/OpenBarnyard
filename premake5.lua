include "Settings.lua"
include "Dependencies.lua"

workspace "OpenBarnyard"
	platforms "Windows"
	configurations { "Debug", "Release", "Dist" }
	
	disablewarnings { "4996" }
	
	debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	editandcontinue "Off"
	
	--defines
	--{
	--	"TOSHI_PROFILER",
	--	"TOSHI_PROFILER_MEMORY",
	--	"TRACY_ENABLE"
	--}
	
	filter "options:arch=x86"
		architecture "x86"
		
	filter "options:arch=x64"
		architecture "x64"

group "Engine"
	include "Toshi"
	include "OpenBarnyard"

group "Tools"
	include "Tools/AssetPack"
	include "Tools/TMDL"
	include "Tools/XML2PProperties"
	include "Tools/TLocaleCompiler"
		
group "Third-Party"
	include "Toshi/Vendor/Opcode"

group "SDK"
	include "SDK/BYardSDK"
	include "SDK/BYModCore"
	include "SDK/BYSpeedrunHelper"
	include "SDK/BYTexturePacks"
	include "SDK/BYWinterMod"
	include "SDK/BYGLRender"