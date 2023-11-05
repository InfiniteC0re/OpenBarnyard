#pragma once
#include <Toshi/Render/TMaterial.h>
#include <Toshi/Render/TTexture.h>

class AGUI2Material : public Toshi::TMaterial
{
public:
	friend class AGUI2Renderer;
	friend class AGUI2RendererDX8;
	
public:
	AGUI2Material()
	{
		m_iNumTex = 1;
		m_pTextureResource = TNULL;
		m_iUnk1 = 1;
		m_iUnk2 = 1;
		m_iUnk3 = 0;
	}

private:
	Toshi::TTexture* m_pTextureResource;
	TUINT m_iUnk1;
	TUINT m_iUnk2;
	TUINT m_iUnk3;
};
