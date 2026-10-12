#include "pch.h"
#include "AMovableUnit.h"
#include "Render/AShadowManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00418cf0, CreateObject 00418d30, CreateObjectInPlace 00418db0
TDEFINE_CLASS( AMovableUnit, 1, 1 );

AMovableUnit::AMovableUnit()
    : m_iUnk98( 0 )
{
	// NOTE: inlined into CreateObject/CreateObjectInPlace in retail
}

// $Barnyard: FUNCTION 00418e30
// $Barnyard: FUNCTION 00418d00
AMovableUnit::~AMovableUnit()
{
}

// $Barnyard: FUNCTION 00418e50
TBOOL AMovableUnit::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	m_eFlags |= FLAGS_THINK_ENABLED;
	AShadowManager::GetSingleton()->AddUnitShadow( this, 1.25f, -1, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );

	return TTRUE;
}

// NOTE: same code as AUnitPlayer::UpdateAI (00414890), folded in retail
void AMovableUnit::UpdateAI( TFLOAT a_fDeltaTime )
{
	AVisibleUnit::UpdateAI( a_fDeltaTime );
}

// NOTE: a thunk to AVisibleUnit::Think (004d7ae0) shared by several classes in retail
TBOOL AMovableUnit::Think( TFLOAT a_fDeltaTime )
{
	return AVisibleUnit::Think( a_fDeltaTime );
}
