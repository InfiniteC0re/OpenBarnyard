#pragma once
#include <Render/TMaterial.h>
#include <Render/TTexture.h>

class AFXMaterial : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( AFXMaterial, Toshi::TMaterial );

	static constexpr TUINT MAX_TEXTURES = 4;

	using BLENDMODE = TINT;
	enum BLENDMODE_ : BLENDMODE
	{
		BLENDMODE_DEFAULT  = 0,
		BLENDMODE_ALPHA    = 1, // alpha blended
		BLENDMODE_MODULATE = 2, // the colour multiplies the destination
		BLENDMODE_ADDITIVE = 3, // the colour (times alpha) is added to the destination
	};

public:
	AFXMaterial();
	~AFXMaterial();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode );       // +0x1C
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode ); // +0x20

	BLENDMODE GetBlendMode() const { return m_eBlendMode; }

	void SetTexture( TUINT a_uiStage, Toshi::TTexture* a_pTexture )
	{
		TASSERT( a_uiStage < MAX_TEXTURES );
		m_aTextures[ a_uiStage ] = a_pTexture;
	}

	Toshi::TTexture* GetTexture( TUINT a_uiStage ) const
	{
		TASSERT( a_uiStage < MAX_TEXTURES );
		return m_aTextures[ a_uiStage ];
	}

protected:
	Toshi::TTexture* m_aTextures[ MAX_TEXTURES ]; // 0x4C
	BLENDMODE        m_eBlendMode;                // 0x5C
	TBOOL            m_bUnk1;                     // 0x60
	TBOOL            m_bUnk2;                     // 0x61
};
