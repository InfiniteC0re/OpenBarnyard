#pragma once

#include "Platform/Windows/DX11/Includes.h"

namespace Toshi
{
	class TPostProcess
	{

		void Render();

	private:
		ID3D11InputLayout* m_pQuadVtxDecl;    // 0x0
		ID3D11Buffer* m_pQuadVB;              // 0x4
		ID3D11VertexShader* m_pScreenSpaceVS; // 0x8
	};
}