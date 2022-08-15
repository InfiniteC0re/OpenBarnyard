workspace "Toshi"
	architecture "x64"
	platforms "Windows"

	configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}_%{cfg.platform}_%{cfg.architecture}"

IncludeDir = {}
IncludeDir["spdlog"] = "%{wks.location}/Deyo/vendor/spdlog/include"

include "Toshi"
include "OpenBlob"