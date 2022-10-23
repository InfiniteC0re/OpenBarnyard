#pragma once

namespace Toshi
{
	template<typename T>
	class TSingleton
	{
	public:
		TSingleton(const TSingleton&) = delete;
		TSingleton(const TSingleton&&) = delete;
		TSingleton& operator=(const TSingleton&) = delete;
		TSingleton& operator=(const TSingleton&&) = delete;

		static T* GetSingleton()
		{
			static T instance;
			return &instance;
		}

	protected:
		TSingleton() = default;
	};
}