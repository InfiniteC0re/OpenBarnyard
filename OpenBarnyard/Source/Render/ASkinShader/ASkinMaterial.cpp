#include "pch.h"
#include "ASkinMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASkinMaterial );

// $Barnyard: FUNCTION 005f14d0
ASkinMaterial::ASkinMaterial()
    : m_eBlendMode( 0 ), m_pTexture( TNULL ), m_bFlag( TTRUE )
{
	m_apLightingTextures[ LT_0 ] = TNULL;
	m_apLightingTextures[ LT_1 ] = TNULL;
	m_apLightingTextures[ LT_2 ] = TNULL;
	m_apLightingTextures[ LT_3 ] = TNULL;
}

// $Barnyard: FUNCTION 005f1510
ASkinMaterial::~ASkinMaterial()
{
}

TBOOL ASkinMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

// $Barnyard: FUNCTION 005f1520
void ASkinMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
	m_Flags &= ~FLAGS_BLENDING;

	if ( a_eBlendMode != 0 )
	{
		m_Flags |= FLAGS_BLENDING;
	}
}
