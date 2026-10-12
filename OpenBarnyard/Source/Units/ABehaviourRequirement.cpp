#include "pch.h"
#include "ABehaviourRequirement.h"
#include "AUnit.h"
#include "AUnitBehaviour.h"
#include "FSM/AFSM.h"
#include "FSM/AFSMInstance.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a8340, CreateObject 005a8530
// NOTE: GetClass 005a8340, CreateObject 005a8530, CreateObjectInPlace 005a85b0
TDEFINE_CLASS( ABehaviourRequirement );

// $Barnyard: FUNCTION 005a83f0
// $Barnyard: FUNCTION 005a8630
ABehaviourRequirement::~ABehaviourRequirement()
{
}

// $Barnyard: FUNCTION 005a8460
TBOOL ABehaviourRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pScript = a_pProperties->GetProperty( "script" );

	if ( pScript )
		m_strScript = pScript->GetTPString8();

	const PBPropertyValue* pRunning = a_pProperties->GetProperty( "running" );

	if ( pRunning )
		m_bRunning = pRunning->GetRaw() != TNULL;

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a8350
void ABehaviourRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TFLOAT fWeight   = 0.0f;
	TINT   iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		TBOOL           bAllowed   = TFALSE;
		AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, a_pParticipants->Get( i )->FindComponent( &TGetClass( AUnitBehaviour ) ) );

		if ( pBehaviour )
		{
			AFSM* pMachine = pBehaviour->GetFSMInstance()->GetCurrentMachine();

			if ( pMachine && ( pMachine->GetName() == m_strScript ) == m_bRunning )
			{
				bAllowed = TTRUE;
				fWeight  = 1.0f;
			}
		}

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}

	if ( m_bInvert == TTRUE )
		fWeight = 1.0f - fWeight;

	m_oRoles.fWeight = fWeight;
}
