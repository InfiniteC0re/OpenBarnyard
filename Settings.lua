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

outputdir = "%{cfg.buildcfg}_%{cfg.platform}_%{cfg.architecture}"