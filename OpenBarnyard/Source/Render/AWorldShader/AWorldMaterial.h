#pragma once
#include <Render/TMaterial.h>
#include <Render/TTexture.h>

class AWorldMaterial : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS(Toshi::TMaterial);

	static constexpr TUINT MAX_TEXTURES = 4;

	using BLENDMODE = TINT;

public:
	AWorldMaterial();
	~AWorldMaterial();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create(BLENDMODE a_eBlendMode);
	virtual void SetBlendMode(BLENDMODE a_eBlendMode);

	AWorldMaterial::BLENDMODE GetBlendMode() const { return m_eBlendMode; }

protected:
	Toshi::TTexture* m_aTextures[MAX_TEXTURES];
	BLENDMODE m_eBlendMode;
	TFLOAT m_fUVAnimX;
	TFLOAT m_fUVAnimY;
	TFLOAT m_fUVAnimSpeedX;
	TFLOAT m_fUVAnimSpeedY;
};