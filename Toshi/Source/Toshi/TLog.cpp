#include "ToshiPCH.h"
#include "TLog.h"

#include <spdlog/sinks/stdout_color_sinks.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{
	std::shared_ptr<spdlog::logger> TLog::s_CoreLogger;
	std::shared_ptr<spdlog::logger> TLog::s_AppLogger;
	TBOOL TLog::s_Created = TFALSE;

	void TLog::Create(const char* a_szAppName)
	{
#ifndef TOSHI_NO_LOGS
		if (!s_Created)
		{
			spdlog::set_pattern("%^[%H:%M:%S] [%n] %v%$");

			s_CoreLogger = spdlog::stdout_color_mt("Toshi 2.0");
			s_CoreLogger->set_level(spdlog::level::level_enum::trace);

			s_AppLogger = spdlog::stdout_color_mt(a_szAppName);
			s_AppLogger->set_level(spdlog::level::level_enum::trace);
			s_Created = TTRUE;
		}
#endif
	}

	void TLog::Destroy()
	{
#ifndef TOSHI_NO_LOGS
		spdlog::shutdown();
		s_CoreLogger = TNULL;
		s_AppLogger = TNULL;
		s_Created = TFALSE;
#endif
	}
}
