#include "pch.h"
#include "AWaitAniEffect.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006336d0, CreateObject 00633720, CreateObjectInPlace 00633790 (the constructor is inlined in both)
TDEFINE_CLASS( AWaitAniEffect );

AWaitAniEffect::AWaitAniEffect()
    : m_fTime( 1.0f )
{
}

// $Barnyard: FUNCTION 006337f0
// $Barnyard: FUNCTION 006336e0
AWaitAniEffect::~AWaitAniEffect()
{
}

// $Barnyard: FUNCTION 006336f0
TBOOL AWaitAniEffect::Update( TFLOAT a_fDeltaTime )
{
	m_fTime -= a_fDeltaTime;

	if ( m_fTime > 0.0f )
		return TTRUE;

	Finish();
	return TFALSE;
}
