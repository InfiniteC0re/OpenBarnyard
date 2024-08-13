#pragma once
#include <type_traits>

TOSHI_NAMESPACE_START

template <typename T>
class TSingleton
{
public:
	TSingleton( const TSingleton& )				= delete;
	TSingleton( const TSingleton&& )			= delete;
	TSingleton& operator=( const TSingleton& )	= delete;
	TSingleton& operator=( const TSingleton&& ) = delete;

	// Creates Singleton instance and returns it
	template <typename... Args>
	TFORCEINLINE static T* CreateSingleton( Args&&... args )
	{
		return new T( std::forward<Args>( args )... );
	}

	// Creates Singleton instance for a derived class and returns it
	template <typename D, typename... Args>
	TFORCEINLINE static D* CreateSingleton( Args&&... args )
	{
		TSTATICASSERT( std::is_base_of_v<T, D> == TTRUE );
		return new D( std::forward<Args>( args )... );
	}

	TFORCEINLINE static void DestroySingleton()
	{
		delete GetSingletonSafe();
		ms_pSingleton = TNULL;
	}

	// Returns pointer and asserts if it's not allocated
	TFORCEINLINE static T* GetSingletonSafe()
	{
		TASSERT( ms_pSingleton != TNULL, "Singleton instance is not created" );
		return ms_pSingleton;
	}

	TFORCEINLINE static TBOOL IsSingletonCreated()
	{
		return ms_pSingleton != TNULL;
	}

	// Returns pointer even if it's not allocated
	TFORCEINLINE static T* GetSingleton()
	{
		return ms_pSingleton;
	}

#if defined( TOSHI_MODLOADER ) || defined( TOSHI_MODLOADER_CLIENT )
	TFORCEINLINE static T* SetSingletonExplicit( T* a_pPtr )
	{
		ms_pSingleton = a_pPtr;
		return ms_pSingleton;
	}
#endif

protected:
	TFORCEINLINE TSingleton()
	{
		TASSERT( ms_pSingleton == TNULL, "Singleton instance is already created" );
		ms_pSingleton = static_cast<T*>( this );
	}

	TFORCEINLINE ~TSingleton() { ms_pSingleton = TNULL; }

	static T* ms_pSingleton;
};

template <typename T>
T* TSingleton<T>::ms_pSingleton = TNULL;

TOSHI_NAMESPACE_END
