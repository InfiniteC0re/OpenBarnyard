#include "pch.h"
#include "AKinematicsObstructionActivity.h"
#include "AKinematicsObstructionActivityMessage.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b9fb0, CreateObject 005b9fe0, CreateObjectInPlace 005ba050
// NOTE: Create (+0x0c) is 005b9fd0, a thunk to AInteractionActivity::Create shared with other activities
TDEFINE_CLASS( AKinematicsObstructionActivity );

// NOTE: inlined into CreateObject, retail leaves m_iUnk08 uninitialised
AKinematicsObstructionActivity::AKinematicsObstructionActivity()
    : m_iUnk08( 0 )
    , m_pMessage( TNULL )
{
}

// $Barnyard: FUNCTION 005b9fc0
// $Barnyard: FUNCTION 005ba0c0
AKinematicsObstructionActivity::~AKinematicsObstructionActivity()
{
}

// $Barnyard: FUNCTION 005ba160
AInteractionActivity::RESULT AKinematicsObstructionActivity::OnStart( AInteractionSequence* a_pSequence )
{
	AKinematicsObstructionActivityMessage* pMessage = a_pSequence->GetActivityMessage<AKinematicsObstructionActivityMessage>( TPString8() );

	// Creates the obstruction around the units of the sequence
	pMessage->CreateObstruction();

	m_pMessage = pMessage;

	return RESULT_DONE;
}
