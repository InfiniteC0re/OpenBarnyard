#include "ToshiPCH.h"
#include "TToneMap_DX11.h"
#include "TRender_DX11.h"

namespace Toshi
{
	TToneMap::TToneMap() :
		m_pTextureView(TNULL),
		m_pRenderTarget(TNULL),
		m_Unk1(0),
		m_Unk2(0),
		m_Unk3(0),
		m_Unk4(0),
		m_Unk5(0)
	{
		m_pTextureView = TRenderDX11::Interface()->CreateTexture(128, 128, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 4, D3D11_USAGE_DEFAULT, 0, 1);
		
		if (m_pTextureView != TNULL)
		{
			m_pRenderTarget = TRenderDX11::Interface()->CreateRenderTargetView(m_pTextureView);
		}

		TASSERT(m_pRenderTarget != TNULL);
	}

	TToneMap::~TToneMap()
	{
		if (m_pTextureView)
		{
			m_pTextureView->Release();
		}

		if (m_pRenderTarget)
		{
			m_pRenderTarget->Release();
		}

		TTODO("Free m_Unk4 and m_Unk5");
	}
}
