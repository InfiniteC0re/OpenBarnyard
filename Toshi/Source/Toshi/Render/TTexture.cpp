#include "ToshiPCH.h"
#include "TTexture.h"

namespace Toshi {

	TTexture::TTexture()
	{
		m_Unk1 = 0;
		m_Unk2 = 0;
		m_Unk3 = 0;
	}

	TTexture::~TTexture()
	{

	}

	TBOOL TTexture::Lock(LOCKSTATE& a_rLockState)
	{
		return TFALSE;
	}

	void TTexture::Unlock()
	{

	}

	TBOOL TTexture::CreateEx(void*, TUINT, TUINT, TUINT, TUINT, TTEXTURERESOURCEFORMAT, TUINT)
	{
		return TResource::Create();
	}

}