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

		// Creates Singleton instance and returns it
		static T* CreateSingleton()
		{
			return new T;
		}

		// Returns pointer and asserts if it's not allocated
		static T* GetSingleton()
		{
			TASSERT(ms_pSingleton != TNULL, "TSingleton::GetSingleton() - ms_pSingleton is TNULL");
			return ms_pSingleton;
		}

		// Returns pointer even if it's not allocated
		static T* GetSingletonWeak()
		{
			return ms_pSingleton;
		}

	protected:
		TSingleton()
		{
			TASSERT(ms_pSingleton == TNULL, "Trying to create TSingleton class twice");
			ms_pSingleton = static_cast<T*>(this);
		}

		~TSingleton() { ms_pSingleton = TNULL; }

		static T* ms_pSingleton;
	};

	template<typename T>
	T* TSingleton<T>::ms_pSingleton = TNULL;
}