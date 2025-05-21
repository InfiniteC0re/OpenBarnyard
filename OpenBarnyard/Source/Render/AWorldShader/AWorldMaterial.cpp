#include "pch.h"
#include "AWorldMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AWorldMaterial );

// $Barnyard: FUNCTION 005f5c70
AWorldMaterial::AWorldMaterial()
    : m_eBlendMode( 0 ), m_fUVAnimX( 0.0f ), m_fUVAnimY( 0.0f ), m_fUVAnimSpeedX( 0.0f ), m_fUVAnimSpeedY( 0.0f )
{
	m_aTextures[ 0 ] = TNULL;
	m_aTextures[ 1 ] = TNULL;
	m_aTextures[ 2 ] = TNULL;
	m_aTextures[ 3 ] = TNULL;
}

// $Barnyard: FUNCTION 005f5ca0
AWorldMaterial::~AWorldMaterial()
{
}

// $Barnyard: FUNCTION 005f8c50
TBOOL AWorldMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

// $Barnyard: FUNCTION 005f79a0
void AWorldMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
	m_Flags &= ~FLAGS_BLENDING;

	if ( a_eBlendMode != 0 )
	{
		m_Flags |= FLAGS_BLENDING;
	}
}
