#include "ToshiPCH.h"
#include "T2GUI.h"
#include "T2GUIElement.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006c2b10
T2GUIElement::T2GUIElement()
{
	m_pParent          = TNULL;
	m_fWidth           = 0.0f;
	m_fHeight          = 0.0f;
	m_uiColour         = 0xFFFFFFFF;
	m_uiVisibilityMask = 0xFFFFFFFF;
	m_cbPostRender     = TNULL;
	m_ePivot           = T2GUIATTACHMENT_MIDDLECENTER;
	m_eAnchor          = T2GUIATTACHMENT_MIDDLECENTER;
	m_eFlags           = 3;
}

// $Barnyard: FUNCTION 006c22c0
void T2GUIElement::Tick( TFLOAT a_fDeltaTime )
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		it->Tick( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 006c2300
void T2GUIElement::PreRender()
{
	TVector2 vecAnchorPos = { 0.0f, 0.0f };
	TVector2 vecPivotPos  = { 0.0f, 0.0f };

	if ( m_pParent != TNULL )
	{
		TFLOAT fParentWidth, fParentHeight;
		m_pParent->GetDimensions( fParentWidth, fParentHeight );
		AnchorPos( vecAnchorPos.x, vecAnchorPos.y, fParentWidth, fParentHeight );
	}

	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	PivotPos( vecPivotPos.x, vecPivotPos.y, fWidth, fHeight );

	T2GUI::GetRenderer()->PushTransform( m_oTransform, vecAnchorPos, vecPivotPos );
}

// $Barnyard: FUNCTION 006c2510
void T2GUIElement::Render()
{
#if 0
	{
		TFLOAT fWidth;
		TFLOAT fHeight;
		GetDimensions(fWidth, fHeight);
		TVector2 pos1 = { -fWidth / 2, -fHeight / 2 };
		TVector2 pos2 = { pos1.x + fWidth - 1, pos1.y + fHeight - 1 };

		AGUI2::GetRenderer()->RenderOutlineRectangle(pos1, pos2);
	}
#endif

	for ( T2GUIElement* it = ChildRBegin(); it != TNULL && it != &m_Children; it = GetPrevChild( it ) )
	{
		if ( it->IsVisible() )
		{
			it->PreRender();
			it->Render();
			it->PostRender();
		}
	}

	if ( m_cbPostRender )
	{
		m_cbPostRender();
	}
}

// $Barnyard: FUNCTION 006c2570
void T2GUIElement::PostRender()
{
	T2GUI::GetRenderer()->PopTransform();
}

// $Barnyard: FUNCTION 006c25d0
void T2GUIElement::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	a_rWidth  = m_fWidth;
	a_rHeight = m_fHeight;
}

// $Barnyard: FUNCTION 005748f0
TFLOAT T2GUIElement::GetWidth()
{
	return m_fWidth;
}

// $Barnyard: FUNCTION 00577290
TFLOAT T2GUIElement::GetHeight()
{
	return m_fHeight;
}

// $Barnyard: FUNCTION 0043ce00
void T2GUIElement::SetWidth( TFLOAT a_fWidth )
{
	m_fWidth = a_fWidth;
}

// $Barnyard: FUNCTION 0043ce10
void T2GUIElement::SetHeight( TFLOAT a_fHeight )
{
	m_fHeight = a_fHeight;
}

// $Barnyard: FUNCTION 006c25f0
void T2GUIElement::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_fWidth  = a_fWidth;
	m_fHeight = a_fHeight;
}

// $Barnyard: FUNCTION 006c2610
void T2GUIElement::GetMins( TFLOAT& a_fX, TFLOAT& a_fY )
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	a_fX = fWidth * -0.5f;
	a_fY = fHeight * -0.5f;
}

// $Barnyard: FUNCTION 006c2650
void T2GUIElement::GetMaxs( TFLOAT& a_fX, TFLOAT& a_fY )
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	a_fX = fWidth * 0.5f;
	a_fY = fHeight * 0.5f;
}

// $Barnyard: FUNCTION 0043cdf0
void T2GUIElement::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

// $Barnyard: FUNCTION 006c2ad0
void T2GUIElement::SetAlpha( TFLOAT a_fAlpha )
{
	TUINT8 alpha8 = (TUINT8)( a_fAlpha * 255.0f );
	m_uiColour    = ( m_uiColour & 0x00FFFFFF ) | alpha8 << 24;
}

