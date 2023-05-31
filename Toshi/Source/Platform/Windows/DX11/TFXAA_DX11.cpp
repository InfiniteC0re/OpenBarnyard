#include "ToshiPCH.h"
#include "TFXAA_DX11.h"
#include "TRender_DX11.h"

namespace Toshi
{
	TFXAA::TFXAA() : m_Width(1280), m_Height(720), m_pTexture(TNULL), m_pShader(TNULL), m_pRenderTarget(TNULL), m_pTextureView(TNULL)
	{
		D3D_SHADER_MACRO macro[5];
		macro[0].Name = "FXAA_PC";
		macro[0].Definition = "1";
		macro[1].Name = "FXAA_HLSL_4";
		macro[1].Definition = "1";
		macro[2].Name = "FXAA_QUALITY__PRESET";
		macro[2].Definition = "12";
		macro[3].Name = "FXAA_GREEN_AS_LUMA";
		macro[3].Definition = "1";
		macro[4].Name = TNULL;
		macro[4].Definition = TNULL;

		ID3DBlob* shader = TRenderDX11::CompileShaderFromFile("Data/Shaders/Other/FXAA.ps", "main", "ps_4_0", macro);
		TRenderDX11::Interface()->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pShader);
		Initialize();
	}

	TFXAA::~TFXAA()
	{
		if (m_pTexture != TNULL)
		{
			m_pTexture->Release();
		}

		if (m_pTextureView != TNULL)
		{
			m_pTextureView->Release();
		}

		if (m_pRenderTarget != TNULL)
		{
			m_pRenderTarget->Release();
		}

		if (m_pRenderTarget != TNULL)
		{
			m_pShader->Release();
		}
	}

	void TFXAA::Initialize()
	{
		auto renderInterface = TRenderDX11::Interface();
		auto swapChainDesk = renderInterface->GetSwapChainDesc();
		m_Width = swapChainDesk->BufferDesc.Width;
		m_Height = swapChainDesk->BufferDesc.Height;

		m_pTextureView = renderInterface->CreateTexture(m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 4, D3D11_USAGE_DEFAULT, 0, 1);
		m_pRenderTarget = renderInterface->CreateRenderTargetView(m_pTextureView);

		TASSERT(m_pRenderTarget != TNULL);

		if (m_pTextureView != TNULL)
		{
			ID3D11Resource* pResource;
			m_pTextureView->GetResource(&pResource);
			pResource->QueryInterface(_uuidof(ID3D11Texture2D), (void**)&m_pTexture);
			pResource->Release();
		}
		
		m_pTextureView->SetPrivateData(WKPDID_D3DDebugObjectName, 8, "FXAA SRV");
		m_pRenderTarget->SetPrivateData(WKPDID_D3DDebugObjectName, 7, "FXAA RT");
	}

	void TFXAA::Render(ID3D11ShaderResourceView* pShaderResourceView)
	{
		auto pRender = TRenderDX11::Interface();
		auto pDeviceContext = pRender->GetDeviceContext();
		ID3D11RenderTargetView* pRenderTargetViews;
		ID3D11DepthStencilView* pDepthStencilView;

		// Save current render targets
		pDeviceContext->OMGetRenderTargets(1, &pRenderTargetViews, &pDepthStencilView);
		pRender->SetBlendMode(TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);

		// Set our render target
		pDeviceContext->OMSetRenderTargets(1, &m_pRenderTarget, NULL);

		TVector4 unk;
		unk.x = (float)m_Width;
		unk.y = (float)m_Height;
		unk.z = 0;
		unk.w = 0;

		pRender->SetVec4InPSBuffer(PSBufferOffset_V2ScreenSize, &unk, 1);
		TRenderDX11::FUN_006a6700(0.0f, 0.0f, (float)m_Width, (float)m_Height, pShaderResourceView, m_pShader, NULL);

		// Restore original render targets
		pDeviceContext->OMSetRenderTargets(1, &pRenderTargetViews, pDepthStencilView);
		TRenderDX11::FUN_006a6700(0.0f, 0.0f, (float)m_Width, (float)m_Height, m_pTextureView, NULL, NULL);
	}
}
