#include "pch.h"
#include "AGUI2Renderer_DX8.h"

#include "Assets/AMaterialLibraryManager.h"
#include "AGUI2/AGUI2.h"

#include TOSHI_MULTIRENDER(TRenderInterface)

TOSHI_NAMESPACE_USING

AGUI2RendererDX8::AGUI2RendererDX8()
{
	m_pTransforms = new AGUI2Transform[MAX_NUM_TRANSFORMS];
	m_iTransformCount = 0;
	m_bIsTransformDirty = TFALSE;
}

AGUI2RendererDX8::~AGUI2RendererDX8()
{
	delete[] m_pTransforms;
}

AGUI2Material* AGUI2RendererDX8::CreateMaterial(const char* a_szTextureName)
{
	return CreateMaterial(
		GetTexture(a_szTextureName)
	);
}

AGUI2Material* AGUI2RendererDX8::CreateMaterial(Toshi::TTexture* a_pTexture)
{
	auto pMaterial = new AGUI2Material;

	pMaterial->Create();
	pMaterial->m_pTextureResource = a_pTexture;

	return pMaterial;
}

void AGUI2RendererDX8::DestroyMaterial(AGUI2Material* a_pMaterial)
{
	if (a_pMaterial)
	{
		a_pMaterial->OnDestroy();
		delete a_pMaterial;
	}
}

Toshi::TTexture* AGUI2RendererDX8::GetTexture(const char* a_szTextureName)
{
	return AMaterialLibraryManager::GetSingleton()->FindTexture(a_szTextureName);
}

TUINT AGUI2RendererDX8::GetWidth(AGUI2Material* a_pMaterial)
{
	return a_pMaterial->m_pTextureResource->GetWidth();
}

TUINT AGUI2RendererDX8::GetHeight(AGUI2Material* a_pMaterial)
{
	return a_pMaterial->m_pTextureResource->GetHeight();
}

void AGUI2RendererDX8::BeginScene()
{
	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	auto pD3DDevice = pRender->GetDirect3DDevice();

	TRenderD3DInterface::FlushShaders();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	TFLOAT fRootWidth;
	TFLOAT fRootHeight;
	AGUI2::GetContext()->GetRootElement()->GetDimensions(fRootWidth, fRootHeight);

	auto& rTransform = m_pTransforms[m_iTransformCount];
	rTransform.m_Rotation[0] = { pDisplayParams->uiWidth / fRootWidth, 0.0f };
	rTransform.m_Rotation[1] = { 0.0f, pDisplayParams->uiHeight / fRootHeight };
	rTransform.m_Position = { 0.0f, 0.0f };

	static TUINT32 s_MatrixFlags = 0;
	static Toshi::TMatrix44 s_IdentityMatrix;

	if ((s_MatrixFlags & 1) == 0)
	{
		s_MatrixFlags |= 1;
		s_IdentityMatrix = TMatrix44::IDENTITY;
	}

	pD3DDevice->SetTransform(D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&s_IdentityMatrix);
	pD3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&s_IdentityMatrix);

	TMatrix44 projection = {
		2.0f / (float)pDisplayParams->uiWidth, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (float)pDisplayParams->uiHeight, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	pD3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&projection);
	pD3DDevice->SetVertexShader(322);
	pD3DDevice->SetPixelShader(0);
	pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1);
	pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, 0);
	pD3DDevice->SetRenderState(D3DRS_LIGHTING, 0);
	pD3DDevice->SetRenderState(D3DRS_FOGENABLE, 0);
	pD3DDevice->SetRenderState(D3DRS_ZFUNC, 8);
	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0);
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, 1);

	SetColour(0xFFFFFFFF);
	m_pMaterial = TNULL;

	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);
	pD3DDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	pD3DDevice->SetRenderState(D3DRS_BLENDOP, 1);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, 5);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, 6);
	pD3DDevice->SetRenderState(D3DRS_ZBIAS, 0);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, 2);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 0);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 0);
	pD3DDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, 0);
	pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
	pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
	sm_bUnknownFlag = TFALSE;
}

void AGUI2RendererDX8::EndScene()
{
	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	auto pD3DDevice = pRender->GetDirect3DDevice();

	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1);
	pD3DDevice->SetRenderState(D3DRS_ZFUNC, 4);
	pD3DDevice->SetRenderState(D3DRS_ZBIAS, 0);
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	pD3DDevice->SetRenderState(D3DRS_BLENDOP, 1);
	pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	pD3DDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, 2);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 0);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 0);
	pD3DDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, 0);
	pD3DDevice->SetTexture(1, NULL);
	pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
	pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
	pRender->SetTextureStageState(0, 0, -1);
}

