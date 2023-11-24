include "Settings.lua"
include "Dependencies.lua"

workspace "OpenBarnyard"
	platforms "Windows"
	configurations { "Debug", "Release", "Dist" }
	
	disablewarnings { "4996" }
	
	debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	filter "options:arch=x86"
		architecture "x86"
		
	filter "options:arch=x64"
		architecture "x64"

group "Engine"
	include "Toshi"
	include "OpenBarnyard"
	
group "Third-Party"
	include "Toshi/Vendor/Opcode"