include "Settings.lua"
include "Dependencies.lua"

workspace "Toshi"
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
	include "Shared"
	include "OpenBlob"
    include "OpenJPOG"
    include "OpenGlobs"

group "Toshi Tools"
	include "Tools/TRBF"
	include "Tools/BYardLevelCompiler"
	include "Tools/ttc"
	include "Tools/TTEX_Rev_To_Win"
	
group "Third-Party"
	include "Toshi/Vendor/libogg"
	include "Toshi/Vendor/libvorbis"
	include "Toshi/Vendor/libtheora"
	include "Toshi/Vendor/theoraplay"