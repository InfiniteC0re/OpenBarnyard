project "TracyProfiler"
	kind "SharedLib"
	language "C++"
	staticruntime "off"
	
	removelinks { "TracyProfiler" }
	
	files
	{
		"Source/Profiler/TracyClient.cpp"
	}
	
	defines
	{
		"TRACY_PROFILER_EXPORTS"
	}

	includedirs
	{
		"Source",
		"Include",
	}
	
	postbuildcommands
	{
		"{COPY} \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/%{prj.name}.dll\" %{wks.location}../Game/",
	}
