#include "pch.h"
#include "AFSMRequirement.h"
#include "ADrive.h"
#include "AUnit.h"
#include "AUnitBehaviour.h"
#include "FSM/AFSM.h"
#include "FSM/AFSMInstance.h"
#include "FSM/AFSMState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005aa5f0, CreateObject 005aa830
// NOTE: GetClass 005aa5f0, CreateObject 005aa830, CreateObjectInPlace 005aa8b0
TDEFINE_CLASS( AFSMRequirement );

TPSTRING8_DECLARE( drive );
TPSTRING8_DECLARE( fsm );
TPSTRING8_DECLARE( fsmstate );

// $Barnyard: FUNCTION 005aa780
// $Barnyard: FUNCTION 005aa920
AFSMRequirement::~AFSMRequirement()
{
}

// $Barnyard: FUNCTION 005aa600
TBOOL AFSMRequirement::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strDrive, TPS8( drive ) );
	a_pProperties->GetOptionalPropertyValue( m_strFSM, TPS8( fsm ) );
	a_pProperties->GetOptionalPropertyValue( m_strState, TPS8( fsmstate ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005aa650
void AFSMRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		TBOOL           bAllowed   = TFALSE;
		AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, a_pParticipants->Get( i )->FindComponent( &TGetClass( AUnitBehaviour ) ) );

		if ( pBehaviour )
		{
			bAllowed = TTRUE;

			if ( !m_strDrive.IsEmpty() )
			{
				ADrive* pDrive = pBehaviour->GetCurrentDrive();

				if ( !pDrive || pDrive->GetName() != m_strDrive )
					bAllowed = TFALSE;
			}

			if ( !m_strFSM.IsEmpty() )
			{
				AFSM* pMachine = pBehaviour->GetFSMInstance()->GetCurrentMachine();
				bAllowed       = bAllowed && pMachine && pMachine->GetName() == m_strFSM;
			}

			if ( !m_strState.IsEmpty() )
			{
				AFSMState* pState = pBehaviour->GetFSMInstance()->GetCurrentState();
				bAllowed          = bAllowed && pState && pState->GetName() == m_strState;
			}
		}

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}
}
