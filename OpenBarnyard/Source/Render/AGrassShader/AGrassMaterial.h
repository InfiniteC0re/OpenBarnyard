#pragma once
#include "Render/AWorldShader/AWorldMaterial.h"

#include <Toshi/Toshi.h>
#include <Render/TMaterial.h>

class AGrassMaterial
    : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( AGrassMaterial, Toshi::TMaterial );

	using BLENDMODE = TINT;

public:
	AGrassMaterial();
	~AGrassMaterial();

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
