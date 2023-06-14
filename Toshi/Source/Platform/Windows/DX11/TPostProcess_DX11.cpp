#include "ToshiPCH.h"
#include "TPostProcess_DX11.h"
#include <Platform/Windows/DX11/TRender_DX11.h>

using namespace Toshi;

TPostProcess::TPostProcess()
{
	m_pQuadVtxDecl = NULL;
	m_pQuadVB = NULL;
	m_pScreenSpaceVS = NULL;
	m_pPS1 = NULL;
	m_pPS2 = NULL;

	auto renderer = Toshi::TRenderDX11::Interface();
	auto shader = renderer->CompileShader(s_defaultPostProcessShader, "main", "vs_4_0_level_9_3", NULL);
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

	FLOAT vertices[8] = {
			-1.0f, -1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 1.0f
	};

	renderer->CreateBuffer(0, 32, vertices, D3D11_USAGE_IMMUTABLE, 0);

	TWIP();
}

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
	renderer->DrawNonIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pQuadVB, 4, 8, 0, 0);
}

void Toshi::TPostProcess::FUN_006b8b00(ID3D11PixelShader* ps, ID3D11ShaderResourceView* srv, float fVal, ID3D11ShaderResourceView* srv2)
{
	auto renderer = Toshi::TRenderDX11::Interface();
	auto deviceContext = renderer->GetDeviceContext();

	ID3D11Resource* res;
	srv->GetResource(&res);

	ID3D11Texture2D* tex;
	res->QueryInterface(IID_ID3D11Texture2D, (void**)&tex);

	D3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	tex->Release();
	res->Release();

	TTODO("FUN_006b8350");

	char* data[256];
	char* data2[268];

	renderer->SetVec4InPSBuffer(29, data, 16);
	renderer->SetVec4InPSBuffer(45, data2, 16);

	if (srv2)
	{
		TVector4 unk;
		unk.x = (TFloat)texDesc.Width;
		unk.y = (TFloat)texDesc.Height;
		unk.z = 0;
		unk.w = 0;

		renderer->SetVec4InPSBuffer(61, &unk, 1);
	}

	deviceContext->PSSetShader(ps, NULL, 0);
	deviceContext->PSSetShaderResources(0, 1, &srv);
	renderer->SetSamplerState(0, 0, TRUE);

	if (srv2)
	{
		deviceContext->PSSetShaderResources(1, 1, &srv2);
		renderer->SetSamplerState(1, 0, TRUE);
	}

	Render();

	if (srv2)
	{
		deviceContext->PSSetShaderResources(1, 1, NULL);
	}
}
