#include "pch.h"
#include "ASysMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASysMaterial );

ASysMaterial::ASysMaterial()
{
	m_eBlendMode = BLENDMODE_DEFAULT;
	m_Vec3		 = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pTexture	 = TNULL;
	m_Unk1		 = 0;
	m_Unk2		 = 0;
	m_Unk3		 = 0;
}

TBOOL ASysMaterial::Create()
{
	return TMaterial::Create();
}

TBOOL ASysMaterial::Create( BLENDMODE a_eBlendMode )
{
	return TMaterial::Create();
}

void ASysMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
}
