#pragma once

template <class T, size_t Address>
class THookedSingleton
{
public:
	THookedSingleton() = delete;

	static TBOOL IsSingletonCreated() { return ( *ms_ppSingleton ) != TNULL; }
	static void	 SetSingleton( T* a_pInstance ) { *ms_ppSingleton = a_pInstance; }
	static T*	 GetSingleton() { return *ms_ppSingleton; }
	static T*	 GetSingletonSafe()
	{
		TASSERT( IsSingletonCreated() );
		return *ms_ppSingleton;
	}

	inline static T** ms_ppSingleton = (T**)( Address );
};
