#pragma once
#include <Render/TMaterial.h>
#include <Render/TTexture.h>

class SDK_T2GUIMaterial : public Toshi::TMaterial
{
public:
	friend class SDK_T2GUIRenderer;
	friend class AGUI2RendererDX8;

public:
	SDK_T2GUIMaterial();

	void SetBlendState( TUINT a_eBlendState )
	{
		m_eBlendState = a_eBlendState;
	}

	void SetTextureAddress( TUINT a_eTextureAddress )
	{
		m_eTextureAddress = a_eTextureAddress;
	}

	TBOOL Create_Hack() { return Create(); }

public:
	Toshi::TTexture* m_pTextureResource;
	TUINT            m_eBlendState;
	TUINT            m_eTextureAddress;
	TUINT            m_iMipMapLODBias;
};
