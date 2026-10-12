#include "pch.h"
#include "ACharacterAnimControllerType.h"
#include "ACharacterAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00588930, CreateObject 00588c40, CreateObjectInPlace 00588cb0
TDEFINE_CLASS( ACharacterAnimControllerType );

// $Barnyard: FUNCTION 00588950
ACharacterAnimControllerType::ACharacterAnimControllerType()
{
}

// $Barnyard: FUNCTION 00588d00
// $Barnyard: FUNCTION 00588970
ACharacterAnimControllerType::~ACharacterAnimControllerType()
{
}

TPSTRING8_DECLARE( UserRender );

// $Barnyard: FUNCTION 00588980
// $Barnyard: FUNCTION 00585550
TBOOL ACharacterAnimControllerType::Create( const PBProperties* a_pProperties )
{
	m_bUserRender = TFALSE;
	a_pProperties->GetOptionalPropertyValue( m_bUserRender, TPS8( UserRender ) );

	return TTRUE;
}

// NOTE: folded with ACharacterAnimController::GetClass
// $Barnyard: FUNCTION 00588940
TClass* ACharacterAnimControllerType::GetControllerClass()
{
	return &TGetClass( ACharacterAnimController );
}
