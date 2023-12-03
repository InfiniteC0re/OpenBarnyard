#pragma once

template <class T, size_t Address>
class AHookedSingleton
{
public:
	AHookedSingleton() = delete;

	static TBOOL IsSingletonCreated() { return (*ms_ppSingleton) != TNULL; }
	static T* GetSingleton() { return *ms_ppSingleton; }
	static T* GetSingletonSafe() { TASSERT(IsSingletonCreated()); return *ms_ppSingleton; }

	inline static T** ms_ppSingleton = (T**)(Address);
};
