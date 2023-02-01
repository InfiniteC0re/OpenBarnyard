#pragma once
#include "Toshi/Render/TMaterial.h"
#include "Platform/Windows/DX11/TTexture_DX11.h"

namespace Toshi
{
	class T2GUIMaterial : public TMaterial
	{
	public:
		T2GUIMaterial();

		void SetBlendMode()
		{

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
	};
}