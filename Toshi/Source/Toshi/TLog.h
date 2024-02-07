#pragma once

#ifndef TOSHI_APP_LOGNAME
#define TOSHI_APP_LOGNAME "Barnyard"
#endif

#include <spdlog/spdlog.h>

namespace Toshi
{
	class TLog
	{
	public:
		static void Create(const char* a_szAppName = TOSHI_APP_LOGNAME);
		static void Destroy();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; };
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return s_AppLogger; };

	private:
		static TBOOL s_Created;
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_AppLogger;
	};
}

