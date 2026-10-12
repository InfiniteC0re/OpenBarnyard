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
	enum BLENDMODE_ : BLENDMODE
	{
		BLENDMODE_DEFAULT  = 0,
		BLENDMODE_ALPHA    = 1, // alpha blended
		BLENDMODE_MODULATE = 2, // the colour multiplies the destination
		BLENDMODE_ADDITIVE = 3, // the colour (times alpha) is added to the destination
	};

	friend class AModelLoader;
	friend class ASkinSwap;

public:
	ASkinMaterial();
	~ASkinMaterial();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode );
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode );

	BLENDMODE GetBlendMode() const { return m_eBlendMode; }

	// NOTE: the meaning of the flag (0x64) is unknown, ABarnYardPoolMiniGameState clears it on its units
	void SetFlag( TBOOL a_bFlag ) { m_bFlag = a_bFlag; }

	void SetTexture( Toshi::TTexture* a_pTexture )
	{
		m_pTexture = a_pTexture;
		SetTextureNum( 1 );
	}

	void SetLightingTexture( ELightingTexture a_eTex, Toshi::TTexture* a_pTexture )
	{
		m_apLightingTextures[ a_eTex ] = a_pTexture;
	}

protected:
	Toshi::TTexture* m_pTexture;
	Toshi::TTexture* m_apLightingTextures[ LT_NUMOF ];
	BLENDMODE        m_eBlendMode;
	TBOOL            m_bFlag;
};
