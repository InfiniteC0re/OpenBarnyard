#include "pch.h"
#include "SDKHooks.h"
#include "SDK_T2GUIElement.h"

#include <Render/TRenderInterface.h>

SDK_T2GUIElement::SDK_T2GUIElement()
{
}

void SDK_T2GUIElement::Tick( TFLOAT a_fDeltaTime )
{
	CALL_THIS( 0x006c22c0, SDK_T2GUIElement*, SDK_T2GUIElement*, this, TFLOAT, a_fDeltaTime );
}

void SDK_T2GUIElement::PreRender()
{
	CALL_THIS( 0x006c2300, SDK_T2GUIElement*, SDK_T2GUIElement*, this );
}

void SDK_T2GUIElement::Render()
{
	CALL_THIS( 0x006c2510, SDK_T2GUIElement*, SDK_T2GUIElement*, this );
}

void SDK_T2GUIElement::PostRender()
{
	CALL_THIS( 0x006c2570, SDK_T2GUIElement*, SDK_T2GUIElement*, this );
}

void SDK_T2GUIElement::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	a_rWidth  = m_fWidth;
	a_rHeight = m_fHeight;
}

TFLOAT SDK_T2GUIElement::GetWidth()
{
	return m_fWidth;
}

TFLOAT SDK_T2GUIElement::GetHeight()
{
	return m_fHeight;
}

void SDK_T2GUIElement::SetWidth( TFLOAT a_fWidth )
{
	m_fWidth = a_fWidth;
}

void SDK_T2GUIElement::SetHeight( TFLOAT a_fHeight )
{
	m_fHeight = a_fHeight;
}

void SDK_T2GUIElement::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_fWidth  = a_fWidth;
	m_fHeight = a_fHeight;
}

void SDK_T2GUIElement::GetMins( TFLOAT& a_fX, TFLOAT& a_fY )
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	a_fX = fWidth * -0.5f;
	a_fY = fHeight * -0.5f;
}

void SDK_T2GUIElement::GetMaxs( TFLOAT& a_fX, TFLOAT& a_fY )
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	a_fX = fWidth * 0.5f;
	a_fY = fHeight * 0.5f;
}

void SDK_T2GUIElement::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

void SDK_T2GUIElement::SetAlpha( TFLOAT a_fAlpha )
{
	TUINT8 alpha8 = (TUINT8)( a_fAlpha * 255.0f );
	m_uiColour    = ( m_uiColour & 0x00FFFFFF ) | alpha8 << 24;
}

void SDK_T2GUIElement::SetShadowAlpha( TFLOAT a_fAlpha )
{
}

void SDK_T2GUIElement::SetFocus( TBOOL a_bFocused )
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

TBOOL SDK_T2GUIElement::IsPointInside( const Toshi::TVector2& a_rPoint )
{
	TFLOAT minX, minY;
	TFLOAT maxX, maxY;

	GetMins( minX, minY );
	GetMaxs( maxX, maxY );

	if ( minX <= a_rPoint.x && a_rPoint.x <= maxX && minY <= a_rPoint.y )
	{
		return a_rPoint.y <= maxY;
	}

	return TFALSE;
}

void SDK_T2GUIElement::GetScreenTransform( Toshi::T2GUITransform& a_rOutTransform )
{
	CALL_THIS( 0x006c2690, SDK_T2GUIElement*, SDK_T2GUIElement*, this, Toshi::T2GUITransform&, a_rOutTransform );
}

void SDK_T2GUIElement::GetInvScreenTransform( Toshi::T2GUITransform& a_rOutTransform )
{
	CALL_THIS( 0x006c29d0, SDK_T2GUIElement*, SDK_T2GUIElement*, this, Toshi::T2GUITransform&, a_rOutTransform );
}

SDK_T2GUIElement* SDK_T2GUIElement::CreateFromEngine()
{
	return CALL_THIS( 0x006c2b10, SDK_T2GUIElement*, SDK_T2GUIElement*, new SDK_T2GUIElement );
}
