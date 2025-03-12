#include "pch.h"
#include "AAnimController.h"

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
	m_pUnk1             = TNULL;
	m_pAnimatableObject = TNULL;
	m_pControllerType   = TNULL;
	m_bUnk4             = TFALSE;
}

// $Barnyard: FUNCTION 00580420
// $Barnyard: FUNCTION 00580370
AAnimController::~AAnimController()
{
}

TBOOL AAnimController::OnCreate()
{
	return TTRUE;
}

void AAnimController::Unknown2()
{
}

TBOOL AAnimController::Unknown3( void* )
{
	return TTRUE;
}

TBOOL AAnimController::Unknown4( void* )
{
	return TTRUE;
}

TBOOL AAnimController::Create( void* a_pUnk, AAnimatableObject* a_pAnimatableObject, AAnimControllerType* a_pControllerType )
{
	m_pUnk1             = a_pUnk;
	m_pAnimatableObject = a_pAnimatableObject;
	m_pControllerType   = a_pControllerType;

	return OnCreate();
}
