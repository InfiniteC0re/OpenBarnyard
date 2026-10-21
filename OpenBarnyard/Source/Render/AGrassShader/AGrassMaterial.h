#pragma once
#include "Render/AWorldShader/AWorldMaterial.h"

#include <Toshi/Toshi.h>
#include <Render/TMaterial.h>

class MODAPI AGrassMaterial
    : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( AGrassMaterial, Toshi::TMaterial );

	using BLENDMODE = TINT;
	enum BLENDMODE_ : BLENDMODE
	{
		BLENDMODE_DEFAULT  = 0,
		BLENDMODE_ALPHA    = 1, // alpha blended
		BLENDMODE_MODULATE = 2, // the colour multiplies the destination
		BLENDMODE_ADDITIVE = 3, // the colour (times alpha) is added to the destination
	};

public:
	AGrassMaterial();
	~AGrassMaterial();

	static void InitialiseStatic();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode );
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode );

	void SetTexture( TUINT a_uiStage, Toshi::TTexture* a_pTexture )
	{
		TASSERT( a_uiStage < AWorldMaterial::MAX_TEXTURES );
		m_aTextures[ a_uiStage ] = a_pTexture;
	}

protected:
	Toshi::TTexture* m_aTextures[ AWorldMaterial::MAX_TEXTURES ];
	BLENDMODE        m_eBlendMode;
};
