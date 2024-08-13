#include "pch.h"
#include "ASkinMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASkinMaterial );

ASkinMaterial::ASkinMaterial() :
	m_eBlendMode( 0 ),
	m_pTexture( TNULL ),
	m_bFlag( TTRUE )
{
	m_apLightingTextures[ LT_0 ] = TNULL;
	m_apLightingTextures[ LT_1 ] = TNULL;
	m_apLightingTextures[ LT_2 ] = TNULL;
	m_apLightingTextures[ LT_3 ] = TNULL;
}

ASkinMaterial::~ASkinMaterial()
{
}

TBOOL ASkinMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

void ASkinMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
	m_Flags &= ~FLAGS_HAS_BLENDMODE;

	if ( a_eBlendMode != 0 )
	{
		m_Flags |= FLAGS_HAS_BLENDMODE;
	}
}
