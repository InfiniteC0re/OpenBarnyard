#include "pch.h"
#include "AGameTimeManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005469a0
AGameTimeManager::AGameTimeManager()
{
}

AGameTimeManager::~AGameTimeManager()
{
}

AGameTimeManager::DAYPHASE AGameTimeManager::GetCurrentDayPhase() const
{
	return DAYPHASE_DAWN;
}

TBOOL AGameTimeManager::IsItNight() const
{
	return GetCurrentDayPhase() == DAYPHASE_NIGHT;
}
