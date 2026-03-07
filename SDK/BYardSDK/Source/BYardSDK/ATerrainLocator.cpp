#include "pch.h"
#include "ATerrainLocator.h"

void ATerrainLocatorList::LoadFromTRB( Toshi::TTRB* a_pTRB )
{
	m_pTRB              = a_pTRB;
	m_pLocatorsHeader   = a_pTRB->CastSymbol<ATerrainLocatorTRBHeader>( "Locators" );
	m_pLocatorVISHeader = a_pTRB->CastSymbol<ATerrainLocatorVISTRBHeader>( "LocatorVIS" );

	TASSERT( m_pLocatorsHeader && m_pLocatorVISHeader );
}
