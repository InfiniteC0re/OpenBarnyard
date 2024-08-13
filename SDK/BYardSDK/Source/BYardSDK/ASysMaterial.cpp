#include "pch.h"
#include "ASysMaterial.h"

TOSHI_NAMESPACE_USING

ASysMaterial::ASysMaterial()
{
	TASSERT( TFALSE );
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
