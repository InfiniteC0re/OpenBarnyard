#pragma once
#include <Render/TMaterial.h>
#include <Render/TTexture.h>

class MODAPI AWorldMaterial : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( AWorldMaterial, Toshi::TMaterial );

	static constexpr TUINT MAX_TEXTURES = 4;

	using BLENDMODE = TINT;
	enum BLENDMODE_ : BLENDMODE
	{
		BLENDMODE_DEFAULT      = 0,
		BLENDMODE_ALPHA        = 1, // alpha blended
		BLENDMODE_MODULATE     = 2, // the colour multiplies the destination
		BLENDMODE_ADDITIVE     = 3, // the colour (times alpha) is added to the destination
		BLENDMODE_START_RENDER = 7, // rendered before the rest of the world (IsWorldStartRenderMaterial)
	};

public:
	AWorldMaterial();
	~AWorldMaterial();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode );
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode );

	AWorldMaterial::BLENDMODE GetBlendMode() const { return m_eBlendMode; }

	// Sets the scrolling speed of the UVs, resets the scrolling if both are (nearly) zero
	void SetUVAnimationSpeed( TFLOAT a_fSpeedX, TFLOAT a_fSpeedY );

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
	Toshi::TTexture* m_aTextures[ MAX_TEXTURES ];
	BLENDMODE        m_eBlendMode;
	TFLOAT           m_fUVAnimX;
	TFLOAT           m_fUVAnimY;
	TFLOAT           m_fUVAnimSpeedX;
	TFLOAT           m_fUVAnimSpeedY;
};
