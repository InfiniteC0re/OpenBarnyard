#pragma once
#include "Toshi/Render/TMaterial.h"
#include "Platform/Windows/DX11/TTexture_DX11.h"

namespace Toshi
{
	class T2GUIMaterial : public TMaterial
	{
	public:

		enum BLENDMODE
		{
			BLENDMODE_NORMAL,
			BLENDMODE_ADD,
			BLENDMODE_SUBSTRACT,
			BLENDMODE_MAX
		};

		T2GUIMaterial();

		void SetBlendMode(BLENDMODE blendMode)
		{
			m_eBlendMode = blendMode;
		}

		void SetTexture(TTexture* pTexture)
		{
			m_pTexture = pTexture;
		}

		TTexture* GetTexture()
		{
			return m_pTexture;
		}

	private:
		TTexture* m_pTexture;
		BLENDMODE m_eBlendMode; // 0x54
	};
}