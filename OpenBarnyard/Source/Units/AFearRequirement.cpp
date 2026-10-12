#include "pch.h"
#include "AFearRequirement.h"
#include "AInteractionEventManager.h"
#include "AInteractionFearEvent.h"
#include "AUnit.h"
#include "AUnitFactory.h"
#include "AUnitManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005aa300, CreateObject 005aa320, CreateObjectInPlace 005aa3a0
TDEFINE_CLASS( AFearRequirement );

// $Barnyard: FUNCTION 005aa310
// $Barnyard: FUNCTION 005aa410
AFearRequirement::~AFearRequirement()
{
}

// $Barnyard: FUNCTION 005aa530
TBOOL AFearRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pUnitType = a_pProperties->GetProperty( "unittype" );

	if ( pUnitType )
		m_pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( pUnitType->GetTPString8() );

	const PBPropertyValue* pFear = a_pProperties->GetProperty( "fear" );

	if ( pFear )
		m_bFear = pFear->GetRaw() != TNULL;

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005aa430
void AFearRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	AInteractionEventManager*          pEventManager = AInteractionEventManager::GetSingleton();
	AInteractionEventManager::Iterator oIterator;

	TINT   iNumUnits = a_pParticipants->iNumUnits;
	TFLOAT fWeight   = 0.0f;

	if ( !m_bFear )
	{
		// Everyone who doesn't know about the fear is fine
		fWeight = 1.0f;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			a_pParticipants->Get( i );
			SetUnitAllowed( m_eCategory, i, TTRUE );
		}
	}

	for ( AInteractionEvent* pEvent = pEventManager->FindFirstEvent( &TGetClass( AInteractionFearEvent ), oIterator );
	      pEvent != TNULL;
	      pEvent = pEventManager->FindNextEvent( &TGetClass( AInteractionFearEvent ), oIterator ) )
	{
		if ( TSTATICCAST( AInteractionFearEvent, pEvent )->GetUnitType() != m_pUnitType )
			continue;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			if ( pEvent->IsKnownBy( a_pParticipants->Get( i ) ) )
			{
				SetUnitAllowed( m_eCategory, i, m_bFear );

				if ( m_bFear )
					fWeight = 1.0f;
			}
		}
	}

	m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f - fWeight : fWeight;
}
