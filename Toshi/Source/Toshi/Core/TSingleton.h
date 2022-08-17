#pragma once

namespace Toshi
{
	template<typename T>
	class TSingleton
	{
	public:
		inline static T& const Instance()
		{
			static T s_Instance;
			return s_Instance;
		}
	};
}