#include "ToshiPCH.h"
#include "TViewport_DX11.h"
#include "TRender_DX11.h"
#include "Includes.h"

namespace Toshi
{
	TViewport::TViewport()
	{
		m_ColorR = 0;
		m_ColorG = 0;
		m_ColorB = 0;
		m_ColorA = 0;
		m_bInViewport = TFALSE;
		m_bAllowBackgroundClear = TTRUE;
		m_bAllowDepthClear = TTRUE;
		m_bDefaultBeginRender = TTRUE;
		m_pRenderCtx = TRender::GetSingletonWeak()->CreateRenderContext();
		m_MemAllocatorBlock = TNULL;

		TIMPLEMENT();
	}

	TViewport::~TViewport()
	{
		TFALSE;
	}

	void TViewport::BeginSKU()
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = GetX();
		viewport.TopLeftY = GetY();
		viewport.Width = GetWidth();
		viewport.Height = GetHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		auto pRender = TRenderDX11::Interface();
		auto pDeviceContext = pRender->GetDeviceContext();

		if (m_bAllowBackgroundClear || m_bAllowDepthClear)
		{
			ID3D11RenderTargetView* pRenderTargetView;
			ID3D11DepthStencilView* pDepthStencilView;

			pDeviceContext->OMGetRenderTargets(1, &pRenderTargetView, &pDepthStencilView);

			if (m_bAllowBackgroundClear && pRenderTargetView != TNULL)
			{
				uint8_t r, g, b, a;
				GetBackgroundColor(r, g, b, a);

				FLOAT clearColor[4];
				clearColor[0] = (float)r / 255.0f;
				clearColor[1] = (float)g / 255.0f;
				clearColor[2] = (float)b / 255.0f;
				clearColor[3] = (float)a / 255.0f;

				pDeviceContext->ClearRenderTargetView(pRenderTargetView, clearColor);
			}

			if (m_bAllowDepthClear && pDepthStencilView != TNULL)
			{
				pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, viewport.MaxDepth, 0);
			}
		}
	}

	void TViewport::EndSKU()
	{
		TRenderDX11::Interface()->FUN_00691190();
	}

	void TViewport::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		if (m_ColorR != r || m_ColorG != g || m_ColorB != b || m_ColorA != a)
		{
			m_ColorR = r;
			m_ColorG = g;
			m_ColorB = b;
			m_ColorA = a;

			ChangeSKU(ChangeEvent_Unk5);
		}
	}

	void TViewport::GetBackgroundColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
	{
		r = m_ColorR;
		g = m_ColorG;
		b = m_ColorB;
		a = m_ColorA;
	}
}