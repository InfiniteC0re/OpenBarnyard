#include "ToshiPCH.h"
#include "TPostProcess_DX11.h"
#include <Platform/Windows/DX11/TRender_DX11.h>

namespace Toshi {

	TPostProcess::TPostProcess()
	{
		TTODO("Allocate some buffers, set other variables");

		auto renderer = TRenderDX11::Interface();
		auto shader = renderer->CompileShaderFromFile("Data\\Shaders\\Other\\ScreenSpace.vs", "main", "vs_4_0_level_9_3", NULL);
		renderer->CreateVertexShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pScreenSpaceVS);

		D3D11_INPUT_ELEMENT_DESC inputDesc;
		inputDesc.SemanticName = "POSITION";
		inputDesc.SemanticIndex = 0;
		inputDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		inputDesc.InputSlot = 0;
		inputDesc.AlignedByteOffset = -1;
		inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc.InstanceDataStepRate = 0;
		renderer->m_pDevice->CreateInputLayout(&inputDesc, 1, shader->GetBufferPointer(), shader->GetBufferSize(), &m_pQuadVtxDecl);
		shader->Release();

		FLOAT vertices[8] = {
				-1.0f, -1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, 1.0f, 1.0f
		};

		m_pQuadVB = renderer->CreateBuffer(0, sizeof(vertices), vertices, D3D11_USAGE_IMMUTABLE, 0);

		// GaussBlur5x5
		shader = renderer->CompileShaderFromFile("Data\\Shaders\\Other\\GaussBlur5x5.ps", "main", "ps_4_0_level_9_3", NULL);
		renderer->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pPSGaussBlur5x5);
		shader->Release();

		// GaussBlurMultByAlpha
		shader = renderer->CompileShaderFromFile("Data\\Shaders\\Other\\GaussBlurMultByAlpha.ps", "main", "ps_4_0_level_9_3", NULL);
		renderer->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pPSGaussBlurAlpha);
		shader->Release();

		// GaussBlurMultByAlphaStencil
		shader = renderer->CompileShaderFromFile("Data\\Shaders\\Other\\GaussBlurMultByAlphaStencil.ps", "main", "ps_4_0", NULL);
		renderer->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pPSGaussBlurAlphaStencil);
		shader->Release();
	}

	void TPostProcess::DrawScreenOverlay()
	{
		TASSERT(m_pQuadVtxDecl);
		TASSERT(m_pQuadVB);
		TASSERT(m_pScreenSpaceVS);

		auto renderer = TRenderDX11::Interface();
		auto deviceContext = renderer->GetDeviceContext();

		renderer->m_CurrentRasterizerId.Flags.Parts.CullMode = D3D11_CULL_NONE;
		renderer->SetZMode(false, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);

		deviceContext->IASetInputLayout(m_pQuadVtxDecl);
		deviceContext->VSSetShader(m_pScreenSpaceVS, TNULL, 0);
		renderer->DrawNonIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pQuadVB, 4, 8, 0, 0);
	}

	void TPostProcess::RenderImpl(ID3D11PixelShader* pPS, ID3D11ShaderResourceView* pSRV, float fIntensity, ID3D11ShaderResourceView* pStencilSRV)
	{
		auto renderer = TRenderDX11::Interface();
		auto deviceContext = renderer->GetDeviceContext();

		ID3D11Resource* res;
		pSRV->GetResource(&res);

		ID3D11Texture2D* tex;
		res->QueryInterface(IID_ID3D11Texture2D, (void**)&tex);

		D3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		tex->Release();
		res->Release();

		TTODO("FUN_006b8350");

		TVector4 avSampleOffsets[MAX_SAMPLES];
		TVector4 avSampleWeights[MAX_SAMPLES];

		renderer->SetVec4InPSBuffer(29, avSampleOffsets, MAX_SAMPLES);
		renderer->SetVec4InPSBuffer(45, avSampleWeights, MAX_SAMPLES);

		if (pStencilSRV)
		{
			TVector4 stencilSize = { (TFloat)texDesc.Width, (TFloat)texDesc.Height, 0, 0 };
			renderer->SetVec4InPSBuffer(61, &stencilSize, 1);
		}

		deviceContext->PSSetShader(pPS, NULL, 0);
		deviceContext->PSSetShaderResources(0, 1, &pSRV);
		renderer->SetSamplerState(0, 0, TRUE);

		if (pStencilSRV)
		{
			deviceContext->PSSetShaderResources(1, 1, &pStencilSRV);
			renderer->SetSamplerState(1, 0, TRUE);
		}

		DrawScreenOverlay();

		if (pStencilSRV)
		{
			deviceContext->PSSetShaderResources(1, 1, NULL);
		}
	}

}
