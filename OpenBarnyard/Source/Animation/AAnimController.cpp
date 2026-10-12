#include "pch.h"
#include "AAnimController.h"
#include "AAnimatableObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AAnimController );

// $Barnyard: FUNCTION 005803f0
AAnimController::AAnimController()
{
	m_pOwner            = TNULL;
	m_pAnimatableObject = TNULL;
	m_pControllerType   = TNULL;
	m_bDisabled         = TFALSE;
}

// $Barnyard: FUNCTION 00580420
// $Barnyard: FUNCTION 00580370
AAnimController::~AAnimController()
{
}

TBOOL AAnimController::Create( void* a_pOwner, AAnimatableObject* a_pAnimatableObject, AAnimControllerType* a_pControllerType )
{
	m_pOwner            = a_pOwner;
	m_pAnimatableObject = a_pAnimatableObject;
	m_pControllerType   = a_pControllerType;

	return OnCreate();
}

// $Barnyard: FUNCTION 005803a0
AToshiAnimationInterface& AAnimController::GetAnimInterface()
{
	return m_pAnimatableObject->GetToshiAnimInterface();
}
