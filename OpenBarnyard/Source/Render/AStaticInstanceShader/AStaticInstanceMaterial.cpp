#include "pch.h"
#include "AStaticInstanceMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AStaticInstanceMaterial );

// $Barnyard: FUNCTION 005f8c20
AStaticInstanceMaterial::AStaticInstanceMaterial()
    : m_eBlendMode( BLENDMODE_DEFAULT )
{
	m_aTextures[ 0 ] = TNULL;
	m_aTextures[ 1 ] = TNULL;
	m_aTextures[ 2 ] = TNULL;
	m_aTextures[ 3 ] = TNULL;
}

// $Barnyard: FUNCTION 005f8c70
// $Barnyard: FUNCTION 005f8e20
AStaticInstanceMaterial::~AStaticInstanceMaterial()
{
}

// NOTE: folded with AWorldMaterial::Create (005f8c50)
TBOOL AStaticInstanceMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

// NOTE: folded with AWorldMaterial::SetBlendMode (005f79a0)
void AStaticInstanceMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
	m_Flags &= ~FLAGS_BLENDING;

	if ( a_eBlendMode != BLENDMODE_DEFAULT )
		m_Flags |= FLAGS_BLENDING;
}
