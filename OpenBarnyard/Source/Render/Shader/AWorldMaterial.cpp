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
	m_iUnk1(0),
	m_fUnk2(0.0),
	m_iUnk3(0),
	m_fUnk4(0.0)
{
	m_aTextures[0] = TNULL;
	m_aTextures[1] = TNULL;
	m_aTextures[2] = TNULL;
	m_aTextures[3] = TNULL;
}

AWorldMaterial::~AWorldMaterial()
{

}

void AWorldMaterial::Create(BLENDMODE a_eBlendMode)
{
	Create(a_eBlendMode);
	TMaterial::Create();
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
