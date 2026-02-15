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
	m_pMaterial = a_rTexSection.GetMaterial();
	m_UV1       = a_rTexSection.GetUVPoint1();
	m_UV2       = a_rTexSection.GetUVPoint2();
}

TOSHI_NAMESPACE_END
