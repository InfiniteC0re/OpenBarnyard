#pragma once
#include "Toshi/Render/TRender.h"
#include "Toshi2/T2GUI/T2GUITransform.h"
#include "Toshi2/T2GUI/T2GUIRenderer.h"
#include "Platform/SDL/T2GUI/T2GUIMaterial_SDL.h"
#include "Platform/SDL/TRender_SDL.h"
#include "Platform/SDL/TPrimShader_SDL.h"

namespace Toshi {

	class TXUIRenderer : public T2GUIRenderer
	{
	public:
		static constexpr size_t TRANSFORMSTACKSIZE = 32;
		static constexpr size_t MAXVERTS = 1024;

	public:
		TXUIRenderer();
		~TXUIRenderer();

		virtual void BeginScene() override;
		virtual void EndScene() override;
		virtual void SetMaterial(T2GUIMaterial* pMat) override;
		virtual void PushTransform(const T2GUITransform& transform, const TVector2& vec1, const TVector2& vec2) override;
		virtual void PopTransform() override;
		virtual void SetTransform(const T2GUITransform& transform) override;
		virtual void SetColour(uint32_t colour) override;
		virtual void RenderRectangle(const TVector2& a, const TVector2& b, const TVector2& uv1, const TVector2& uv2) override;
		virtual void RenderTriStrip(TVector2* vertices, TVector2* UV, uint32_t numverts, float fPosScaleX, float fPosScaleY) override;
		virtual void RenderLine(const TVector2& a, const TVector2& b) override;
		virtual void RenderLine(float x1, float y1, float x2, float y2) override;
		virtual void RenderOutlineRectangle(const TVector2& a, const TVector2& b) override;
		virtual void RenderFilledRectangle(const TVector2& a, const TVector2& b) override;
		virtual void RenderIndexedTriList(TVector2* pVertices, TVector2* pUV, void* pIndices, uint32_t numindices, int indexSize, uint32_t numverts, float fPosScaleX, float fPosScaleY) override;
		virtual void RenderIndexedTriStripColoursList(TVector2* pVertices, TVector2* pUV, uint32_t* pColors, void* pIndices, uint32_t numindices, int indexSize, uint32_t numverts, float fPosScaleX, float fPosScaleY) override;
		virtual void ScaleCoords(float& x, float& y) override;
		virtual void SetScissors(float f1, float f2, float f3, float f4) override;
		virtual void ClearScissors() override;
		virtual uint32_t GetWidth(T2GUIMaterial* pMat);
		virtual uint32_t GetHeight(T2GUIMaterial* pMat);
		virtual void RenderTriStripColours(TVector2* pVertices, TVector2* pUV, uint32_t* pColors, size_t numverts, float fPosScaleX, float fPosScaleY);

	private:
		inline static TPrimShader::Vertex s_Vertices[MAXVERTS];

	private:
		T2GUITransform* m_pTransforms;
		int m_iTransformStackPointer;
		uint32_t m_ui32Colour;
		TBOOL m_bIsInScene;
		T2GUIMaterial* m_pMaterial;
		//TRenderDX11::DepthState m_OldDepthState;
	};

}