#include "pch.h"
#include "AGUI2.h"
#include "AGUI2Element.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

AGUI2Element::AGUI2Element()
{
	m_pParent          = TNULL;
	m_fWidth           = 0.0f;
	m_fHeight          = 0.0f;
	m_uiColour         = 0xFFFFFFFF;
	m_uiVisibilityMask = 0xFFFFFFFF;
	m_cbPostRender     = TNULL;
	m_ePivot           = AGUI2ATTACHMENT_MIDDLECENTER;
	m_eAnchor          = AGUI2ATTACHMENT_MIDDLECENTER;
	m_eFlags           = 3;
}

void AGUI2Element::Tick( TFLOAT a_fDeltaTime )
{
	for ( AGUI2Element* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		it->Tick( a_fDeltaTime );
	}
}

void AGUI2Element::PreRender()
{
	Toshi::TVector2 vecAnchorPos = { 0.0f, 0.0f };
	Toshi::TVector2 vecPivotPos  = { 0.0f, 0.0f };

	if ( m_pParent != TNULL )
	{
		TFLOAT fParentWidth, fParentHeight;
		m_pParent->GetDimensions( fParentWidth, fParentHeight );
		AnchorPos( vecAnchorPos.x, vecAnchorPos.y, fParentWidth, fParentHeight );
	}

	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	PivotPos( vecPivotPos.x, vecPivotPos.y, fWidth, fHeight );

	AGUI2::GetRenderer()->PushTransform( m_oTransform, vecAnchorPos, vecPivotPos );
}

void AGUI2Element::Render()
{
#if 0
	{
		TFLOAT fWidth;
		TFLOAT fHeight;
		GetDimensions(fWidth, fHeight);
		Toshi::TVector2 pos1 = { -fWidth / 2, -fHeight / 2 };
		Toshi::TVector2 pos2 = { pos1.x + fWidth - 1, pos1.y + fHeight - 1 };

		AGUI2::GetRenderer()->RenderOutlineRectangle(pos1, pos2);
	}
#endif

	for ( AGUI2Element* it = ChildRBegin(); it != TNULL && it != &m_Children; it = GetPrevChild( it ) )
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

void AGUI2Element::PostRender()
{
	AGUI2::GetRenderer()->PopTransform();
}

void AGUI2Element::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	a_rWidth  = m_fWidth;
	a_rHeight = m_fHeight;
}

TFLOAT AGUI2Element::GetWidth()
{
	return m_fWidth;
}

TFLOAT AGUI2Element::GetHeight()
{
	return m_fHeight;
}

void AGUI2Element::SetWidth( TFLOAT a_fWidth )
{
	m_fWidth = a_fWidth;
}

void AGUI2Element::SetHeight( TFLOAT a_fHeight )
{
	m_fHeight = a_fHeight;
}

void AGUI2Element::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_fWidth  = a_fWidth;
	m_fHeight = a_fHeight;
}

void AGUI2Element::GetMins( TFLOAT& a_fX, TFLOAT& a_fY )
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	a_fX = fWidth * -0.5f;
	a_fY = fHeight * -0.5f;
}

void AGUI2Element::GetMaxs( TFLOAT& a_fX, TFLOAT& a_fY )
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	a_fX = fWidth * 0.5f;
	a_fY = fHeight * 0.5f;
}

void AGUI2Element::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

void AGUI2Element::SetAlpha( TFLOAT a_fAlpha )
{
	TUINT8 alpha8 = (TUINT8)( a_fAlpha * 255.0f );
	m_uiColour    = ( m_uiColour & 0x00FFFFFF ) | alpha8 << 24;
}

void AGUI2Element::SetShadowAlpha( TFLOAT a_fAlpha )
{
}

void AGUI2Element::SetFocus( TBOOL a_bFocused )
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

