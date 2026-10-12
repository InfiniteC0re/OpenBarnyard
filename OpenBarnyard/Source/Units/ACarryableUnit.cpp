#include "pch.h"
#include "ACarryableUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00416a40, CreateObject 00416b60, CreateObjectInPlace 00416bc0 (the constructor is inlined in both)
TDEFINE_CLASS( ACarryableUnit, 1, 1 );

// $Barnyard: FUNCTION 00416a50
ACarryableUnit::ACarryableUnit()
    : m_oHelper( this )
{
}

// $Barnyard: FUNCTION 00416c00
// $Barnyard: FUNCTION 00416ac0
ACarryableUnit::~ACarryableUnit()
{
}

// $Barnyard: FUNCTION 00416ab0
ACarryableUnitHelper* ACarryableUnit::GetCarryableHelper()
{
	return &m_oHelper;
}

// $Barnyard: FUNCTION 004d7c40
TBOOL ACarryableUnit::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	m_eFlags |= FLAGS_AI_ENABLED;
	m_eFlags |= FLAGS_THINK_ENABLED;

	return TTRUE;
}

// NOTE: same code as ACharacterUnit::OnDestroy (00507930), folded in retail
void ACarryableUnit::OnDestroy()
{
	AAnimatedUnit::OnDestroy();
}

// NOTE: same code as AUnitPlayer::UpdateAI (00414890), folded in retail
void ACarryableUnit::UpdateAI( TFLOAT a_fDeltaTime )
{
	AVisibleUnit::UpdateAI( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00416b20
TBOOL ACarryableUnit::Think( TFLOAT a_fDeltaTime )
{
	TBOOL bResult = AVisibleUnit::Think( a_fDeltaTime );
	m_oHelper.Update( a_fDeltaTime );

	return bResult;
}
