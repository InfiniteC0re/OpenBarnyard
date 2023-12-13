#include "pch.h"
#include "ASysMaterial.h"

TOSHI_NAMESPACE_USING

ASysMaterial::ASysMaterial()
{
	m_eBlendMode = BLENDMODE_DEFAULT;
	m_Vec3 = { 0.0, 0.0, 0.0, 0.0f };
	m_Vec1 = { 0.0, 0.0, 0.0, 0.0f };
}

TBOOL ASysMaterial::Create()
{
	return TMaterial::Create();
}

TBOOL ASysMaterial::Create(BLENDMODE a_eBlendMode)
{
	return TMaterial::Create();
}

void ASysMaterial::SetBlendMode(BLENDMODE a_eBlendMode)
{
	m_eBlendMode = a_eBlendMode;
}
