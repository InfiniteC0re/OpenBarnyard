#include "pch.h"
#include "AWorldMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME(AWorldMaterial);

AWorldMaterial::AWorldMaterial() :
	m_eBlendMode(0),
	m_fUVAnimX(0.0f),
	m_fUVAnimY(0.0f),
	m_fUVAnimSpeedX(0.0f),
	m_fUVAnimSpeedY(0.0f)
{
	m_aTextures[0] = TNULL;
	m_aTextures[1] = TNULL;
	m_aTextures[2] = TNULL;
	m_aTextures[3] = TNULL;
}

AWorldMaterial::~AWorldMaterial()
{

}

TBOOL AWorldMaterial::Create(BLENDMODE a_eBlendMode)
{
	SetBlendMode(a_eBlendMode);
	return TMaterial::Create();
}

void AWorldMaterial::SetBlendMode(BLENDMODE a_eBlendMode)
{
	m_eBlendMode = a_eBlendMode;
	m_Flags &= ~Flags_Unk2;

	if (a_eBlendMode != 0)
	{
		m_Flags |= Flags_Unk2;
	}
}
