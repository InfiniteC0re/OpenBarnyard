#include "ToshiPCH.h"
#include "TPrimShader_DX11.h"
#include "TRender_DX11.h"
#include "TRenderContext_DX11.h"
#include <DirectXMath.h>

namespace Toshi
{
	TPrimShader::TPrimShader()
	{
		Initialize();
	}

	TPrimShader::~TPrimShader()
	{
		Destroy();
	}

	void TPrimShader::Destroy()
	{
		m_pBuffer->Release();
		m_pVShader->Release();
		m_pPShaderImage->Release();
		m_pPShaderBasic->Release();
		m_pInputLayout->Release();
		TFree(m_pMeshes);
	}

	bool TPrimShader::AddMesh()
	{
		TASSERT(TFALSE == m_bIsLocked);

		if (m_uiNumUsedMeshes >= m_uiNumMeshes)
		{
			return TFALSE;
		}

		UINT meshIndex = m_uiNumUsedMeshes;

		m_bIsLocked = TTRUE;
		m_uiNumUsedVerts = 0;
		m_uiNumUsedMeshes += 1;

		D3D11_MAPPED_SUBRESOURCE subResource;
		TRenderDX11::Interface()->m_pDeviceContext->Map(
			m_pBuffer,
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&subResource
		);

		m_pCurrentVertex = (TPrimShader::Vertex*)subResource.pData + m_uiNumUsedVerts;

		m_pMeshes[meshIndex].uiStartPrim = m_uiNumUsedVerts;
		m_pMeshes[meshIndex].uiNumPrims = 0;

		return TTRUE;
	}

	void TPrimShader::AddVert()
	{
		if (m_bIsLocked)
		{
			m_pCurrentVertex += 1;
			m_uiNumUsedVerts += 1;

			TASSERT(m_uiNumUsedVerts < ONE_VERTEX_BUFFER_SIZE);
			GetCurrentMesh()->uiNumPrims += 1;
		}
	}

	void TPrimShader::Unlock()
	{
		TASSERT(TTRUE == m_bIsLocked);
		m_bIsLocked = TFALSE;

		Mesh* pMesh = GetCurrentMesh();
		TRenderDX11::Interface()->m_pDeviceContext->Unmap(m_pBuffer, 0);
		DrawPrim(m_ePrimType, pMesh->uiNumPrims, pMesh->uiStartPrim);
	}

