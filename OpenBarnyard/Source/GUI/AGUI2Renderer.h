#pragma once
#include "AGUI2Material.h"
#include "AGUI2Transform.h"

#include <Math/TVector2.h>

class AGUI2Renderer
{
public:
	virtual ~AGUI2Renderer() = default;
	virtual AGUI2Material* CreateMaterial(const char* a_szTextureName) = 0;
	virtual AGUI2Material* CreateMaterial(Toshi::TTexture* a_pTexture) = 0;
	virtual void DestroyMaterial(AGUI2Material* a_pMaterial) = 0;
	virtual Toshi::TTexture* GetTexture(const char* a_szTextureName) = 0;
	virtual TUINT GetWidth(AGUI2Material* a_pMaterial) = 0;
	virtual TUINT GetHeight(AGUI2Material* a_pMaterial) = 0;
	virtual void BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void ResetRenderer() = 0;
	virtual void PrepareRenderer() = 0;
	virtual void SetMaterial(AGUI2Material* a_pMaterial) = 0;
	virtual void PushTransform(const AGUI2Transform& a_rTransform, const Toshi::TVector2& a_rVec1, const Toshi::TVector2& a_rVec2) = 0;
	virtual void PopTransform() = 0;
	virtual void SetTransform(const AGUI2Transform& a_rTransform) = 0;
	virtual void SetColour(TUINT32 a_uiColour) = 0;
	virtual void SetupViewport(TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4) = 0;
	virtual void SetupViewport() = 0;
	virtual void RenderRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2) = 0;
	virtual void RenderTriStrip(Toshi::TVector2* vertices, Toshi::TVector2* UV, uint32_t numverts) = 0;
	virtual void RenderLine(const Toshi::TVector2& a, const Toshi::TVector2& b) = 0;
	virtual void RenderLine(TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2) = 0;
	virtual void RenderOutlineRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b) = 0;
	virtual void RenderFilledRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b) = 0;
	virtual void ScaleCoords(TFLOAT& x, TFLOAT& y) = 0;
	virtual void ResetZCoordinate() = 0;
};
