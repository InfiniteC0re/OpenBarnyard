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

		TFile* pFile = TFile::Create("Data/Shaders/Other/FXAA.ps");
		DWORD fileSize = pFile->GetSize();
		char* srcData = new char[fileSize];
		pFile->Read(srcData, fileSize);
		pFile->Destroy();

		ID3DBlob* shader = TRenderDX11::CompileShader(srcData, "main", "ps_4_0", macro);
		delete[] srcData;

		TRenderDX11::Interface()->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pShader);
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
	}

	void TFXAA::Render(ID3D11ShaderResourceView* pShaderResourceView)
	{
		auto pRender = TRenderDX11::Interface();
		auto pDeviceContext = pRender->GetDeviceContext();
		ID3D11RenderTargetView* pRenderTargetViews;
		ID3D11DepthStencilView* pDepthStencilView;

		// Save current render targets
		pDeviceContext->OMGetRenderTargets(1, &pRenderTargetViews, &pDepthStencilView);
		TTODO("TRenderDX11::FUN_006a7d00 and etc...");

		// Set our render targets
		pDeviceContext->OMSetRenderTargets(1, &m_pRenderTarget, NULL);

		TVector4 unk;
		unk.x = m_Width;
		unk.y = m_Height;
		unk.z = 0;
		unk.w = 0;

		pRender->CopyToPixelConstantBuffer(26, &unk, 1);

		// Restore original render targets
		pDeviceContext->OMSetRenderTargets(1, &pRenderTargetViews, pDepthStencilView);
	}
}
