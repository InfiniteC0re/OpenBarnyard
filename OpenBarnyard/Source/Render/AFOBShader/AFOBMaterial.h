#pragma once
#include <Render/TMaterial.h>
#include <Render/TTexture.h>

//-----------------------------------------------------------------------------
// NOTE: stripped from PC retail, only the TClass object is left (see AFOBShader.h).
// AModelLoader treats it like AWorldMaterial: Create( BLENDMODE ) at +0x1C and
// the first texture at 0x4C.
//-----------------------------------------------------------------------------
class AFOBMaterial : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( AFOBMaterial, Toshi::TMaterial );

	static constexpr TUINT MAX_TEXTURES = 4;

	using BLENDMODE = TINT;

public:
	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode )       = 0; // +0x1C
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode ) = 0; // +0x20

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
	TUINT8           PADDING[ 0x14 ];             // 0x5C
};
