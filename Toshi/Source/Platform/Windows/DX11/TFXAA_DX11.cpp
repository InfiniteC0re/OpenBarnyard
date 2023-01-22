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
		char* srcData = new char[fileSize + 1];
		pFile->Read(srcData, fileSize);
		srcData[fileSize] = '\0';
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

		// Set our render targets
		pDeviceContext->OMSetRenderTargets(1, &m_pRenderTarget, NULL);

		TVector4 unk;
		unk.x = m_Width;
		unk.y = m_Height;
		unk.z = 0;
		unk.w = 0;

		pRender->CopyToPixelConstantBuffer(26, &unk, 1);
		FUN_006a6700(0.0f, 0.0f, m_Width, m_Height, pShaderResourceView, m_pShader, NULL);

		// Restore original render targets
		pDeviceContext->OMSetRenderTargets(1, &pRenderTargetViews, pDepthStencilView);
		FUN_006a6700(0.0f, 0.0f, m_Width, m_Height, m_pTextureView, NULL, NULL);
	}

	void TFXAA::FUN_006a6700(float posX, float posY, float width, float height, ID3D11ShaderResourceView* pShaderResourceView, ID3D11PixelShader* pPixelShader, const void* srcData)
	{
		auto pRender = TRenderDX11::Interface();
		auto pDeviceContext = pRender->GetDeviceContext();
		pDeviceContext->VSSetShader(pRender->m_pVertexShader, NULL, NULL);
		if (pPixelShader == NULL) pPixelShader = pRender->m_pPixelShader1;
		pDeviceContext->PSSetShader(pPixelShader, NULL, NULL);
		pDeviceContext->IASetInputLayout(pRender->m_pInputLayout);
		pRender->m_Flags &= 0xF7 | 4;
		pRender->m_Flags2 &= 0xD0 | 16;


		pDeviceContext->PSSetShaderResources(0, 1, &pShaderResourceView);
		pDeviceContext->PSSetSamplers(0, 1, &pRender->m_SamplerState2);

		UINT numViewports = 1;
		
		D3D11_VIEWPORT viewPort;

		pDeviceContext->RSGetViewports(&numViewports, &viewPort);

		TVector4 unk;

		unk.x = (width / viewPort.Width) * 2 - 1;
		unk.y = (height / viewPort.Height) * 2 - 1;
		unk.z = (posX / viewPort.Width) * 2;
		unk.w = (posY / viewPort.Height) * 2;

		if (srcData == TNULL)
		{
			srcData = &unk;
			unk.x = 1.0F;
			unk.y = 1.0F;
			unk.z = 0.0F;
			unk.w = 0.0F;
		}
		
		pRender->m_IsVertexConstantBufferSet = true;
		pRender->CopyToVertexConstantBuffer(1, srcData, 1);

		pRender->FUN_006a8d30();

		UINT stride = 20;
		UINT offsets = 0;

		pDeviceContext->IASetVertexBuffers(0, 1, &pRender->m_pSomeBuffer, &stride, &offsets);

		pRender->UnsetConstantBuffers();

		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		pDeviceContext->Draw(4, 0);
	}
}
