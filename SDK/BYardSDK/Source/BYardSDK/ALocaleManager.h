#pragma once
#include <T2Locale/T2Locale.h>

#include "THookedSingleton.h"

class ALocaleManager
    : public Toshi::T2Locale
    , public THookedSingleton<ALocaleManager, 0x007ce6bc>
{
public:
	using THookedSingleton<ALocaleManager, 0x007ce6bc>::GetSingleton;
	using THookedSingleton<ALocaleManager, 0x007ce6bc>::GetSingletonSafe;
	using THookedSingleton<ALocaleManager, 0x007ce6bc>::IsSingletonCreated;
	using THookedSingleton<ALocaleManager, 0x007ce6bc>::SetSingleton;
};