	void TPrimShader::StartRendering(PrimType primType)
	{
		TRenderDX11* pRender = TRenderDX11::Interface();
		TRenderContextDX11* pRenderContext = static_cast<TRenderContextDX11*>(pRender->GetCurrentRenderContext());
		TASSERT(pRender->IsInScene());
		TASSERT(m_bIsRendering == TFALSE);
		m_bIsRendering = TTRUE;
		m_ePrimType = primType;

		if (m_uiNumDrawnFrames != pRender->m_NumDrawnFrames)
		{
			// If we start rendering in a new frame, we don't need meshes from the old frame anymore
			m_uiNumDrawnFrames = pRender->m_NumDrawnFrames;
			m_uiNumUsedMeshes = 0;
		}

		AddMesh();
		pRender->m_pDeviceContext->VSSetShader(m_pVShader, TNULL, 0);

		if (m_pImage != TNULL)
		{
			m_pImage->Bind(0);
			pRender->m_pDeviceContext->PSSetShader(m_pPShaderImage, TNULL, 0);
		}
		else
		{
			pRender->m_pDeviceContext->PSSetShader(m_pPShaderBasic, TNULL, 0);
		}

		pRender->SetAlphaUpdate(false);

		D3D11_DEPTH_WRITE_MASK depthWriteMask;

		if (m_eBlendMode == 0)
		{
			pRender->SetBlendMode(true, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO);
			depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		}
		else if (m_eBlendMode == 2)
		{
			pRender->SetBlendMode(true, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE);
			depthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		}
		else
		{
			pRender->SetBlendMode(true, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
			depthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		}

		pRender->SetZMode(true, D3D11_COMPARISON_LESS_EQUAL, depthWriteMask);

		TMatrix44 worldViewProjection = pRenderContext->GetModelViewMatrix().XMM() * pRenderContext->GetProjectionMatrix().XMM();

		pRender->SetVec4InVSBuffer(0, &worldViewProjection, 4);
		pRender->m_pDeviceContext->IASetInputLayout(m_pInputLayout);
		pRender->m_CurrentRasterizerId.Flags.Parts.CullMode = D3D11_CULL_NONE;
	}

	void TPrimShader::StopRendering()
	{
		TRenderDX11* pRender = TRenderDX11::Interface();
		TASSERT(pRender->IsInScene());
		TASSERT(m_bIsRendering == TTRUE);

		if (m_bIsLocked)
		{
			Unlock();
		}

		m_bIsRendering = TFALSE;
		pRender->SetAlphaUpdate(TTRUE);
		pRender->SetZMode(TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL);
	}

	void TPrimShader::DrawPrim(PrimType primType, UINT numPrim, UINT startPrim)
	{
		D3D11_PRIMITIVE_TOPOLOGY primitive;

		switch (primType)
		{
		case 0:
			primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			break;
		case 1:
			primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case 2:
			primitive = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			TASSERT(numPrim >= 2);
			break;
		case 3:
			primitive = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			TASSERT((numPrim & 1) == 0);
			break;
		default:
			TASSERT(TFALSE, "Unknown prim type");
		}

		TRenderDX11::Interface()->DrawNonIndexed(
			primitive,
			m_pBuffer,
			numPrim,
			sizeof(TPrimShader::Vertex),
			startPrim,
			0
		);
	}

	void TPrimShader::Initialize()
	{
		m_uiNumMeshes = 1000;
		m_uiNumUsedMeshes = 0;
		m_uiNumDrawnFrames = 0;
		m_bIsLocked = TFALSE;
		m_bIsRendering = TFALSE;
		m_ePrimType = 0;
		m_eBlendMode = 0;
		m_pImage = TNULL;

		TRenderDX11* pRender = TRenderDX11::Interface();
		ID3DBlob* pBlob;

		// Vertex shader
		pBlob = pRender->CompileShaderFromFile(
			"Data/Shaders/Other/PrimShader.vs",
			"main",
			"vs_4_0_level_9_3",
			TNULL
		);
		pRender->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pVShader);

		D3D11_INPUT_ELEMENT_DESC inputDescs[3];
		inputDescs[0].SemanticName = "POSITION";
		inputDescs[0].SemanticIndex = 0;
		inputDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputDescs[0].InputSlot = 0;
		inputDescs[0].AlignedByteOffset = -1;
		inputDescs[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDescs[0].InstanceDataStepRate = 0;
		inputDescs[1].SemanticName = "COLOR";
		inputDescs[1].SemanticIndex = 0;
		inputDescs[1].Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		inputDescs[1].InputSlot = 0;
		inputDescs[1].AlignedByteOffset = -1;
		inputDescs[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDescs[1].InstanceDataStepRate = 0;
		inputDescs[2].SemanticName = "TEXCOORD";
		inputDescs[2].SemanticIndex = 0;
		inputDescs[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputDescs[2].InputSlot = 0;
		inputDescs[2].AlignedByteOffset = -1;
		inputDescs[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDescs[2].InstanceDataStepRate = 0;

		pRender->m_pDevice->CreateInputLayout(inputDescs, 3, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pInputLayout);
		pBlob->Release();

		// Pixel shader for image rendering
		pBlob = pRender->CompileShaderFromFile(
			"Data/Shaders/Other/PrimShader.ps",
			"main",
			"ps_4_0_level_9_3",
			TNULL
		);
		pRender->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pPShaderImage);
		pBlob->Release();

		// Pixel shader for color rendering
		pBlob = pRender->CompileShaderFromFile(
			"Data/Shaders/Other/PrimShaderNoTex.ps",
			"main",
			"ps_4_0_level_9_3",
			TNULL
		);
		pRender->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pPShaderBasic);
		pBlob->Release();

		// Allocating the meshes array
		size_t arraySize = m_uiNumMeshes * sizeof(TPrimShader::Mesh);
		m_pMeshes = (Mesh*)TMemalign(16, arraySize);
		TUtil::MemClear(m_pMeshes, arraySize);

		// Allocating the buffer
		m_pBuffer = pRender->CreateBuffer(
			0,
			ONE_VERTEX_BUFFER_SIZE * sizeof(TPrimShader::Vertex),
			TNULL,
			D3D11_USAGE_DYNAMIC,
			D3D11_CPU_ACCESS_WRITE
		);
	}
}
