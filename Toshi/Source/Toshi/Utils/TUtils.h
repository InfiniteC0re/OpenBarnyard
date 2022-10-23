#pragma once
#include <cstdint>

namespace Toshi
{
	class TUtils
	{
	public:
		TUtils() = delete;

		static uint64_t GetUnixSeconds(uint64_t* pOut = nullptr);


	};
}