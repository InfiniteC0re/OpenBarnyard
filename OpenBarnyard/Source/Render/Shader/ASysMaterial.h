#pragma once
#include <Toshi/Render/TMaterial.h>

TOBJECT(ASysMaterial, Toshi::TMaterial, TFALSE)
{
public:
	enum BLENDMODE
	{
		BLENDMODE_DEFAULT
	};

public:
	ASysMaterial();

	virtual TBOOL Create() override;
	virtual TBOOL Create(BLENDMODE a_eBlendMode);
	virtual void SetBlendMode(BLENDMODE a_eBlendMode);

private:
	Toshi::TVector4 m_Vec1;
	BLENDMODE m_eBlendMode;
	Toshi::TVector4 m_Vec3;
};
