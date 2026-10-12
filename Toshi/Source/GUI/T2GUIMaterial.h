#pragma once
#include "Render/TMaterial.h"
#include "Render/TTexture.h"

TOSHI_NAMESPACE_START

class T2GUIMaterial : public TMaterial
{
public:
	friend class T2GUIRenderer;

	using BLENDSTATE = TUINT;
	enum BLENDSTATE_ : BLENDSTATE
	{
		BLENDSTATE_OPAQUE       = 0,
		BLENDSTATE_ALPHA        = 1,
		BLENDSTATE_ADDITIVE     = 2,
		BLENDSTATE_SUBTRACTIVE  = 3,
		BLENDSTATE_MASK         = 4, // only writes the depth of the alpha tested pixels (the shape of the mask)
		BLENDSTATE_MASKED       = 5, // alpha blended, drawn only where the mask is
		BLENDSTATE_ALPHA_ZWRITE = 6, // alpha blended, writes the depth
	};

	using TEXTUREADDRESS = TUINT;
	enum TEXTUREADDRESS_ : TEXTUREADDRESS
	{
		TEXTUREADDRESS_WRAP   = 0,
		TEXTUREADDRESS_CLAMP  = 1,
		TEXTUREADDRESS_MIRROR = 2,
	};

public:
	// $Barnyard: FUNCTION 0064e450
	T2GUIMaterial()
	{
		m_iNumTex          = 1;
		m_pTextureResource = TNULL;
		m_eBlendState      = BLENDSTATE_ALPHA;
		m_eTextureAddress  = TEXTUREADDRESS_CLAMP;
		m_iMipMapLODBias   = 0;
	}

	// $Barnyard: FUNCTION 0064e480
	// $Barnyard: FUNCTION 0064e5a0
	~T2GUIMaterial() {}

	void SetBlendState( BLENDSTATE a_eBlendState )
	{
		m_eBlendState = a_eBlendState;
	}

	BLENDSTATE GetBlendState() const { return m_eBlendState; }

	void SetTextureAddress( TEXTUREADDRESS a_eTextureAddress )
	{
		m_eTextureAddress = a_eTextureAddress;
	}

	// NOTE: the bias is a float, it's stored the way D3DTSS_MIPMAPLODBIAS takes it
	void SetMipMapLODBias( TFLOAT a_fBias )
	{
		m_iMipMapLODBias = *TREINTERPRETCAST( TUINT*, &a_fBias );
	}

	TTexture* GetTextureResource() const { return m_pTextureResource; }

#ifdef BARNYARD_LATEST
	void SetTextureResource( TTexture* a_pTexture )
	{
		m_pTextureResource = a_pTexture;
	}
	TEXTUREADDRESS GetTextureAddress() const { return m_eTextureAddress; }
#endif // BARNYARD_LATEST

protected:
	TTexture*      m_pTextureResource;
	BLENDSTATE     m_eBlendState;
	TEXTUREADDRESS m_eTextureAddress;
	TUINT          m_iMipMapLODBias;
};

TOSHI_NAMESPACE_END
