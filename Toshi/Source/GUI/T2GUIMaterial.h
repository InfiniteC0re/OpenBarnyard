#pragma once
#include "Render/TMaterial.h"
#include "Render/TTexture.h"

TOSHI_NAMESPACE_START

class T2GUIMaterial : public TMaterial
{
public:
	friend class T2GUIRenderer;

public:
	// $Barnyard: FUNCTION 0064e450
	T2GUIMaterial()
	{
		m_iNumTex          = 1;
		m_pTextureResource = TNULL;
		m_eBlendState      = 1;
		m_eTextureAddress  = 1;
		m_iMipMapLODBias   = 0;
	}

	void SetBlendState( TUINT a_eBlendState )
	{
		m_eBlendState = a_eBlendState;
	}

	void SetTextureAddress( TUINT a_eTextureAddress )
	{
		m_eTextureAddress = a_eTextureAddress;
	}

protected:
	TTexture* m_pTextureResource;
	TUINT     m_eBlendState;
	TUINT     m_eTextureAddress;
	TUINT     m_iMipMapLODBias;
};

TOSHI_NAMESPACE_END
