#pragma once
#include "T2GUIMaterial.h"
#include "T2GUITransform.h"

TOSHI_NAMESPACE_START

class T2GUIRenderer
{
public:
	T2GUIRenderer()          = default;
	virtual ~T2GUIRenderer() = default;

	virtual T2GUIMaterial* CreateMaterial( const TCHAR* a_szTextureName )                                                        = 0;
	virtual T2GUIMaterial* CreateMaterial( TTexture* a_pTexture )                                                                = 0;
	virtual void           DestroyMaterial( T2GUIMaterial* a_pMaterial )                                                         = 0;
	virtual TTexture*      GetTexture( const TCHAR* a_szTextureName )                                                            = 0;
	virtual TUINT          GetWidth( T2GUIMaterial* a_pMaterial )                                                                = 0;
	virtual TUINT          GetHeight( T2GUIMaterial* a_pMaterial )                                                               = 0;
	virtual void           BeginScene()                                                                                          = 0;
	virtual void           EndScene()                                                                                            = 0;
	virtual void           ResetRenderer()                                                                                       = 0;
	virtual void           PrepareRenderer()                                                                                     = 0;
	virtual void           SetMaterial( T2GUIMaterial* a_pMaterial )                                                             = 0;
	virtual void           PushTransform( const T2GUITransform& a_rTransform, const TVector2& a_rVec1, const TVector2& a_rVec2 ) = 0;
	virtual void           PopTransform()                                                                                        = 0;
	virtual void           SetTransform( const T2GUITransform& a_rTransform )                                                    = 0;
	virtual void           SetColour( TUINT32 a_uiColour )                                                                       = 0;
	virtual void           SetScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 )                          = 0;
	virtual void           ClearScissor()                                                                                        = 0;
	virtual void           RenderRectangle( const TVector2& a, const TVector2& b, const TVector2& uv1, const TVector2& uv2 )     = 0;
	virtual void           RenderTriStrip( TVector2* vertices, TVector2* UV, TUINT32 numverts )                                 = 0;
	virtual void           RenderLine( const TVector2& a, const TVector2& b )                                                    = 0;
	virtual void           RenderLine( TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2 )                                              = 0;
	virtual void           RenderOutlineRectangle( const TVector2& a, const TVector2& b )                                        = 0;
	virtual void           RenderFilledRectangle( const TVector2& a, const TVector2& b )                                         = 0;
	virtual void           ScaleCoords( TFLOAT& x, TFLOAT& y )                                                                   = 0;
	virtual void           ResetZCoordinate()                                                                                    = 0;
};

TOSHI_NAMESPACE_END
