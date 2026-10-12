#include "pch.h"
#include "AUnitGolfProjectile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0041a150, CreateObject 0041a190, CreateObjectInPlace 0041a210 (the constructor is inlined in both)
TDEFINE_CLASS( AUnitGolfProjectile );

// $Barnyard: FUNCTION 0041a160
// $Barnyard: FUNCTION 0041a270
AUnitGolfProjectile::~AUnitGolfProjectile()
{
}

// $Barnyard: FUNCTION 0041a290
TBOOL AUnitGolfProjectile::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	m_eFlags |= FLAGS_THINK_ENABLED;
	return TTRUE;
}
