#include "ToshiPCH.h"
#include "TPostProcess_DX11.h"
#include <Platform/Windows/DX11/TRender_DX11.h>

using namespace Toshi;

void TPostProcess::Render()
{
	TASSERT(m_pQuadVtxDecl);
	TASSERT(m_pQuadVB);
	TASSERT(m_pScreenSpaceVS);

	auto renderer = Toshi::TRenderDX11::Interface();
	auto deviceContext = renderer->GetDeviceContext();

	TTODO("renderer->m_CurrentRasterizerId.Flags = renderer->m_CurrentRasterizerId.Flags & 0xF7 | 4;");
	renderer->SetZMode(false, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);
	deviceContext->IASetInputLayout(m_pQuadVtxDecl);
	deviceContext->VSSetShader(m_pScreenSpaceVS, TNULL, 0);
	renderer->DrawMesh(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pQuadVB, 4, 8, 0, 0);
}