void AGUI2RendererDX8::ResetRenderer()
{
	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	auto pD3DDevice = pRender->GetDirect3DDevice();

	pD3DDevice->SetRenderState(D3DRS_BLENDOP, 1);
	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1);
	pD3DDevice->SetRenderState(D3DRS_ZFUNC, 4);
	pD3DDevice->SetRenderState(D3DRS_ZBIAS, 0);
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	pD3DDevice->SetRenderState(D3DRS_BLENDOP, 1);
	pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	pD3DDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, 2);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 0);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 0);
	pD3DDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, 0);
	pD3DDevice->SetTexture(1, NULL);
	pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
	pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
}

void AGUI2RendererDX8::PrepareRenderer()
{
	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	auto pD3DDevice = pRender->GetDirect3DDevice();

	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	static TUINT32 s_MatrixFlags = 0;
	static Toshi::TMatrix44 s_IdentityMatrix;

	if ((s_MatrixFlags & 1) == 0)
	{
		s_MatrixFlags |= 1;
		s_IdentityMatrix = TMatrix44::IDENTITY;
	}

	pD3DDevice->SetTransform(D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&s_IdentityMatrix);
	pD3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&s_IdentityMatrix);

	TMatrix44 projection = {
		2.0f / (float)pDisplayParams->uiWidth, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (float)pDisplayParams->uiHeight, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	pD3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&projection);
	pD3DDevice->SetVertexShader(322);
	pD3DDevice->SetPixelShader(0);
	pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1);
	pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, 0);
	pD3DDevice->SetRenderState(D3DRS_LIGHTING, 0);
	pD3DDevice->SetRenderState(D3DRS_FOGENABLE, 0);
	pD3DDevice->SetRenderState(D3DRS_ZFUNC, 8);
	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0);
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, 1);

	sm_fZCoordinate = 0.1f;

	// Force material to update
	auto pMaterial = m_pMaterial;
	m_pMaterial = (AGUI2Material*)(~(uintptr_t)pMaterial);
	SetMaterial(m_pMaterial);

	// Force colour to update
	auto uiColour = m_uiColour;
	m_uiColour = ~uiColour;
	SetColour(m_uiColour);

	m_bIsTransformDirty = TTRUE;
}

void AGUI2RendererDX8::SetMaterial(AGUI2Material* a_pMaterial)
{
	TIMPLEMENT();
}

void AGUI2RendererDX8::PushTransform(const AGUI2Transform& a_rTransform, const Toshi::TVector2& a_rVec1, const Toshi::TVector2& a_rVec2)
{
	TASSERT(m_iTransformCount < MAX_NUM_TRANSFORMS);
	auto pTransform = m_pTransforms + (m_iTransformCount++);

	AGUI2Transform transform1 = *pTransform;
	AGUI2Transform transform2 = a_rTransform;

	TVector2 vec;
	transform1.Transform(vec, a_rVec1);
	transform1.m_Position = { vec.x, vec.y };

	transform2.Transform(vec, a_rVec2);
	transform2.m_Position = { vec.x, vec.y };

	AGUI2Transform::Multiply(m_pTransforms[m_iTransformCount], transform1, transform2);
	m_bIsTransformDirty = TTRUE;
}

void AGUI2RendererDX8::PopTransform()
{
	TASSERT(m_iTransformCount >= 0);
	m_iTransformCount -= 1;
	m_bIsTransformDirty = TTRUE;
}

void AGUI2RendererDX8::SetTransform(const AGUI2Transform& a_rTransform)
{
	m_pTransforms[m_iTransformCount] = a_rTransform;
	m_bIsTransformDirty = TTRUE;
}

void AGUI2RendererDX8::SetColour(TUINT32 a_uiColour)
{
	m_uiColour = a_uiColour;
}

void AGUI2RendererDX8::SetupViewport(TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4)
{
	TIMPLEMENT();
}

void AGUI2RendererDX8::SetupViewport()
{
	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	auto pD3DDevice = pRender->GetDirect3DDevice();

	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	D3DVIEWPORT8 viewport = {
		.X = 0,
		.Y = 0,
		.Width = pDisplayParams->uiWidth,
		.Height = pDisplayParams->uiHeight,
		.MinZ = 0.0f,
		.MaxZ = 1.0f
	};

	pD3DDevice->SetViewport(&viewport);

	static TUINT32 s_MatrixFlags = 0;
	static Toshi::TMatrix44 s_IdentityMatrix;

	if ((s_MatrixFlags & 1) == 0)
	{
		s_MatrixFlags |= 1;
		s_IdentityMatrix = TMatrix44::IDENTITY;
	}

	pD3DDevice->SetTransform(D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&s_IdentityMatrix);
	pD3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&s_IdentityMatrix);

	TMatrix44 projection;
	SetupProjectionMatrix(projection, viewport.X, viewport.Width + viewport.X, viewport.Y, viewport.Height + viewport.Y);
	pD3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&projection);
}

