#pragma once
#include "GUI/AGUI2Renderer.h"

#include <Math/TMatrix44.h>

class AGUI2RendererDX8 : public AGUI2Renderer
{
public:
	inline constexpr static TUINT32 MAX_NUM_TRANSFORMS = 32;
	inline constexpr static TUINT32 MAX_VERTICES = 8;

	struct Vertex
	{
		Toshi::TVector3 Position;
		TUINT32 Colour;
		Toshi::TVector2 UV;
	};

public:
	AGUI2RendererDX8();
	virtual ~AGUI2RendererDX8();

	virtual AGUI2Material* CreateMaterial(const TCHAR* a_szTextureName) override;
	virtual AGUI2Material* CreateMaterial(Toshi::TTexture* a_pTexture) override;
	virtual void DestroyMaterial(AGUI2Material* a_pMaterial) override;
	virtual Toshi::TTexture* GetTexture(const TCHAR* a_szTextureName) override;
	virtual TUINT GetWidth(AGUI2Material* a_pMaterial) override;
	virtual TUINT GetHeight(AGUI2Material* a_pMaterial) override;
	virtual void BeginScene() override;
	virtual void EndScene() override;
	virtual void ResetRenderer() override;
	virtual void PrepareRenderer() override;
	virtual void SetMaterial(AGUI2Material* a_pMaterial) override;
	virtual void PushTransform(const AGUI2Transform& a_rTransform, const Toshi::TVector2& a_rVec1, const Toshi::TVector2& a_rVec2) override;
	virtual void PopTransform() override;
	virtual void SetTransform(const AGUI2Transform& a_rTransform) override;
	virtual void SetColour(TUINT32 a_uiColour) override;
	virtual void SetupViewport(TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4) override;
	virtual void SetupViewport() override;
	virtual void RenderRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2) override;
	virtual void RenderTriStrip(Toshi::TVector2* vertices, Toshi::TVector2* UV, uint32_t numverts) override;
	virtual void RenderLine(const Toshi::TVector2& a, const Toshi::TVector2& b) override;
	virtual void RenderLine(TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2) override;
	virtual void RenderOutlineRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b) override;
	virtual void RenderFilledRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b) override;
	virtual void ScaleCoords(TFLOAT& x, TFLOAT& y) override;
	virtual void ResetZCoordinate() override;

	void UpdateTransform();

private:
	static void SetupProjectionMatrix(Toshi::TMatrix44& a_rOutMatrix, TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom);

public:
	inline static Vertex sm_Vertices[MAX_VERTICES];
	inline static TBOOL sm_bUnknownFlag = TFALSE;
	inline static TFLOAT sm_fZCoordinate = 0.1f;

private:
	AGUI2Transform* m_pTransforms;
	TINT m_iTransformCount;
	TUINT32 m_uiColour;
	TBOOL m_bIsTransformDirty;
	AGUI2Material* m_pMaterial; // ?
};
