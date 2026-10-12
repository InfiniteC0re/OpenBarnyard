#include "pch.h"
#include "ARemoveUnitsActivity.h"
#include "AInteractionComponent.h"
#include "AInteractionSequence.h"
#include "AKinematicsObstructionActivityMessage.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bc7d0, CreateObject 005bc7f0
// NOTE: Create (+0x0c) is 005b9fd0, a thunk to AInteractionActivity::Create shared with other activities
// NOTE: GetClass 005bc7d0, CreateObject 005bc7f0, CreateObjectInPlace 005bc850
TDEFINE_CLASS( ARemoveUnitsActivity );

ARemoveUnitsActivity::ARemoveUnitsActivity()
{
}

// $Barnyard: FUNCTION 005bc7e0
// $Barnyard: FUNCTION 005bc8a0
ARemoveUnitsActivity::~ARemoveUnitsActivity()
{
}

// $Barnyard: FUNCTION 005bc940
AInteractionActivity::RESULT ARemoveUnitsActivity::Update( AInteractionSequence* a_pSequence )
{
	TINT iNumUnits = a_pSequence->GetComponents().Size();

	auto it = a_pSequence->GetCategoryBegin( m_eCategory );

	if ( it != a_pSequence->GetCategoryEnd( m_eCategory ) )
		it->LeaveSequence( TFALSE );

	if ( iNumUnits < 3 )
	{
		AKinematicsObstructionActivityMessage* pMessage = a_pSequence->FindActivityMessage<AKinematicsObstructionActivityMessage>( TPString8() );

		// The interaction is over, nothing has to steer around it anymore
		if ( pMessage )
			pMessage->ReleaseObstruction();
	}

	return RESULT_DONE;
}
