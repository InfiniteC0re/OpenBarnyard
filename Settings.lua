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
	description = "Choose renderer api you want to use in the build",
	allowed = {
		{ "DX11", "DirectX 11 (Windows only)"},
		{ "OpenGL", "OpenGL (SDL)"},
	},
	default = "DX11"
}


outputdir = "%{cfg.buildcfg}_%{cfg.platform}_%{cfg.architecture}"