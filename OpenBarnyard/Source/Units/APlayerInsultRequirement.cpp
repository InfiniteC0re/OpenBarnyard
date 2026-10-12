#include "pch.h"
#include "APlayerInsultRequirement.h"
#include "AInteractionPlayerInsultEvent.h"
#include "AUnitGameEventComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005adc50, CreateObject 005adc80
// NOTE: GetClass 005adc50, CreateObject 005adc80, CreateObjectInPlace 005add00
TDEFINE_CLASS( APlayerInsultRequirement );

// $Barnyard: FUNCTION 005adc60
// $Barnyard: FUNCTION 005add70
APlayerInsultRequirement::~APlayerInsultRequirement()
{
}

// $Barnyard: FUNCTION 005adc70
TBOOL APlayerInsultRequirement::Create( const PBProperties* a_pProperties )
{
	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005add90
void APlayerInsultRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TFLOAT fWeight   = 0.0f;
	TINT   iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		TBOOL bInsulted = TFALSE;

		AUnitGameEventComponent* pEvents = TSTATICCAST( AUnitGameEventComponent, a_pParticipants->Get( i )->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );

		if ( pEvents )
		{
			AInteractionPlayerInsultEvent* pInsult = TSTATICCAST( AInteractionPlayerInsultEvent, pEvents->FindEventOfExactClass( &TGetClass( AInteractionPlayerInsultEvent ) ) );

			if ( pInsult && pInsult->IsRecent() )
			{
				bInsulted = TTRUE;
				fWeight   = 1.0f;
			}
		}

		SetUnitAllowed( m_eCategory, i, bInsulted );
	}

	if ( m_bInvert == TTRUE )
		fWeight = 1.0f - fWeight;

	m_oRoles.fWeight = fWeight;
}
