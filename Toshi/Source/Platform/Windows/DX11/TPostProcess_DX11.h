#pragma once

#include "Platform/Windows/DX11/Includes.h"

namespace Toshi {

	class TPostProcess
	{
	public:
		static constexpr size_t MAX_SAMPLES = 16;

	public:
		TPostProcess();

		void ApplyGaussBlur(ID3D11ShaderResourceView* pSRV, float fIntensity)
		{
			RenderImpl(m_pPSGaussBlur5x5, pSRV, fIntensity, TNULL);
		}

		void ApplyGaussBlurWithAlpha(ID3D11ShaderResourceView* pSRV, float fIntensity, ID3D11ShaderResourceView* pStencilSRV)
		{
			RenderImpl(pStencilSRV ? m_pPSGaussBlurAlphaStencil : m_pPSGaussBlurAlpha, pSRV, fIntensity, pStencilSRV);
		}

	private:
		void GaussianBlur5x5(int texWidth, int texHeight, TVector4* offsets, TVector4* weights, float fUnk);
		void DrawScreenOverlay();
		void RenderImpl(ID3D11PixelShader* pPS, ID3D11ShaderResourceView* pSRV, float fIntensity, ID3D11ShaderResourceView* pStencilSRV);

	private:
		ID3D11InputLayout* m_pQuadVtxDecl;             // 0x00
		ID3D11Buffer* m_pQuadVB;                       // 0x04
		ID3D11VertexShader* m_pScreenSpaceVS;          // 0x08

		ID3D11PixelShader* m_pPSGaussBlur5x5;          // 0x30
		ID3D11PixelShader* m_pPSGaussBlurAlpha;        // 0x34
		ID3D11PixelShader* m_pPSGaussBlurAlphaStencil; // 0x38
	};

}