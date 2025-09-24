newoption
{
	trigger = "arch",
	description = "Choose architecture you want to build for",
	allowed = {
		{ "x86", "32bit"},
		{ "x64", "64bit"},
	},
	default = "x86"
}

newoption
{
	trigger = "renderer",
	description = "Choose renderer API you want to build for",
	allowed = {
		{ "DX8", "DirectX 8"},
		{ "GL", "OpenGL"},
	},
	default = "DX8"
}

newoption
{
	trigger = "profiler",
	description = "Enable or disable Tracy profiler support",
	allowed = {
		{ "no", "Disable Tracy"},
		{ "perf", "Enable Tracy"},
		{ "perfmem", "Enable Tracy with memory debugging"},
	},
	default = "no"
}

newoption
{
	trigger = "dlmalloc",
	description = "Enable or disable dlmalloc instead of Toshi engine TMemory",
	allowed = {
		{ "no", "Use legacy Toshi memory allocator"},
		{ "yes", "Use dlmalloc as a memory allocator"},
	},
	default = "no"
}

outputdir = ("%{cfg.buildcfg}_%{cfg.platform}_%{cfg.architecture}_" .. _OPTIONS["renderer"])