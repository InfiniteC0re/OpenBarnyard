#pragma once
#include "GUI/AGUI2Material.h"

#include <Math/TMatrix44.h>
#include <GUI/T2GUIRenderer.h>

class AGUI2RendererDX8
	: public Toshi::T2GUIRenderer
{
public:
	inline constexpr static TUINT32 MAX_NUM_TRANSFORMS = 32;
	inline constexpr static TUINT32 MAX_VERTICES       = 8;

	struct Vertex
	{
		Toshi::TVector3 Position;
		TUINT32         Colour;
		Toshi::TVector2 UV;
	};

public:
	AGUI2RendererDX8();
	virtual ~AGUI2RendererDX8();

	virtual Toshi::T2GUIMaterial* CreateMaterial( const TCHAR* a_szTextureName ) OVERRIDE;
	virtual Toshi::T2GUIMaterial* CreateMaterial( Toshi::TTexture* a_pTexture ) OVERRIDE;
	virtual void                  DestroyMaterial( Toshi::T2GUIMaterial* a_pMaterial ) OVERRIDE;
	virtual Toshi::TTexture*      GetTexture( const TCHAR* a_szTextureName ) OVERRIDE;
	virtual TUINT                 GetWidth( Toshi::T2GUIMaterial* a_pMaterial ) OVERRIDE;
	virtual TUINT                 GetHeight( Toshi::T2GUIMaterial* a_pMaterial ) OVERRIDE;
	virtual void                  BeginScene() OVERRIDE;
	virtual void                  EndScene() OVERRIDE;
	virtual void                  ResetRenderer() OVERRIDE;
	virtual void                  PrepareRenderer() OVERRIDE;
	virtual void                  SetMaterial( Toshi::T2GUIMaterial* a_pMaterial ) OVERRIDE;
	virtual void                  PushTransform( const Toshi::T2GUITransform& a_rTransform, const Toshi::TVector2& a_rVec1, const Toshi::TVector2& a_rVec2 ) OVERRIDE;
	virtual void                  PopTransform() OVERRIDE;
	virtual void                  SetTransform( const Toshi::T2GUITransform& a_rTransform ) OVERRIDE;
	virtual void                  SetColour( TUINT32 a_uiColour ) OVERRIDE;
	virtual void                  SetScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 ) OVERRIDE;
	virtual void                  ClearScissor() OVERRIDE;
	virtual void                  RenderRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2 ) OVERRIDE;
	virtual void                  RenderTriStrip( Toshi::TVector2* vertices, Toshi::TVector2* UV, uint32_t numverts ) OVERRIDE;
	virtual void                  RenderLine( const Toshi::TVector2& a, const Toshi::TVector2& b ) OVERRIDE;
	virtual void                  RenderLine( TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2 ) OVERRIDE;
	virtual void                  RenderOutlineRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b ) OVERRIDE;
	virtual void                  RenderFilledRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b ) OVERRIDE;
	virtual void                  ScaleCoords( TFLOAT& x, TFLOAT& y ) OVERRIDE;
	virtual void                  ResetZCoordinate() OVERRIDE;

	void UpdateTransform();

private:
	static void SetupProjectionMatrix( Toshi::TMatrix44& a_rOutMatrix, TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom );

public:
	inline static Vertex sm_Vertices[ MAX_VERTICES ];
	inline static TBOOL  sm_bUnknownFlag = TFALSE;
	inline static TFLOAT sm_fZCoordinate = 0.1f;

private:
	Toshi::T2GUITransform* m_pTransforms;
	TINT                   m_iTransformCount;
	TUINT32                m_uiColour;
	TBOOL                  m_bIsTransformDirty;
	AGUI2Material*         m_pMaterial;
};
