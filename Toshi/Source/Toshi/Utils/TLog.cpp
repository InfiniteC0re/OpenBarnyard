#include "ToshiPCH.h"
#include "TLog.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Toshi
{
	std::shared_ptr<spdlog::logger> TLog::s_CoreLogger;
	std::shared_ptr<spdlog::logger> TLog::s_AppLogger;
	TBOOL TLog::s_Created = TFALSE;

	void TLog::Create()
	{
		if (!s_Created)
		{
			spdlog::set_pattern("%^[%H:%M:%S] [%n] %v%$");

			s_CoreLogger = spdlog::stdout_color_mt("Toshi");
			s_CoreLogger->set_level(spdlog::level::level_enum::trace);

			s_AppLogger = spdlog::stdout_color_mt("App");
			s_AppLogger->set_level(spdlog::level::level_enum::trace);
			s_Created = TTRUE;
		}
	}

	void TLog::Destroy()
	{
		spdlog::shutdown();
		s_CoreLogger = TNULL;
		s_AppLogger = TNULL;
		s_Created = TFALSE;
	}
}
