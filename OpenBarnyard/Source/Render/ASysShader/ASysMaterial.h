#pragma once
#include "ASysShader.h"

#include <Render/TMaterial.h>

class ASysMaterial : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS(Toshi::TMaterial);

	enum BLENDMODE
	{
		BLENDMODE_DEFAULT,
		BLENDMODE_1,
		BLENDMODE_2,
		BLENDMODE_3,
		BLENDMODE_4,
		BLENDMODE_5,
		BLENDMODE_6,
		BLENDMODE_7,
		BLENDMODE_8,
		BLENDMODE_9,
		BLENDMODE_10,
		BLENDMODE_NUMOF,
	};

public:
	ASysMaterial();

	virtual TBOOL Create() override;
	virtual TBOOL Create(BLENDMODE a_eBlendMode);
	virtual void SetBlendMode(BLENDMODE a_eBlendMode);

	void SetTexture(Toshi::TTexture* a_pTexture)
	{
		m_pTexture = a_pTexture;
	}

	ASysShader* GetSysShader() const
	{
		return TSTATICCAST(ASysShader*, GetShader());
	}

protected:
	Toshi::TTexture* m_pTexture;
	TUINT m_Unk1;
	TUINT m_Unk2;
	TUINT m_Unk3;
	BLENDMODE m_eBlendMode;
	Toshi::TVector4 m_Vec3;
};
