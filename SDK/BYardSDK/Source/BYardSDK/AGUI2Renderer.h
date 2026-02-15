#pragma once
#include "SDK_T2GUIMaterial.h"

#include <Math/TVector2.h>
#include <GUI/T2GUITransform.h>

class SDK_T2GUIRenderer
{
public:
	SDK_T2GUIRenderer()          = default;
	virtual ~SDK_T2GUIRenderer() = default;

	virtual SDK_T2GUIMaterial* CreateMaterialFromName( const TCHAR* a_szTextureName )                                                                        = 0;
	virtual SDK_T2GUIMaterial* CreateMaterial( Toshi::TTexture* a_pTexture )                                                                                 = 0;
	virtual void               DestroyMaterial( SDK_T2GUIMaterial* a_pMaterial )                                                                             = 0;
	virtual Toshi::TTexture*   GetTexture( const TCHAR* a_szTextureName )                                                                                    = 0;
	virtual TUINT              GetWidth( SDK_T2GUIMaterial* a_pMaterial )                                                                                    = 0;
	virtual TUINT              GetHeight( SDK_T2GUIMaterial* a_pMaterial )                                                                                   = 0;
	virtual void               BeginScene()                                                                                                                  = 0;
	virtual void               EndScene()                                                                                                                    = 0;
	virtual void               ResetRenderer()                                                                                                               = 0;
	virtual void               PrepareRenderer()                                                                                                             = 0;
	virtual void               SetMaterial( SDK_T2GUIMaterial* a_pMaterial )                                                                                 = 0;
	virtual void               PushTransform( const Toshi::T2GUITransform& a_rTransform, const Toshi::TVector2& a_rVec1, const Toshi::TVector2& a_rVec2 )    = 0;
	virtual void               PopTransform()                                                                                                                = 0;
	virtual void               SetTransform( const Toshi::T2GUITransform& a_rTransform )                                                                     = 0;
	virtual void               SetColour( TUINT32 a_uiColour )                                                                                               = 0;
	virtual void               SetScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 )                                                  = 0;
	virtual void               ClearScissor()                                                                                                                = 0;
	virtual void               RenderRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2 ) = 0;
	virtual void               RenderTriStrip( Toshi::TVector2* vertices, Toshi::TVector2* UV, uint32_t numverts )                                           = 0;
	virtual void               RenderLine( const Toshi::TVector2& a, const Toshi::TVector2& b )                                                              = 0;
	virtual void               RenderLine( TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2 )                                                                      = 0;
	virtual void               RenderOutlineRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b )                                                  = 0;
	virtual void               RenderFilledRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b )                                                   = 0;
	virtual void               ScaleCoords( TFLOAT& x, TFLOAT& y )                                                                                           = 0;
	virtual void               ResetZCoordinate()                                                                                                            = 0;
};
