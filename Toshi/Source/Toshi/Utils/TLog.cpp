#include "ToshiPCH.h"
#include "TLog.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Toshi
{
	STL::Ref<spdlog::logger> TLog::s_CoreLogger;
	STL::Ref<spdlog::logger> TLog::s_AppLogger;
	bool TLog::s_Created = false;

	void TLog::Create()
	{
		if (!s_Created)
		{
			spdlog::set_pattern("%^[%H:%M:%S] [%n] %v%$");

			s_CoreLogger = spdlog::stdout_color_mt("Toshi");
			s_CoreLogger->set_level(spdlog::level::level_enum::trace);

			s_AppLogger = spdlog::stdout_color_mt("App");
			s_AppLogger->set_level(spdlog::level::level_enum::trace);
			s_Created = true;
		}
	}

	void TLog::Destroy()
	{
		spdlog::drop_all();
		s_CoreLogger = nullptr;
		s_AppLogger = nullptr;
		s_Created = false;
	}
}
