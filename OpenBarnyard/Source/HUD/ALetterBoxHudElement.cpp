#include "pch.h"
#include "ALetterBoxHudElement.h"
#include "GUI/AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00455760, CreateObject 004558b0, CreateObjectInPlace 00455910
TDEFINE_CLASS_NORUNTIME( ALetterBoxHudElement );

// $Barnyard: FUNCTION 00455770
ALetterBoxHudElement::ALetterBoxHudElement()
    : AHudElement( MASK_LETTERBOX )
{
}

// $Barnyard: FUNCTION 00455950
// $Barnyard: FUNCTION 004557d0
ALetterBoxHudElement::~ALetterBoxHudElement()
{
}

// $Barnyard: FUNCTION 00455970
TBOOL ALetterBoxHudElement::OnAttach()
{
	TFLOAT fWidth;
	TFLOAT fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	// The bars cover whatever doesn't fit a 16:9 picture
	TFLOAT fBarsHeight      = fHeight - fWidth * 0.5625f;
	TFLOAT fTopBarHeight    = 0.5f * fBarsHeight;
	TFLOAT fBottomBarHeight = fBarsHeight - fTopBarHeight;

	m_oTopBar.SetDimensions( fWidth, fTopBarHeight );
	m_oTopBar.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTopBar.SetColour( 0xff000000 );

	m_oBottomBar.SetDimensions( fWidth, fBottomBarHeight );
	m_oBottomBar.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oBottomBar.SetColour( 0xff000000 );

	AGUI2::GetRootElement()->AddChildTail( m_oTopBar );
	m_oTopBar.Hide();

	AGUI2::GetRootElement()->AddChildTail( m_oBottomBar );
	m_oBottomBar.Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 00455840
void ALetterBoxHudElement::OnDetach()
{
	m_oTopBar.RemoveSelf();
	m_oBottomBar.RemoveSelf();
}

// $Barnyard: FUNCTION 00455870
void ALetterBoxHudElement::Show()
{
	m_oTopBar.Show();
	m_oBottomBar.Show();
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00455890
void ALetterBoxHudElement::Hide()
{
	m_oTopBar.Hide();
	m_oBottomBar.Hide();
	m_bCurrentlyVisible = TFALSE;
}
