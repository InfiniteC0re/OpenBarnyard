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
	virtual void Create(BLENDMODE a_eBlendMode);
	virtual void SetBlendMode(BLENDMODE a_eBlendMode);

protected:
	Toshi::TTexture* m_aTextures[MAX_TEXTURES];
	BLENDMODE m_eBlendMode;
	TINT m_iUnk1;
	TFLOAT m_fUnk2;
	TINT m_iUnk3;
	TFLOAT m_fUnk4;
};