void T2GUIElement::SetShadowAlpha( TFLOAT a_fAlpha )
{
}

// $Barnyard: FUNCTION 006c2a80
void T2GUIElement::SetFocus( TBOOL a_bFocused )
{
	if ( a_bFocused )
	{
		m_eFlags |= 2;
	}
	else
	{
		m_eFlags &= ~2;
	}
}

// $Barnyard: FUNCTION 006c2a00
TBOOL T2GUIElement::IsPointInside( TFLOAT a_fX, TFLOAT a_fY )
{
	TFLOAT minX, minY;
	TFLOAT maxX, maxY;

	GetMins( minX, minY );
	GetMaxs( maxX, maxY );

	return ( minX <= a_fX && a_fX <= maxX ) && ( minY <= a_fY && a_fY <= maxY );
}

void T2GUIElement::AnchorPos( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	switch ( m_eAnchor )
	{
		case T2GUIATTACHMENT_TOPLEFT:
			a_rX -= a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_TOPCENTER:
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_TOPRIGHT:
			a_rX += a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_MIDDLELEFT:
			a_rX -= a_fWidth * 0.5f;
			break;
		case T2GUIATTACHMENT_MIDDLERIGHT:
			a_rX += a_fWidth * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMLEFT:
			a_rX -= a_fWidth * 0.5f;
			a_rY += a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMCENTER:
			a_rY += a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMRIGHT:
			a_rX += a_fWidth * 0.5f;
			a_rY += a_fHeight * 0.5f;
			break;
	}
}

void T2GUIElement::PivotPos( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	switch ( m_ePivot )
	{
		case T2GUIATTACHMENT_TOPLEFT:
			a_rX += a_fWidth * 0.5f;
			a_rY += a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_TOPCENTER:
			a_rY += a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_TOPRIGHT:
			a_rX -= a_fWidth * 0.5f;
			a_rY += a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_MIDDLELEFT:
			a_rX += a_fWidth * 0.5f;
			break;
		case T2GUIATTACHMENT_MIDDLERIGHT:
			a_rX -= a_fWidth * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMLEFT:
			a_rX += a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMCENTER:
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMRIGHT:
			a_rX -= a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
	}
}

// $Barnyard: FUNCTION 006c2690
void T2GUIElement::GetScreenTransform( T2GUITransform& a_rOutTransform )
{
	TVector2 vecAnchorPos = { 0.0f, 0.0f };
	TVector2 vecPivotPos  = { 0.0f, 0.0f };

	if ( m_pParent != TNULL )
	{
		TFLOAT fParentWidth, fParentHeight;
		m_pParent->GetDimensions( fParentWidth, fParentHeight );
		AnchorPos( vecAnchorPos.x, vecAnchorPos.y, fParentWidth, fParentHeight );
	}

	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	PivotPos( vecPivotPos.x, vecPivotPos.y, fWidth, fHeight );

	T2GUITransform transform1;
	T2GUITransform transform2;

	if ( m_pParent )
	{
		m_pParent->GetScreenTransform( transform1 );
		transform2 = GetTransform();
	}
	else
	{
		auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

		transform1.m_aMatrixRows[ 0 ] = { 1.0f, 0.0f };
		transform1.m_aMatrixRows[ 1 ] = { 0.0f, 1.0f };
		transform1.m_vecTranslation   = { pDisplayParams->uiWidth / 2.0f, pDisplayParams->uiHeight / 2.0f };

		transform1.Scale( pDisplayParams->uiWidth / fWidth, pDisplayParams->uiHeight / fHeight );
		transform2 = GetTransform();
	}

	TVector2 vec;
	transform1.Transform( vec, vecAnchorPos );
	transform1.m_vecTranslation = { vec.x, vec.y };

	transform2.Transform( vec, vecPivotPos );
	transform2.m_vecTranslation = { vec.x, vec.y };

	T2GUITransform::Multiply( a_rOutTransform, transform1, transform2 );
}

// $Barnyard: FUNCTION 006c29d0
void T2GUIElement::GetInvScreenTransform( T2GUITransform& a_rOutTransform )
{
	T2GUITransform transform;
	GetScreenTransform( transform );
	transform.GetInverse( a_rOutTransform );
}

TOSHI_NAMESPACE_END
