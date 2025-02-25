#include "pch.h"
#include "AGrassMaterial.h"

#include <Render/TOrderTable.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AGrassMaterial );

// $Barnyard: FUNCTION 005f7970
AGrassMaterial::AGrassMaterial()
    : m_eBlendMode( 0 )
{
	m_aTextures[ 0 ] = TNULL;
	m_aTextures[ 1 ] = TNULL;
	m_aTextures[ 2 ] = TNULL;
	m_aTextures[ 3 ] = TNULL;
}

// $Barnyard: FUNCTION 005f7ab0
// $Barnyard: FUNCTION 005f7a90
AGrassMaterial::~AGrassMaterial()
{
	TRegMaterial* pRegMaterial = GetRegMaterial();

	if ( pRegMaterial != TNULL )
	{
		TOrderTable::DeregisterMaterial( pRegMaterial );
	}
}

TBOOL AGrassMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

void AGrassMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
	m_Flags &= ~FLAGS_BLENDING;

	if ( a_eBlendMode != 0 )
		m_Flags |= FLAGS_BLENDING;
}
