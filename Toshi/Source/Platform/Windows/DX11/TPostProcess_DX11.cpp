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

	static inline float GaussianDistribution(float x, float y, float rho) noexcept
	{
		return expf(-(x * x + y * y) / (2 * rho * rho)) / sqrtf(2 * DirectX::XM_PI * rho * rho);
	}

	void TPostProcess::GaussianBlur5x5(int texWidth, int texHeight, TVector4* offsets, TVector4* weights, float fUnk)
	{
		const float tu = 1.0f / float(texWidth);
		const float tv = 1.0f / float(texHeight);

		float totalWeight = 0.0f;
		size_t index = 0;


		for (int x = -2; x <= 2; ++x)
		{
			for (int y = -2; y <= 2; ++y)
			{
				// Exclude pixels with a block distance greater than 2. This will
				// create a kernel which approximates a 5x5 kernel using only 13
				// sample points instead of 25; this is necessary since 2.0 shaders
				// only support 16 texture grabs.
				if (fabsf(float(x)) + fabsf(float(y)) > 2.0f)
					continue;

				// Get the unscaled Gaussian intensity for this offset
				offsets[index].x = float(x) * tu;
				offsets[index].y = float(y) * tv;
				offsets[index].z = 0.0f;
				offsets[index].w = 0.0f;

				const float g = GaussianDistribution(float(x), float(y), 1.0f);
				weights[index] = DirectX::XMVectorReplicate(g);

				totalWeight += DirectX::XMVectorGetX(weights[index].XMM());

				++index;
			}
		}

		// Divide the current weight by the total weight of all the samples; Gaussian
		// blur kernels add to 1.0f to ensure that the intensity of the image isn't
		// changed when the blur occurs. An optional multiplier variable is used to
		// add or remove image intensity during the blur.
		const DirectX::XMVECTOR vtw = DirectX::XMVectorReplicate(totalWeight);
		const DirectX::XMVECTOR vm = DirectX::XMVectorReplicate(1.0f);
		for (size_t i = 0; i < index; ++i)
		{
			weights[i] = DirectX::XMVectorDivide(weights[i].XMM(), vtw);
			weights[i] = DirectX::XMVectorMultiply(weights[i].XMM(), vm);
		}
	}

	void TPostProcess::DrawScreenOverlay()
	{
		TASSERT(m_pQuadVtxDecl);
		TASSERT(m_pQuadVB);
		TASSERT(m_pScreenSpaceVS);

		auto renderer = TRenderDX11::Interface();
		auto deviceContext = renderer->GetDeviceContext();

		renderer->m_CurrentRasterizerId.Flags.Parts.CullMode = D3D11_CULL_NONE;
		renderer->SetZMode(TFALSE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);

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

		TVector4 avSampleOffsets[MAX_SAMPLES];
		TVector4 avSampleWeights[MAX_SAMPLES];

		GaussianBlur5x5(texDesc.Width, texDesc.Height, avSampleOffsets, avSampleWeights, 1.0f);

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
