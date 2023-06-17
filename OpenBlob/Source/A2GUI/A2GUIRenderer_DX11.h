#pragma once
#include <Toshi2/T2GUI/T2GUIMaterial.h>
#include <Toshi2/T2GUI/T2GUIRenderer.h>

class A2GUIRenderer : public Toshi::T2GUIRenderer
{
public:
	static constexpr size_t TRANSFORMSTACKSIZE = 32;
	static constexpr size_t MAXVERTS = 128;

public:
	A2GUIRenderer();
	~A2GUIRenderer();

	virtual Toshi::TTexture* GetTexture(const char* texName) const override;
	virtual void BeginScene() override;
	virtual void EndScene() override;
	virtual void SetMaterial(Toshi::T2GUIMaterial* pMat) override;
	virtual void PushTransform(const Toshi::T2GUITransform& transform, const Toshi::TVector2& vec1, const Toshi::TVector2& vec2) override;
	virtual void PopTransform() override;
	virtual void SetTransform(const Toshi::T2GUITransform& transform) override;
	virtual void SetColour(uint32_t colour) override;
	virtual void RenderRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2) override;
	virtual void RenderTriStrip(Toshi::TVector2* vertices, Toshi::TVector2* UV, uint32_t numverts, float fPosScaleX, float fPosScaleY) override;
	virtual void RenderLine(const Toshi::TVector2& a, const Toshi::TVector2& b) override;
	virtual void RenderLine(float x1, float y1, float x2, float y2) override;
	virtual void RenderOutlineRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b) override;
	virtual void RenderFilledRectangle(const Toshi::TVector2& a, const Toshi::TVector2& b) override;
	virtual void RenderIndexedTriList(Toshi::TVector2* pVertices, Toshi::TVector2* pUV, void* pIndices, uint32_t numindices, int indexSize, uint32_t numverts, float fPosScaleX, float fPosScaleY) override;
	virtual void RenderIndexedTriStripColoursList(Toshi::TVector2* pVertices, Toshi::TVector2* pUV, uint32_t* pColors, void* pIndices, uint32_t numindices, int indexSize, uint32_t numverts, float fPosScaleX, float fPosScaleY) override;
	virtual void ScaleCoords(float& x, float& y) override;
	virtual void SetScissors(float f1, float f2, float f3, float f4) override;
	virtual void ClearScissors() override;
	virtual Toshi::T2GUIMaterial* CreateMaterial(const char* texName);
	virtual Toshi::T2GUIMaterial* CreateMaterial(Toshi::TTexture* pTex);
	virtual void DestroyMaterial(Toshi::T2GUIMaterial* pMat);
	virtual uint32_t GetWidth(Toshi::T2GUIMaterial* pMat);
	virtual uint32_t GetHeight(Toshi::T2GUIMaterial* pMat);

private:
	Toshi::T2GUITransform* m_pTransforms;
	size_t m_iTransformStackPointer;
	uint32_t m_ui32Colour;
	TBOOL m_bIsInScene;
	Toshi::T2GUIMaterial* m_pMaterial;
};