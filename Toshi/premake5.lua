project "Toshi"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	characterset "ASCII"

	pchheader "ToshiPCH.h"
	pchsource "Source/ToshiPCH.cpp"
	
	files
	{
		"Source/ToshiPCH.cpp",
		"Source/Mod/**.h",
		"Source/Core/**.h",
		"Source/Core/**.cpp",
		"Source/Toshi/**.h",
		"Source/Toshi/**.cpp",
		"Source/ToshiTools/**.h",
		"Source/ToshiTools/**.cpp",
		"Source/File/**.h",
		"Source/File/**.cpp",
		"Source/Math/**.h",
		"Source/Math/**.cpp",
		"Source/Input/**.h",
		"Source/Input/**.cpp",
		"Source/Render/**.h",
		"Source/Render/**.cpp",
		"Source/T2Locale/**.h",
		"Source/T2Locale/**.cpp",
		"Source/Thread/**.h",
		"Source/Thread/**.cpp",
		"Source/Plugins/**.h",
		"Source/Plugins/**.cpp",
		"Source/GUI/**.h",
		"Source/GUI/**.cpp",
		"Source/TGUI/**.h",
		"Source/TGUI/**.cpp"
	}

	includedirs
	{
		"Source",
		"Icons",
		"Resources",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.stb}"
	}
	
	libdirs
	{
		"%{LibDir.fmod}"
	}

	defines
	{
		"TOSHI_USER_ENGINE",
		"_CRT_SECURE_NO_WARNINGS",
		"MODAPI_HOST"
	}
	
	filter "files:**.c"
		enablepch "Off"

	filter "files:Vendor/DirectXTex/**"
		enablepch "Off"
		
	filter "files:Source/Profiler/**.*"
		enablepch "Off"

	filter "system:windows"
		systemversion "latest"
		
		files
		{
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.c",
			"Source/Platform/Windows/**.h",
			"Icons/*.ico",
			"Resources/*.h",
			"Resources/*.aps",
			"Resources/*.rc"
		}
		
		filter "options:renderer=DX8"
			links
			{
				"d3d8.lib",
				"d3dx8.lib",
				"dxguid.lib",
				"dxgi.lib",
				"legacy_stdio_definitions.lib",
				"winmm.lib",
				"dinput8.lib"
			}
			
			externalincludedirs 
			{
				"%{IncludeDir.dx8}"
			}
	
			libdirs
			{
				"%{LibDir.dx8}"
			}

			files
			{
				"Source/Platform/DX8/**.cpp",
				"Source/Platform/DX8/**.c",
				"Source/Platform/DX8/**.h"
			}
			
		filter "options:renderer=DX11"
			links
			{
				"d3d11.lib",
				"dxgi.lib",
				"d3dcompiler.lib",
				"dxguid.lib",
				"legacy_stdio_definitions.lib",
				"winmm.lib",
				"dinput8.lib"
			}

			includedirs
			{
				"%{IncludeDir.directxtex}"
			}

			files
			{
				"Source/Platform/DX11/**.cpp",
				"Source/Platform/DX11/**.c",
				"Source/Platform/DX11/**.h",
				"Vendor/DirectXTex/*.cpp",
				"Vendor/DirectXTex/*.h",
				"Vendor/DirectXTex/*.inl"
			}

		defines
		{
			"TOSHI_SKU_WINDOWS"
		}

	filter "configurations:Debug"
		defines "TOSHI_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "TOSHI_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Final"
		defines "TOSHI_FINAL"
		runtime "Release"
		optimize "On"