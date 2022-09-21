workspace "Toshi"
	architecture "x86"
	platforms "Windows"

	configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}_%{cfg.platform}_%{cfg.architecture}"

IncludeDir = {}
IncludeDir["spdlog"] = "%{wks.location}/Toshi/vendor/spdlog/include"

include "Toshi"
include "OpenBlob"