TBOOL AGUI2Element::IsPointInside( TFLOAT a_fX, TFLOAT a_fY )
{
	TFLOAT minX, minY;
	TFLOAT maxX, maxY;

	GetMins( minX, minY );
	GetMaxs( maxX, maxY );

	return ( minX <= a_fX && a_fX <= maxX ) && ( minY <= a_fY && a_fY <= maxY );
}

void AGUI2Element::AnchorPos( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	switch ( m_eAnchor )
	{
		case AGUI2ATTACHMENT_TOPLEFT:
			a_rX -= a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_TOPCENTER:
			a_rY -= a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_TOPRIGHT:
			a_rX += a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_MIDDLELEFT:
			a_rX -= a_fWidth * 0.5f;
			break;
		case AGUI2ATTACHMENT_MIDDLERIGHT:
			a_rX += a_fWidth * 0.5f;
			break;
		case AGUI2ATTACHMENT_BOTTOMLEFT:
			a_rX -= a_fWidth * 0.5f;
			a_rY += a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_BOTTOMCENTER:
			a_rY += a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_BOTTOMRIGHT:
			a_rX += a_fWidth * 0.5f;
			a_rY += a_fHeight * 0.5f;
			break;
	}
}

void AGUI2Element::PivotPos( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	switch ( m_ePivot )
	{
		case AGUI2ATTACHMENT_TOPLEFT:
			a_rX += a_fWidth * 0.5f;
			a_rY += a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_TOPCENTER:
			a_rY += a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_TOPRIGHT:
			a_rX -= a_fWidth * 0.5f;
			a_rY += a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_MIDDLELEFT:
			a_rX += a_fWidth * 0.5f;
			break;
		case AGUI2ATTACHMENT_MIDDLERIGHT:
			a_rX -= a_fWidth * 0.5f;
			break;
		case AGUI2ATTACHMENT_BOTTOMLEFT:
			a_rX += a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_BOTTOMCENTER:
			a_rY -= a_fHeight * 0.5f;
			break;
		case AGUI2ATTACHMENT_BOTTOMRIGHT:
			a_rX -= a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
	}
}

void AGUI2Element::GetScreenTransform( AGUI2Transform& a_rOutTransform )
{
	Toshi::TVector2 vecAnchorPos = { 0.0f, 0.0f };
	Toshi::TVector2 vecPivotPos  = { 0.0f, 0.0f };

	if ( m_pParent != TNULL )
	{
		TFLOAT fParentWidth, fParentHeight;
		m_pParent->GetDimensions( fParentWidth, fParentHeight );
		AnchorPos( vecAnchorPos.x, vecAnchorPos.y, fParentWidth, fParentHeight );
	}

	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );
	PivotPos( vecPivotPos.x, vecPivotPos.y, fWidth, fHeight );

	AGUI2Transform transform1;
	AGUI2Transform transform2;

	if ( m_pParent )
	{
		m_pParent->GetScreenTransform( transform1 );
		transform2 = GetTransform();
	}
	else
	{
		auto pDisplayParams = Toshi::TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

		transform1.m_Rotation[ 0 ] = { 1.0f, 0.0f };
		transform1.m_Rotation[ 1 ] = { 0.0f, 1.0f };
		transform1.m_Translation   = { pDisplayParams->uiWidth / 2.0f, pDisplayParams->uiHeight / 2.0f };

		transform1.PreMultiply( pDisplayParams->uiWidth / fWidth, pDisplayParams->uiHeight / fHeight );
		transform2 = GetTransform();
	}

	Toshi::TVector2 vec;
	transform1.Transform( vec, vecAnchorPos );
	transform1.m_Translation = { vec.x, vec.y };

	transform2.Transform( vec, vecPivotPos );
	transform2.m_Translation = { vec.x, vec.y };

	AGUI2Transform::Multiply( a_rOutTransform, transform1, transform2 );
}

void AGUI2Element::GetInvScreenTransform( AGUI2Transform& a_rOutTransform )
{
	AGUI2Transform transform;
	GetScreenTransform( transform );
	transform.GetInverse( a_rOutTransform );
}
