#include "ToshiPCH.h"
#include "TGlow_DX11.h"
#include "TRender_DX11.h"

using namespace Toshi;

TGlow::TGlow()
{
	m_pTexture = TNULL;
	m_fIntensity = 0.0f;
	m_fDist = 2.0f;
	m_pGlowSrv = TNULL;
	m_pTexture2 = TNULL;
	m_pGlowSrv2 = TNULL;
	m_pRenderTarget = TNULL;
	m_pPixelShader = TNULL;

	auto renderer = Toshi::TRenderDX11::Interface();
	m_uiWidth = renderer->m_SwapChainDesc.BufferDesc.Width / 2;
	m_uiHeight = renderer->m_SwapChainDesc.BufferDesc.Height / 2;
	m_pGlowSrv = renderer->CreateTexture(m_uiWidth, m_uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, 0, D3D11_USAGE_DEFAULT, 0, 1);
	TASSERT(m_pGlowSrv);

	if (m_pGlowSrv)
	{
		ID3D11Resource* textureResource = TNULL;
		m_pGlowSrv->GetResource(&textureResource);

		HRESULT hr = textureResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pTexture));
		if (textureResource || FAILED(hr))
		{
			textureResource->Release();
		}
	}

	m_pGlowSrv2 = renderer->CreateTexture(m_uiWidth, m_uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, 4, D3D11_USAGE_DEFAULT, 0, 1);
	m_pRenderTarget = renderer->CreateRenderTargetView(m_pGlowSrv2);
	TASSERT(m_pRenderTarget);

	if (m_pGlowSrv2)
	{
		ID3D11Resource* textureResource = TNULL;
		m_pGlowSrv->GetResource(&textureResource);

		HRESULT hr = textureResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pTexture2));
		if (textureResource || FAILED(hr))
		{
			textureResource->Release();
		}
	}

	ID3DBlob* shader = renderer->CompileShader(s_defaultGlowPixelShader, "main", "ps_4_0_level_9_3", TNULL);
	renderer->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pPixelShader);
}

void TGlow::Render(ID3D11ShaderResourceView* srv, void* unk)
{
	if (m_fIntensity > 0.0f)
	{
		auto renderer = Toshi::TRenderDX11::Interface();
		auto deviceContext = renderer->GetDeviceContext();
		ID3D11RenderTargetView* pRenderTargetView;
		ID3D11DepthStencilView* pDepthStencilView;
		UINT numViewports = 1;
		D3D11_VIEWPORT viewports;
		const FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		deviceContext->OMGetRenderTargets(1, &pRenderTargetView, &pDepthStencilView);
		deviceContext->RSGetViewports(&numViewports, &viewports);
		deviceContext->ClearRenderTargetView(m_pRenderTarget, color);
		deviceContext->OMSetRenderTargets(1, &m_pRenderTarget, TNULL);

		D3D11_VIEWPORT viewport;
		viewport.Width = m_uiWidth;
		viewport.TopLeftX = 0.0;
		viewport.TopLeftY = 0.0;
		viewport.MinDepth = 0.0;
		viewport.MaxDepth = 1.0;
		viewport.Height = m_uiHeight;
		deviceContext->RSSetViewports(1, &viewport);
		renderer->SetBlendMode(TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE);

		TIMPLEMENT();

		deviceContext->CopyResource(m_pTexture, m_pTexture2);
		deviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

		if (pRenderTargetView) pRenderTargetView->Release();
		if (pDepthStencilView) pDepthStencilView->Release();

		deviceContext->RSSetViewports(1, &viewports);

		auto uvs = new TVector4(viewports.Width, viewports.Height, viewports.TopLeftX, viewports.TopLeftY);

		renderer->FUN_006a6700(0.0f, 0.0f, viewports.Width, viewports.Height, srv, TNULL, uvs);
		renderer->SetAlphaUpdate(0);
		renderer->SetBlendMode(true, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE);
		renderer->SetVec4InPSBuffer(28, &m_fIntensity);
		renderer->FUN_006a6700(0.0f, 0.0f, viewports.Width, viewports.Height, srv, m_pPixelShader, uvs);
	}
}
