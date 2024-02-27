#include "pch.h"
#include "ATerrainLocator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

void ATerrainLocatorManager::LoadFromTRB(Toshi::TTRB* a_pTRB)
{
	m_pTRB = a_pTRB;
	m_pLocators = a_pTRB->GetSymbolAddress("Locators");
	m_pLocatorVIS = a_pTRB->GetSymbolAddress("LocatorVIS");

	TASSERT(m_pLocators && m_pLocatorVIS);
	TTODO("Create trees, regrowths, instances and other things from locators");
}
