#include "pch.h"
#include "ATerrainLocator.h"

void ATerrainLocatorList::LoadFromTRB( Toshi::TTRB* a_pTRB )
{
	m_pTRB        = a_pTRB;
	m_pLocators   = a_pTRB->GetSymbolAddress( "Locators" );
	m_pLocatorVIS = a_pTRB->GetSymbolAddress( "LocatorVIS" );

	TASSERT( m_pLocators && m_pLocatorVIS );
}
