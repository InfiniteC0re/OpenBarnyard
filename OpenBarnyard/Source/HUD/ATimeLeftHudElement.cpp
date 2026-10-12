#include "pch.h"
#include "ATimeLeftHudElement.h"
#include "GUI/AGUI2.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0045aa00, CreateObject 0045ac00, CreateObjectInPlace 0045ac60
TDEFINE_CLASS( ATimeLeftHudElement );

// $Barnyard: FUNCTION 0045aa10
ATimeLeftHudElement::ATimeLeftHudElement()
    : AHudElement( MASK_MINIGAME | MASK_UNK2 | MASK_UNK15 | MASK_TIME_LEFT )
{
}

// $Barnyard: FUNCTION 0045aca0
// $Barnyard: FUNCTION 0045aa60
ATimeLeftHudElement::~ATimeLeftHudElement()
{
}

// $Barnyard: FUNCTION 0045ab30
void ATimeLeftHudElement::GetDimensions( TINT& a_rWidth, TINT& a_rHeight )
{
	TFLOAT fWidth;
	TFLOAT fHeight;
	m_oTimer.GetDimensions( fWidth, fHeight );

	a_rWidth  = TMath::Round( fWidth );
	a_rHeight = TMath::Round( fHeight );
}

// $Barnyard: FUNCTION 0045acc0
TBOOL ATimeLeftHudElement::OnAttach()
{
	m_oTimer.Create( 0.0f, 0.0f );
	m_oTimer.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );

	TFLOAT fWidth;
	TFLOAT fHeight;
	m_oTimer.GetDimensions( fWidth, fHeight );
	m_oTimer.SetDimensions( fWidth * 0.75f, fHeight * 0.75f );

	AGUI2::GetRootElement()->AddChildTail( m_oTimer );
	Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 0045a6b0
void ATimeLeftHudElement::OnDetach()
{
	// NOTE: folded with ATerrainSectionHudElement::OnDestroy in retail.
	m_oTimer.RemoveSelf();
}

// $Barnyard: FUNCTION 0045aac0
void ATimeLeftHudElement::Show()
{
	m_oTimer.SetVisible( TTRUE );
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 0045aae0
void ATimeLeftHudElement::Hide()
{
	m_oTimer.SetVisible( TFALSE );
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 0045ab00
void ATimeLeftHudElement::Setup( TFLOAT a_fDuration )
{
	m_oTimer.Reset();
	m_oTimer.SetDuration( a_fDuration );
}

// $Barnyard: FUNCTION 0045ab20
void ATimeLeftHudElement::SetTime( TFLOAT a_fTime )
{
	m_oTimer.SetTime( a_fTime );
}

// $Barnyard: FUNCTION 0045ab70
void ATimeLeftHudElement::SetScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	m_oTimer.GetTransform().SetFromScale( a_fScaleX, a_fScaleY );
}
