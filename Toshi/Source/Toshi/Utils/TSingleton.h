#pragma once
#include <type_traits>

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
		template <typename ... Args>
		__forceinline static T* CreateSingleton(Args&& ... args)
		{
			return new T(std::forward<Args>(args)...);
		}

		// Creates Singleton instance for a derived class and returns it
		template <typename D, typename ... Args>
		__forceinline static D* CreateSingleton(Args&& ... args)
		{
			static_assert(std::is_base_of_v<T, D> == TTRUE);
			return new D(std::forward<Args>(args)...);
		}

		__forceinline static void DestroySingleton()
		{
			delete GetSingleton();
			ms_pSingleton = TNULL;
		}

		// Returns pointer and asserts if it's not allocated
		__forceinline static T* GetSingleton()
		{
			TASSERT(ms_pSingleton != TNULL, "TSingleton::GetSingleton() - ms_pSingleton is TNULL");
			return ms_pSingleton;
		}

		__forceinline static TBOOL IsSingletonCreated()
		{
			return ms_pSingleton != TNULL;
		}

		// Returns pointer even if it's not allocated
		__forceinline static T* GetSingletonWeak()
		{
			return ms_pSingleton;
		}

	protected:
		__forceinline TSingleton()
		{
			TASSERT(ms_pSingleton == TNULL, "Trying to create TSingleton class twice");
			ms_pSingleton = static_cast<T*>(this);
		}

		__forceinline ~TSingleton() { ms_pSingleton = TNULL; }

		static T* ms_pSingleton;
	};

	template<typename T>
	T* TSingleton<T>::ms_pSingleton = TNULL;
}