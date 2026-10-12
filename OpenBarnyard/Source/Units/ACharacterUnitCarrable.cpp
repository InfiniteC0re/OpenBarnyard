#include "pch.h"
#include "ACharacterUnitCarrable.h"
#include "AAnimatedUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00417b30, CreateObject 00417ca0, CreateObjectInPlace 00417d00 (the constructor is inlined in both)
TDEFINE_CLASS_INIT( ACharacterUnitCarrable, 1, 1 );

AFunctionTable ACharacterUnitCarrable::ms_oFunctionTable( &TGetClass( ACharacterUnitCarrable ), &ACharacterUnit::ms_oFunctionTable );

// $Barnyard: FUNCTION 00417b70
void ACharacterUnitCarrable::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );
}

// $Barnyard: FUNCTION 00417b80
void ACharacterUnitCarrable::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00417b90
ACharacterUnitCarrable::ACharacterUnitCarrable()
    : m_oHelper( this )
{
}

// $Barnyard: FUNCTION 00417c30
// $Barnyard: FUNCTION 00417c50
ACharacterUnitCarrable::~ACharacterUnitCarrable()
{
}

// $Barnyard: FUNCTION 00417bf0
AFunctionTable* ACharacterUnitCarrable::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0055d090
ACarryableUnitHelper* ACharacterUnitCarrable::GetCarryableHelper()
{
	return &m_oHelper;
}

// $Barnyard: FUNCTION 00417c10
// $Barnyard: FUNCTION 00417b20
ACharacterUnitCarrable::ACarryableCharacterHelper::~ACarryableCharacterHelper()
{
}

// $Barnyard: FUNCTION 00417b40
TBOOL ACharacterUnitCarrable::ACarryableCharacterHelper::PutAway()
{
	AUnit* pUnit = m_pUnit;

	TSTATICCAST( AAnimatedUnit, m_pHolder )->DetachUnit( pUnit, TTRUE, TTRUE );
	pUnit->GetSteer()->Unk_0x98( 0 );

	return TTRUE;
}
