#include "pch.h"
#include "A2GUIRenderer_DX11.h"
#include "Memory/AMemory.h"

#include <Platform/Windows/DX11/TRender_DX11.h>
#include <Platform/Windows/DX11/TRenderContext_DX11.h>
#include <Platform/Windows/DX11/TPrimShader_DX11.h>
#include <Platform/Windows/DX11/TTexture_DX11.h>
#include <Toshi2/T2GUI/T2GUI.h>

using namespace Toshi;

A2GUIRenderer::A2GUIRenderer()
{
	m_pTransforms = TNULL;
	m_iTransformStackPointer = 0;
	m_pMaterial = TNULL;
	m_bIsInScene = TFALSE;
	m_pTransforms = new T2GUITransform[TRANSFORMSTACKSIZE];
}

A2GUIRenderer::~A2GUIRenderer()
{
	delete[] m_pTransforms;
}

TTexture* A2GUIRenderer::GetTexture(const char* texName) const
{
	char lowercaseTexName[512];

	TASSERT(T2String8::Length(texName) < sizeof(lowercaseTexName));
	TStringManager::String8CopySafe(lowercaseTexName, texName, sizeof(lowercaseTexName));
	TStringManager::String8ToLowerCase(lowercaseTexName);
	TTODO("Calculate string offset");

	return TTextureManager::GetSingletonWeak()->FindTexture(lowercaseTexName);
}

void A2GUIRenderer::BeginScene()
{
	TASSERT(m_iTransformStackPointer == 0);

	auto pRender = TRenderDX11::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();
	auto pRenderContext = (TRenderContextDX11*)pRender->GetCurrentRenderContext();
	pRender->FUN_00691190();

	static bool s_IsMatrixSet = false;
	static Toshi::TMatrix44 s_IdentityMatrix;

	if (!s_IsMatrixSet)
	{
		s_IsMatrixSet = true;
		s_IdentityMatrix.Identity();
	}

	pRenderContext->SetModelViewMatrix(s_IdentityMatrix);
	pRenderContext->SetWorldViewMatrix(s_IdentityMatrix);

	Toshi::TRenderContext::PROJECTIONPARAMS projParams;
	projParams.m_Proj.x = 1.0f;
	projParams.m_Proj.y = -1.0f;
	projParams.m_Centre.x = pDisplayParams->Width * 0.5f;
	projParams.m_Centre.y = pDisplayParams->Height * 0.5f;
	projParams.m_fNearClip = 0.0f;
	projParams.m_fFarClip = 1.0f;

	pRenderContext->SetCameraMode(TRenderContext::CameraMode_Orthographic);
	pRenderContext->SetProjectionParams(projParams);
	pRenderContext->Update();
	SetColour(0xFFFFFFFF);
}

void A2GUIRenderer::EndScene()
{
	TASSERT(m_iTransformStackPointer == 0);
}

void A2GUIRenderer::SetMaterial(T2GUIMaterial* pMat)
{
	auto pPrimShader = TPrimShader::GetSingletonWeak();
	TTexture* pTex = TNULL;

	if (pMat != TNULL)
	{
		TASSERT(pMat->IsA(TGetClass(T2GUIMaterial)));
		pTex = pMat->GetTexture();
	}
	
	pPrimShader->SetImageAndUnlock(pTex);
	m_pMaterial = pMat;
}

void A2GUIRenderer::PushTransform(const T2GUITransform& transform, const TVector2& vec1, const TVector2& vec2)
{
	T2GUITransform transform1;
	T2GUITransform transform2;

	m_iTransformStackPointer += 1;
	TASSERT(m_iTransformStackPointer < TRANSFORMSTACKSIZE);

	T2GUITransform& currTransform = *(m_pTransforms + m_iTransformStackPointer);
	T2GUITransform& prevTransform = *(&currTransform - 1);
	
	transform1 = transform;
	transform2 = prevTransform;
	transform2.Transform(transform2.GetPos(), vec1);
	transform1.Transform(transform1.GetPos(), vec2);
	T2GUITransform::Multiply(currTransform, transform2, transform1);
	m_bIsInScene = TTRUE;
}

void A2GUIRenderer::PopTransform()
{
	TASSERT(m_iTransformStackPointer > 0);
	m_iTransformStackPointer -= 1;
	m_bIsInScene = TTRUE;
}

void A2GUIRenderer::SetTransform(const T2GUITransform& transform)
{
	T2GUITransform& currTransform = *(m_pTransforms + m_iTransformStackPointer);
	currTransform = transform;
	m_bIsInScene = TTRUE;
}

void A2GUIRenderer::SetColour(uint32_t colour)
{
	m_ui32Colour = colour;
}

