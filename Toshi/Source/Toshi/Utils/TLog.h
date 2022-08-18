#pragma once
#include "Toshi/Core/Core.h"
#include <spdlog/spdlog.h>

namespace Toshi
{
	class TLog
	{
	public:
		static void Init();

		static inline Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; };
		static inline Ref<spdlog::logger>& GetAppLogger() { return s_AppLogger; };

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_AppLogger;
	};
}

// Engine log macros
#define TOSHI_CORE_INFO(...) Toshi::TLog::GetCoreLogger()->info(__VA_ARGS__)
#define TOSHI_CORE_WARN(...) Toshi::TLog::GetCoreLogger()->warn(__VA_ARGS__)
#define TOSHI_CORE_ERROR(...) Toshi::TLog::GetCoreLogger()->error(__VA_ARGS__)
#define TOSHI_CORE_TRACE(...) Toshi::TLog::GetCoreLogger()->trace(__VA_ARGS__)
#define TOSHI_CORE_CRITICAL(...) Toshi::TLog::GetCoreLogger()->critical(__VA_ARGS__)

// App log macros
#define TOSHI_INFO(...) Toshi::TLog::GetAppLogger()->info(__VA_ARGS__)
#define TOSHI_WARN(...) Toshi::TLog::GetAppLogger()->warn(__VA_ARGS__)
#define TOSHI_ERROR(...) Toshi::TLog::GetAppLogger()->error(__VA_ARGS__)
#define TOSHI_TRACE(...) Toshi::TLog::GetAppLogger()->trace(__VA_ARGS__)
#define TOSHI_CRITICAL(...) Toshi::TLog::GetAppLogger()->critical(__VA_ARGS__)