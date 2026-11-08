#pragma once
#include "Render/AFXShader/AFXMaterial.h"

class MODAPI AFXMaterialHAL : public AFXMaterial
{
public:
	TDECLARE_CLASS( AFXMaterialHAL, AFXMaterial );

	friend class AFXShaderHAL;

public:
	AFXMaterialHAL();
	~AFXMaterialHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual void PreRender() override;
	virtual void PostRender() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void CopyToAlphaBlendMaterial(); // +0x24

	AFXMaterialHAL* GetAlphaBlendMaterial() const { return m_pAlphaBlendMaterial; }

private:
	AFXMaterialHAL* m_pAlphaBlendMaterial; // 0x64
};
