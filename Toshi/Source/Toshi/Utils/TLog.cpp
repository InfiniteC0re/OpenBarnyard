#include "ToshiPCH.h"
#include "TLog.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Toshi
{
	Ref<spdlog::logger> TLog::s_CoreLogger;
	Ref<spdlog::logger> TLog::s_AppLogger;

	void TLog::Init()
	{
		static bool s_Initialized = false;

		if (!s_Initialized)
		{
			spdlog::set_pattern("%^[%H:%M:%S] [%n] %v%$");

			s_CoreLogger = spdlog::stdout_color_mt("Toshi");
			s_CoreLogger->set_level(spdlog::level::level_enum::trace);

			s_AppLogger = spdlog::stdout_color_mt("App");
			s_AppLogger->set_level(spdlog::level::level_enum::trace);
			s_Initialized = true;
		}
	}
}