void AGUI2RendererDX8::RenderRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2)
{
	if (m_bIsTransformDirty)
	{
		UpdateTransform();
	}

	sm_Vertices[0].Position = { a.x, a.y, sm_fZCoordinate };
	sm_Vertices[0].Colour = m_uiColour;
	sm_Vertices[0].UV = { uv1.x, uv1.y };

	sm_Vertices[1].Position = { b.x, a.y, sm_fZCoordinate };
	sm_Vertices[1].Colour = m_uiColour;
	sm_Vertices[1].UV = { uv2.x, uv1.y };

	sm_Vertices[2].Position = { a.x, b.y, sm_fZCoordinate };
	sm_Vertices[2].Colour = m_uiColour;
	sm_Vertices[2].UV = { uv1.x, uv2.y };

	sm_Vertices[3].Position = { b.x, b.y, sm_fZCoordinate };
	sm_Vertices[3].Colour = m_uiColour;
	sm_Vertices[3].UV = { uv2.x, uv2.y };

	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	pRender->GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, sm_Vertices, sizeof(Vertex));
}

void AGUI2RendererDX8::RenderTriStrip(Toshi::TVector2* vertices, Toshi::TVector2* UV, uint32_t numverts)
{
	TIMPLEMENT();
}

void AGUI2RendererDX8::RenderLine(const Toshi::TVector2& a, const Toshi::TVector2& b)
{
	auto pMaterial = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsTransformDirty)
	{
		UpdateTransform();
	}

	sm_Vertices[0].Position = { a.x, a.y, 0.0f };
	sm_Vertices[0].Colour = m_uiColour;
	sm_Vertices[0].UV = { 0.0f, 0.0f };

	sm_Vertices[1].Position = { b.x, b.y, 0.0f };
	sm_Vertices[1].Colour = m_uiColour;
	sm_Vertices[1].UV = { 0.0f, 0.0f };

	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	pRender->GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, sm_Vertices, sizeof(Vertex));

	SetMaterial(pMaterial);
}

void AGUI2RendererDX8::RenderLine(float x1, float y1, float x2, float y2)
{
	auto pMaterial = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsTransformDirty)
	{
		UpdateTransform();
	}

	sm_Vertices[0].Position = { x1, y1, 0.0f };
	sm_Vertices[0].Colour = m_uiColour;
	sm_Vertices[0].UV = { 0.0f, 0.0f };

	sm_Vertices[1].Position = { x2, y2, 0.0f };
	sm_Vertices[1].Colour = m_uiColour;
	sm_Vertices[1].UV = { 0.0f, 0.0f };

	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	pRender->GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, sm_Vertices, sizeof(Vertex));

	SetMaterial(pMaterial);
}

void AGUI2RendererDX8::RenderOutlineRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b)
{
	
}

void AGUI2RendererDX8::RenderFilledRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b)
{
	
}

void AGUI2RendererDX8::ScaleCoords(float& x, float& y)
{
	
}

void AGUI2RendererDX8::ResetZCoordinate()
{
	sm_fZCoordinate = 0.01f;
}

void AGUI2RendererDX8::UpdateTransform()
{
	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	AGUI2Transform* pTransform = m_pTransforms + m_iTransformCount;

	TMatrix44 worldMatrix;
	worldMatrix.m_f11 = pTransform->m_Rotation[0].x;
	worldMatrix.m_f12 = pTransform->m_Rotation[0].y;
	worldMatrix.m_f13 = 0.0f;
	worldMatrix.m_f14 = 0.0f;

	worldMatrix.m_f21 = pTransform->m_Rotation[1].x;
	worldMatrix.m_f22 = pTransform->m_Rotation[1].y;
	worldMatrix.m_f23 = 0.0f;
	worldMatrix.m_f24 = 0.0f;

	worldMatrix.m_f31 = 0.0f;
	worldMatrix.m_f32 = 0.0f;
	worldMatrix.m_f33 = 1.0f;
	worldMatrix.m_f34 = 0.0f;

	worldMatrix.m_f41 = pTransform->m_Position.x;
	worldMatrix.m_f42 = pTransform->m_Position.y;
	worldMatrix.m_f43 = 0.0f;
	worldMatrix.m_f44 = 1.0f;

	pRender->GetDirect3DDevice()->SetTransform(D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&worldMatrix);
	m_bIsTransformDirty = TFALSE;
}

void AGUI2RendererDX8::SetupProjectionMatrix(TMatrix44& a_rOutMatrix, TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom)
{
	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	a_rOutMatrix = {
		2.0f / (a_iRight - a_iLeft), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (a_iBottom - a_iTop), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		((pDisplayParams->uiWidth - a_iLeft) - a_iRight) / (TFLOAT)(a_iRight - a_iLeft), ((pDisplayParams->uiHeight - a_iTop) - a_iBottom) / (TFLOAT)(a_iTop - a_iBottom), 0.0f, 1.0f
	};
}