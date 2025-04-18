#include "pch.h"
#include "ATerrainLocator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005ec800
ATerrainLocatorManager::ATerrainLocatorManager()
    : m_pTRB( TNULL )
    , m_pLocatorsHeader( TNULL )
    , m_pLocatorVISHeader( TNULL )
    , m_bCreatedGrowings( TFALSE )
{
	m_oWorldTransform.SetMatrix( TMatrix44{
	    1.0f,
	    0.0f,
	    0.0f,
	    0.0f,
	    0.0f,
	    0.0f,
	    1.0f,
	    0.0f,
	    0.0f,
	    -1.0f,
	    0.0f,
	    0.0f,
	    0.0f,
	    0.0f,
	    0.0f,
	    1.0f,
	} );
}

ATerrainLocatorManager::~ATerrainLocatorManager()
{
}

// $Barnyard: FUNCTION 005ec8b0
ATerrainLocator* ATerrainLocatorManager::FindLocator( const TCHAR* a_pszLocatorName )
{
	TVALIDPTR( m_pLocatorsHeader );

	if ( m_pLocatorVISHeader == TNULL )
	// No VIS data is provided, look for all the locators
	{
		for ( TINT i = 0; i < m_pLocatorsHeader->iNumLocators; i++ )
		{
			ATerrainLocator* pLocator = &m_pLocatorsHeader->pLocators[ i ];

			if ( !TStringManager::String8Compare( m_pLocatorsHeader->ppNames[ pLocator->uiNameId ], a_pszLocatorName ) )
				return &m_pLocatorsHeader->pLocators[ i ];
		}
	}
	else
	// We have VIS data, so it's easier to find the locator using binary search
	{
		for ( TINT i = 0; i < m_pLocatorVISHeader->iNumSections; i++ )
		{
			ATerrainLocatorVISSection* pSection = &m_pLocatorVISHeader->pSections[ i ];

			// Do binary search
			TUINT uiBase    = pSection->uiFirstLocator;
			TUINT uiEnd     = uiBase + pSection->uiNumLocators;
			TUINT uiCurrent = ( ( uiEnd - uiBase ) / 2 ) + uiBase;

			while ( uiBase <= uiCurrent && uiCurrent < uiEnd )
			{
				ATerrainLocator* pLocator = &m_pLocatorsHeader->pLocators[ uiCurrent ];
				TINT             iCmpRes  = TStringManager::String8Compare( m_pLocatorsHeader->ppNames[ pLocator->uiNameId ], a_pszLocatorName );

				if ( iCmpRes == 0 ) return pLocator;

				if ( iCmpRes < 1 )
				// Check the right side of array
				{
					uiBase    = uiCurrent + 1;
					uiCurrent = ( ( uiEnd - uiBase ) / 2 ) + uiBase;
				}
				else
				// Check the left side of array
				{
					uiEnd     = uiCurrent;
					uiCurrent = ( ( uiCurrent - uiBase ) / 2 ) + uiBase;
				}
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005ecad0
void ATerrainLocatorManager::LoadFromTRB( TTRB* a_pTRB )
{
	m_pTRB              = a_pTRB;
	m_pLocatorsHeader   = a_pTRB->CastSymbol<ATerrainLocatorTRBHeader>( "Locators" );
	m_pLocatorVISHeader = a_pTRB->CastSymbol<ATerrainLocatorVISTRBHeader>( "LocatorVIS" );

	TASSERT( m_pLocatorsHeader && m_pLocatorVISHeader );
	TTODO( "Create trees, regrowths, instances and other things from locators" );
}

// $Barnyard: FUNCTION 005ec9f0
void ATerrainLocator::GetMatrix( Toshi::TMatrix44& a_rOutMatrix )
{
	a_rOutMatrix.SetFromQuaternion( quatRotation );
	a_rOutMatrix.AsBasisVector3( BASISVECTOR_TRANSLATION ) = vecPosition;
}
