#pragma once

#ifndef TOSHI_APP_LOGNAME
#define TOSHI_APP_LOGNAME "Barnyard"
#endif

#include "Toshi/Core/Core.h"

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

#ifndef TOSHI_NO_LOGS

#ifdef TOSHI_USER_ENGINE

// Engine log macros
#define TOSHI_INFO(...) Toshi::TLog::GetCoreLogger()->info(__VA_ARGS__)
#define TOSHI_WARN(...) Toshi::TLog::GetCoreLogger()->warn(__VA_ARGS__)
#define TOSHI_ERROR(...) Toshi::TLog::GetCoreLogger()->error(__VA_ARGS__)
#define TOSHI_TRACE(...) Toshi::TLog::GetCoreLogger()->trace(__VA_ARGS__)
#define TOSHI_CRITICAL(...) Toshi::TLog::GetCoreLogger()->critical(__VA_ARGS__)

#else

// App log macros
#define TOSHI_INFO(...) Toshi::TLog::GetAppLogger()->info(__VA_ARGS__)
#define TOSHI_WARN(...) Toshi::TLog::GetAppLogger()->warn(__VA_ARGS__)
#define TOSHI_ERROR(...) Toshi::TLog::GetAppLogger()->error(__VA_ARGS__)
#define TOSHI_TRACE(...) Toshi::TLog::GetAppLogger()->trace(__VA_ARGS__)
#define TOSHI_CRITICAL(...) Toshi::TLog::GetAppLogger()->critical(__VA_ARGS__)

#endif

#else

#define TOSHI_INFO(...)
#define TOSHI_WARN(...)
#define TOSHI_ERROR(...)
#define TOSHI_TRACE(...)
#define TOSHI_CRITICAL(...)

#endif
