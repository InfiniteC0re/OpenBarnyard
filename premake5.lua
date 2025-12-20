include "Settings.lua"
include "Dependencies.lua"

workspace "OpenBarnyard"
	cppdialect "C++20"
	characterset "ASCII"

	platforms "Windows"
	configurations { "Debug", "Release", "Final" }

	disablewarnings { "4996" }
	
	debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	editandcontinue "Off"
	
	-- Global defines
	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"NOMINMAX",
		"BAN_OPCODE_AUTOLINK",
		"ICE_NO_DLL",
		"BARNYARD_COMMUNITY_PATCH", -- Optimize loading speed, fix bugs and more...
		"USE_ATOMIC"                -- Utilize alternative TMutex, TSemaphore and TFifo implementation on Windows platform
	}

	-- Global settings for include dirs
	includedirs
	{
		"%{IncludeDir.toshi}"
	}
	
	filter "options:arch=x86"
		architecture "x86"

	filter { "options:profiler=perf or options:profiler=perfmem" }
		defines
		{
			"TOSHI_PROFILER",
			"TRACY_ENABLE"
		}

		links { "TracyProfiler" }

		includedirs { "%{IncludeDir.tracy}" }

	filter "options:profiler=perfmem"
		defines { "TOSHI_PROFILER_MEMORY" }

	filter "options:dlmalloc=yes"
		defines { "TMEMORY_USE_DLMALLOC" }
		
	--filter "options:arch=x64"
	--	architecture "x64"

	-- Global Windows parameters
	filter "system:windows"
		systemversion "latest"
		
		vectorextensions "SSE2"
		
		linkoptions
		{
			"/SAFESEH:NO"
		}

		defines
		{
			"TOSHI_SKU_WINDOWS"
		}
		
	filter "options:renderer=DX8"
		externalincludedirs
		{
			"%{IncludeDir.dx8}"
		}
			
		defines
		{
			"TRENDERINTERFACE_DX8"
		}
		
	filter "options:renderer=GL"
		defines
		{
			"TRENDERINTERFACE_GL",
			"GLEW_STATIC",
			"GLM_FORCE_LEFT_HANDED"
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "TOSHI_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"

	filter "configurations:Final"
		runtime "Release"
		defines "TOSHI_FINAL"
		optimize "On"

filter {}

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
	include "Tools/Sounds2Enum"

group "Third-Party"
	include "Toshi/Vendor/Opcode"

group "SDK"
	include "SDK/BYardSDK"
	include "SDK/BYModCore"
	include "SDK/BYSpeedrunHelper"
	include "SDK/BYTexturePacks"
	include "SDK/BYWinterMod"

	if _OPTIONS["renderer"] == 'GL' then
		include "SDK/BYEnhanced"
	end

if _OPTIONS["profiler"] == 'perf' or _OPTIONS["profiler"] == 'perfmem' then
	group "Utils"
		include "Utils/TracyProfiler"
end