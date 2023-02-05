#pragma once
#include "T2GUIElement.h"
#include "T2GUIMaterial.h"
#include "Toshi/Render/TTexture.h"

namespace Toshi
{
	class T2GUIRenderer
	{
	public:
		T2GUIRenderer() = default;
		virtual ~T2GUIRenderer() = default;

		virtual TTexture* GetTexture(const char* texName) const;
		virtual void BeginScene() = 0;
		virtual void EndScene() = 0;
		virtual void SetMaterial(T2GUIMaterial* pMat) = 0;
		virtual void PushTransform(const T2GUITransform& transform, const TVector2& vec1, const TVector2& vec2) = 0;
		virtual void PopTransform() = 0;
		virtual void SetTransform(const T2GUITransform& transform) = 0;
		virtual void SetColour(uint32_t colour) = 0;
		virtual void RenderRectangle(const TVector2& a, const TVector2& b, const TVector2& uv1, const TVector2& uv2) = 0;
		virtual void RenderTriStrip(TVector2* unk1, TVector2* unk2, uint32_t numverts, float unk4, float unk5) = 0;
		virtual void RenderLine(const TVector2& a, const TVector2& b) = 0;
		virtual void RenderLine(float x1, float y1, float x2, float y2) = 0;
		virtual void RenderOutlineRectangle(const TVector2& a, const TVector2& b) = 0;
		virtual void RenderFilledRectangle(const TVector2& a, const TVector2& b) = 0;
		virtual void RenderIndexedTriList() = 0; // unknown args, not implemented in de blob and globs
		virtual void RenderIndexedTriStripColoursList() = 0; // unknown args, not implemented in de blob and globs
		virtual void ScaleCoords(float& x, float& y) = 0;
		virtual void SetScissors(float f1, float f2, float f3, float f4) = 0;
		virtual void ClearScissors() = 0;
		virtual T2GUIMaterial* CreateMaterial(const char* texName) = 0;
		virtual T2GUIMaterial* CreateMaterial(TTexture* pTex) = 0;
		virtual void DestroyMaterial(T2GUIMaterial* pMat) = 0;
		virtual uint32_t GetWidth(T2GUIMaterial* pMat) = 0;
		virtual uint32_t GetHeight(T2GUIMaterial* pMat) = 0;

	private:
		int m_WhiteGlyphsCount;
	};
}