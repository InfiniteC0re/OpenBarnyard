#include "ToshiPCH.h"
#include "TSysMaterialHAL.h"
#include "TSysMaterial.h"

namespace Toshi {

	TBOOL TSysMaterial::Create(BlendMode eBlendMode)
	{
		return TMaterial::Create();
	}

	void TSysMaterial::SetBlendMode(BlendMode eBlendMode)
	{
		m_eBlendMode = eBlendMode;
	}

}
