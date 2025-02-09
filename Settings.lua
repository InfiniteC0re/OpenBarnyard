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

outputdir = ("%{cfg.buildcfg}_%{cfg.platform}_%{cfg.architecture}_" .. _OPTIONS["renderer"])