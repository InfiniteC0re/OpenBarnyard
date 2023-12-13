#include "pch.h"
#include "ASysShaderHAL_DX8.h"
#include "ASysMaterialHAL_DX8.h"
#include "ASysMesh_DX8.h"

#include TOSHI_MULTIRENDER(TRenderInterface)
#include TOSHI_MULTIRENDER(TRenderContext)
#include TOSHI_MULTIRENDER(TVertexPoolResource)
#include TOSHI_MULTIRENDER(TIndexPoolResource)

TOSHI_NAMESPACE_USING

void ASysShaderHAL::Flush()
{
	Validate();

	auto pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();
	auto pRenderContext = TSTATICCAST(
		TRenderContextD3D*,
		TRenderD3DInterface::Interface()->GetCurrentRenderContext()
	);

	pDevice->SetTextureStageState(0, D3DTSS_MINFILTER, 2);
	pDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, 2);
	pDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, 1);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, 1);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, 1);
	pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
	pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
	pDevice->SetVertexShader(0x142);

	static Toshi::TMatrix44 s_Identity;
	static TUINT s_IdentityCreated;

	if ((s_IdentityCreated & 1) == 0)
	{
		s_Identity = Toshi::TMatrix44::IDENTITY;
		s_IdentityCreated |= 1;
	}

	pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&pRenderContext->GetProjectionMatrix());
	pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&s_Identity);
	pDevice->SetPixelShader(0);

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHAREF, 1);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);
	pDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_LIGHTING, 0);
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);

	m_aOrderTables[0].Render();
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

void ASysShaderHAL::StartFlush()
{
	Validate();

	auto pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();
	auto pRenderContext = TSTATICCAST(
		TRenderContextD3D*,
		TRenderD3DInterface::Interface()->GetCurrentRenderContext()
	);

	pDevice->SetTextureStageState(0, D3DTSS_MINFILTER, 2);
	pDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, 2);
	pDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, 1);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, 1);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, 1);
	pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
	pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
	pDevice->SetVertexShader(0x142);

	static Toshi::TMatrix44 s_Identity;
	static TUINT s_IdentityCreated;

	if ((s_IdentityCreated & 1) == 0)
	{
		s_Identity = Toshi::TMatrix44::IDENTITY;
		s_IdentityCreated |= 1;
	}

	pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&pRenderContext->GetProjectionMatrix());
	pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&s_Identity);
	pDevice->SetPixelShader(0);

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHAREF, 1);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);
	pDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_LIGHTING, 0);
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

void ASysShaderHAL::EndFlush()
{
	auto pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

TBOOL ASysShaderHAL::Create()
{
	TASSERT(TFALSE == IsCreated());

	m_aOrderTables[0].Create(this, -1000);
	m_aOrderTables[1].Create(this, 2000);
	m_aOrderTables[2].Create(this, -6000);
	m_aOrderTables[3].Create(this, -5990);

	ASysShader::Create();
	return Validate();
}

TBOOL ASysShaderHAL::Validate()
{
	if (!IsValidated())
	{
		return ASysShader::Validate();
	}

	return TTRUE;
}

void ASysShaderHAL::Invalidate()
{
	if (IsValidated())
	{
		return ASysShader::Invalidate();
	}
}

TBOOL ASysShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL ASysShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void ASysShaderHAL::Render(Toshi::TRenderPacket* pPacket)
{
	TASSERT(pPacket->GetMesh()->IsExactly(&TGetClass(ASysMeshHAL)));

	auto pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();
	ASysMeshHAL* pMeshHAL = TSTATICCAST(ASysMeshHAL*, pPacket->GetMesh());

	pDevice->SetTransform(D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&pPacket->GetModelViewMatrix());
	pDevice->SetRenderState(D3DRS_ZBIAS, pMeshHAL->GetZBias());
	
	auto pVertexPool = TSTATICCAST(TVertexPoolResource*, pMeshHAL->GetVertexPool());
	auto pIndexPool = TSTATICCAST(TIndexPoolResource*, pMeshHAL->GetIndexPool());

	TVertexBlockResource::HALBuffer vertexHALBuffer;
	pVertexPool->GetHALBuffer(&vertexHALBuffer);

	TIndexBlockResource::HALBuffer indexHALBuffer;
	pIndexPool->GetHALBuffer(&indexHALBuffer);

	pDevice->SetStreamSource(0, vertexHALBuffer.apVertexBuffers[0], 24);
	pDevice->SetIndices(indexHALBuffer.pIndexBuffer, indexHALBuffer.uiIndexOffset);

	if (ms_bRenderEnabled)
	{
		if (pIndexPool->GetFlags() & 8)
		{
			pDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLESTRIP,
				0,
				pVertexPool->GetNumVertices(),
				indexHALBuffer.uiIndexOffset,
				pIndexPool->GetNumIndices() - 2
			);
		}
		else
		{
			pDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST,
				0,
				pVertexPool->GetNumVertices(),
				indexHALBuffer.uiIndexOffset,
				pIndexPool->GetNumIndices() / 3
			);
		}
	}

	pDevice->SetRenderState(D3DRS_ZBIAS, 0);
}

ASysMaterial* ASysShaderHAL::CreateMaterial(const char* a_szName)
{
	Validate();

	auto pMaterialHAL = new ASysMaterialHAL();
	pMaterialHAL->SetShader(this);
	pMaterialHAL->SetOrderTable(&m_aOrderTables[0]);

	return pMaterialHAL;
}

ASysMesh* ASysShaderHAL::CreateMesh(const char* a_szName)
{
	Validate();

	auto pMeshHAL = new ASysMeshHAL();
	pMeshHAL->SetOwnerShader(this);

	return pMeshHAL;
}
