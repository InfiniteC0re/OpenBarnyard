#pragma once

#include "Platform/Windows/DX11/Includes.h"

namespace Toshi
{
	class TPostProcess
	{
	public:

		TPostProcess();

		void Render();
		void FUN_006b8b00(ID3D11PixelShader* ps, ID3D11ShaderResourceView* srv, float fVal, ID3D11ShaderResourceView* srv2);
		void FUN_006b91b0(ID3D11ShaderResourceView* srv, float fVal, ID3D11ShaderResourceView* srv2)
		{
			FUN_006b8b00(srv2 ? m_pPS2 : m_pPS1, srv, fVal, srv2);
		}

	private:

		static constexpr const char* s_defaultPostProcessShader = " struct VS_OUT                                 {                                                 float4 Position : POSITION;                   float2 UV : TEXCOORD0;                    };                                                                                          VS_OUT main( float2 Position : POSITION )     {                                                 VS_OUT Out;                                   Out.Position = float4(Position, 0.0f, 1.0f);       Out.UV = Position.xy * float2(0.5f, -0.5f) + 0.5f;           return Out;                               }                                            ";

		ID3D11InputLayout* m_pQuadVtxDecl;    // 0x0
		ID3D11Buffer* m_pQuadVB;              // 0x4
		ID3D11VertexShader* m_pScreenSpaceVS; // 0x8

		ID3D11PixelShader* m_pPS1;            // 0x34
		ID3D11PixelShader* m_pPS2;            // 0x38
	};
}