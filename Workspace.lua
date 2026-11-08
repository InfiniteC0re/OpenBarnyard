-- The settings of the OpenBarnyard workspace, call it right after declaring a workspace that includes the
-- OpenBarnyard projects
function OpenBarnyardWorkspaceSettings()
	cppdialect "C++20"
	characterset "ASCII"

	configurations { "Debug", "Release", "Final" }
	platforms ( os.target() == "emscripten" and "WASM" or "Win32" )

	-- Enables fast builds
	if os.target() ~= "emscripten" then
		multiprocessorcompile "on"
	end

	disablewarnings
	{
		"4996",
		"4251",
		"4275"
	}
	
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
	
	filter { "options:arch=x86", "not system:emscripten" }
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

	filter "options:latest=yes"
		defines { "BARNYARD_LATEST" } -- Code from the latest version of the game
		
	--filter "options:arch=x64"
	--	architecture "x64"

	-- Global Windows parameters
	filter "system:windows"
		systemversion "latest"
		vectorextensions "SSE2"
		
		exceptionhandling "Off"
		linkoptions "/SAFESEH:NO"

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
		
	filter "options:renderer=DX11"
		defines
		{
			"TRENDERINTERFACE_DX11"
		}

	filter "options:renderer=GL"
		defines
		{
			"TRENDERINTERFACE_GL"
		}

	-- wasm32 with the layouts and the wide characters of the Windows build
	filter "system:emscripten"
		toolset "emcc"
		architecture "wasm32"
		exceptionhandling "Off"

		defines
		{
			"TOSHI_SKU_WASM"
		}

		buildoptions
		{
			"-pthread",
			"-fms-extensions",
			"-fms-compatibility",
			"-fms-compatibility-version=19.30",
			"-U_MSC_VER",
			"-U_MSC_FULL_VER",
			"-U_MSC_BUILD",
			"-U_MSC_EXTENSIONS",
			"-fdelayed-template-parsing",
			"-fdeclspec",
			"-fshort-wchar",
			-- The optimizer turns our string loops into wcslen, and the one in libc expects 4 byte characters
			"-fno-builtin-wcslen",
			"-fno-strict-aliasing",
			"-w"
		}

		linkoptions
		{
			"-pthread"
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
end