void A2GUIRenderer::RenderRectangle(const TVector2& a, const TVector2& b, const TVector2& uv1, const TVector2& uv2)
{
	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingletonWeak()->GetCurrentRenderContext();
		auto pTransform = m_pTransforms + m_iTransformStackPointer;
		
		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);
		
		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto pPrimShader = TPrimShader::GetSingletonWeak();
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = uv1;
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { uv2.x, uv1.y };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { b.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { uv1.x, uv2.y };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { a.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { uv2.x, uv2.y };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { b.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
}

void A2GUIRenderer::RenderTriStrip(Toshi::TVector2* unk1, Toshi::TVector2* unk2, uint32_t numverts, float unk4, float unk5)
{
	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingletonWeak()->GetCurrentRenderContext();
		auto pTransform = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TASSERT(numverts < MAXVERTS);

	TPrimShader::Vertex* pVertex;
	auto pPrimShader = TPrimShader::GetSingletonWeak();
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	for (size_t i = 0; i < numverts; i++)
	{
		pVertex = pPrimShader->GetCurrentVertex();
		TVector2 dif = unk2[i] - unk1[i];
		pVertex->UV = { unk2[i].x, dif.y};
		pVertex->Color = m_ui32Colour;
		pVertex->Position = { dif.x, dif.y, 0.0f };
		pPrimShader->AddVert();
	}
	pPrimShader->StopRendering();
}

void A2GUIRenderer::RenderLine(const TVector2& a, const TVector2& b)
{
	auto pOldMat = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingletonWeak()->GetCurrentRenderContext();
		auto pTransform = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto pPrimShader = TPrimShader::GetSingletonWeak();
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { b.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial(pOldMat);
}

void A2GUIRenderer::RenderLine(float x1, float y1, float x2, float y2)
{
	auto pOldMat = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingletonWeak()->GetCurrentRenderContext();
		auto pTransform = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto pPrimShader = TPrimShader::GetSingletonWeak();
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { x1, y1, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { x1, y2, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial(pOldMat);
}

void A2GUIRenderer::RenderOutlineRectangle(const TVector2& a, const TVector2& b)
{
	auto pOldMat = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingletonWeak()->GetCurrentRenderContext();
		auto pTransform = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto pPrimShader = TPrimShader::GetSingletonWeak();
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { b.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { a.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial(pOldMat);
}

void A2GUIRenderer::RenderFilledRectangle(const TVector2& a, const TVector2& b)
{
	auto pOldMat = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingletonWeak()->GetCurrentRenderContext();
		auto pTransform = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto pPrimShader = TPrimShader::GetSingletonWeak();
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { b.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { a.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pVertex = pPrimShader->GetCurrentVertex();
	pVertex->UV = { 0, 0 };
	pVertex->Color = m_ui32Colour;
	pVertex->Position = { b.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial(pOldMat);
}

void A2GUIRenderer::RenderIndexedTriList()
{
	TASSERT(TFALSE, "Not implemented in the original engine");
}

void A2GUIRenderer::RenderIndexedTriStripColoursList()
{
	TASSERT(TFALSE, "Not implemented in the original engine");
}

void A2GUIRenderer::ScaleCoords(float& x, float& y)
{
	auto pDisplayParams = TRenderDX11::Interface()->GetCurrentDisplayParams();
	T2GUIElement* pRoot = T2GUI::GetSingletonWeak()->GetRootElement();

	float fRootWidth, fRootHeight;
	pRoot->GetDimensions(fRootWidth, fRootHeight);

	x /= pDisplayParams->Width / fRootWidth;
	y /= pDisplayParams->Height / fRootHeight;
}

void A2GUIRenderer::SetScissors(float f1, float f2, float f3, float f4)
{
	TIMPLEMENT();
}

void A2GUIRenderer::ClearScissors()
{
	TIMPLEMENT();
}

T2GUIMaterial* A2GUIRenderer::CreateMaterial(const char* texName)
{
	return CreateMaterial(GetTexture(texName));
}

T2GUIMaterial* A2GUIRenderer::CreateMaterial(TTexture* pTex)
{
	T2GUIMaterial* pMaterial = new (AMemory::GetPool(AMemory::POOL_FrequentAllocations)) T2GUIMaterial;
	pMaterial->Create();
	pMaterial->SetTexture(pTex);
	
	return pMaterial;
}

void A2GUIRenderer::DestroyMaterial(T2GUIMaterial* pMat)
{
	if (pMat != TNULL)
	{
		TASSERT(pMat->IsA(TGetClass(T2GUIMaterial)));
		pMat->OnDestroy();
		pMat->Delete();
	}
}

uint32_t A2GUIRenderer::GetWidth(T2GUIMaterial* pMat)
{
	TASSERT(pMat->IsA(TGetClass(T2GUIMaterial)));
	TTexture* pTex = pMat->GetTexture();

	if (pTex != TNULL)
	{
		return pTex->GetWidth();
	}

	return 0;
}

uint32_t A2GUIRenderer::GetHeight(T2GUIMaterial* pMat)
{
	TASSERT(pMat->IsA(TGetClass(T2GUIMaterial)));
	TTexture* pTex = pMat->GetTexture();

	if (pTex != TNULL)
	{
		return pTex->GetHeight();
	}

	return 0;
}
