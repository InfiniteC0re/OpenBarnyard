#pragma once
#include <Render/TMaterial.h>

class ASkinMaterial : public Toshi::TMaterial
{
	TDECLARE_CLASS( ASkinMaterial, Toshi::TMaterial );

public:
	using ELightingTexture = TUINT;
	enum ELightingTexture_ : ELightingTexture
	{
		LT_0,
		LT_1,
		LT_2,
		LT_3,
		LT_NUMOF,
	};

	using BLENDMODE = TUINT;

public:
	ASkinMaterial();
	~ASkinMaterial();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode );
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode );

	BLENDMODE GetBlendMode() const { return m_eBlendMode; }

	void SetTexture( Toshi::TTexture* a_pTexture )
	{
		m_pTexture = a_pTexture;
		SetTextureNum( 1 );
	}

protected:
	Toshi::TTexture* m_pTexture;
	Toshi::TTexture* m_apLightingTextures[ LT_NUMOF ];
	BLENDMODE        m_eBlendMode;
	TBOOL            m_bFlag;
};
