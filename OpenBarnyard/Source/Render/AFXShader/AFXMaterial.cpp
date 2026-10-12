#include "pch.h"
#include "AFXMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005f97e0
TDEFINE_CLASS_NORUNTIME( AFXMaterial );

// $Barnyard: FUNCTION 005f9800
AFXMaterial::AFXMaterial()
    : m_eBlendMode( BLENDMODE_DEFAULT ), m_bUnk1( TFALSE ), m_bUnk2( TTRUE )
{
	m_aTextures[ 0 ] = TNULL;
	m_aTextures[ 1 ] = TNULL;
	m_aTextures[ 2 ] = TNULL;
	m_aTextures[ 3 ] = TNULL;
}

// $Barnyard: FUNCTION 005f9830
// $Barnyard: FUNCTION 005f9a10
AFXMaterial::~AFXMaterial()
{
}

// NOTE: Folded with AWorldMaterial::Create (005f8c50)
TBOOL AFXMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

// NOTE: Folded with AWorldMaterial::SetBlendMode (005f79a0)
void AFXMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
	m_Flags &= ~FLAGS_BLENDING;

	if ( a_eBlendMode != BLENDMODE_DEFAULT )
	{
		m_Flags |= FLAGS_BLENDING;
	}
}
