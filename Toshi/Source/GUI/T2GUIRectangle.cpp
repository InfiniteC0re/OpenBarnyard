#include "ToshiPCH.h"
#include "T2GUIRectangle.h"
#include "T2GUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006c3ae0
T2GUIRectangle::T2GUIRectangle()
{
	m_pMaterial = TNULL;
}

// $Barnyard: FUNCTION 006c39b0
void T2GUIRectangle::Render()
{
	auto pRenderer = T2GUI::GetRenderer();

	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->SetColour( m_uiColour );

	if ( IsInFront() )
	{
		pRenderer->ResetZCoordinate();
	}

	Toshi::TVector2 pos1 = { -m_fWidth / 2, -m_fHeight / 2 };
	Toshi::TVector2 pos2 = { pos1.x + m_fWidth, pos1.y + m_fHeight };
	pRenderer->RenderRectangle( pos1, pos2, m_UV1.AsVector2(), m_UV2.AsVector2() );

	T2GUIElement::Render();
}

// $Barnyard: FUNCTION 006c3910
void T2GUIRectangle::SetTextureSection( T2GUITextureSection* a_pTexSection )
{
	if ( a_pTexSection )
	{
		m_pMaterial = a_pTexSection->GetMaterial();
		m_UV1       = a_pTexSection->GetUVPoint1();
		m_UV2       = a_pTexSection->GetUVPoint2();
	}
	else
	{
		m_pMaterial = TNULL;
	}
}

// $Barnyard: FUNCTION 006c3b00
void T2GUIRectangle::SetTextureSection( const T2GUITextureSection& a_rTexSection )
{
	// The rectangle gets the size of the section
	m_fWidth    = TMath::Abs( TFLOAT( a_rTexSection.GetPoint2X() - a_rTexSection.GetPoint1X() ) );
	m_fHeight   = TMath::Abs( TFLOAT( a_rTexSection.GetPoint2Y() - a_rTexSection.GetPoint1Y() ) );
	m_pMaterial = a_rTexSection.GetMaterial();
	m_UV1       = a_rTexSection.GetUVPoint1();
	m_UV2       = a_rTexSection.GetUVPoint2();
}

// $Barnyard: FUNCTION 006c38d0
void T2GUIRectangle::SetUVs( const TVector2& a_rcUV1, const TVector2& a_rcUV2, const TVector2& a_rcUV3, const TVector2& a_rcUV4 )
{
	m_UV1.x = a_rcUV1.x;
	m_UV1.y = a_rcUV1.y;
	m_UV1.z = a_rcUV2.x;
	m_UV1.w = a_rcUV2.y;
	m_UV2.x = a_rcUV3.x;
	m_UV2.y = a_rcUV3.y;
	m_UV2.z = a_rcUV4.x;
	m_UV2.w = a_rcUV4.y;
}

// $Barnyard: FUNCTION 006c3a50
void T2GUIRectangle::FlipU()
{
	TFLOAT fU1 = m_UV1.x;
	TFLOAT fV1 = m_UV1.y;
	m_UV1.x    = m_UV1.z;
	m_UV1.y    = m_UV1.w;
	m_UV1.z    = fU1;
	m_UV1.w    = fV1;

	TFLOAT fU3 = m_UV2.x;
	TFLOAT fV3 = m_UV2.y;
	m_UV2.x    = m_UV2.z;
	m_UV2.y    = m_UV2.w;
	m_UV2.z    = fU3;
	m_UV2.w    = fV3;
}

// $Barnyard: FUNCTION 006c3a90
void T2GUIRectangle::FlipV()
{
	TFLOAT fU1 = m_UV1.x;
	TFLOAT fV1 = m_UV1.y;
	m_UV1.x    = m_UV2.z;
	m_UV1.y    = m_UV2.w;
	m_UV2.z    = fU1;
	m_UV2.w    = fV1;

	TFLOAT fU2 = m_UV1.z;
	TFLOAT fV2 = m_UV1.w;
	m_UV1.z    = m_UV2.x;
	m_UV1.w    = m_UV2.y;
	m_UV2.x    = fU2;
	m_UV2.y    = fV2;
}

// $Barnyard: FUNCTION 006c3970
void T2GUIRectangle::GetUVs( TVector2& a_rUV1, TVector2& a_rUV2, TVector2& a_rUV3, TVector2& a_rUV4 ) const
{
	a_rUV1.x = m_UV1.x;
	a_rUV1.y = m_UV1.y;
	a_rUV2.x = m_UV1.z;
	a_rUV2.y = m_UV1.w;
	a_rUV3.x = m_UV2.x;
	a_rUV3.y = m_UV2.y;
	a_rUV4.x = m_UV2.z;
	a_rUV4.y = m_UV2.w;
}

TOSHI_NAMESPACE_END
