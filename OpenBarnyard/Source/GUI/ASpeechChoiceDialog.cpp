#include "pch.h"
#include "ASpeechChoiceDialog.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0044e600
ASpeechChoiceDialog::ASpeechChoiceDialog()
    : m_iSelection( 0 )
    , m_iNumChoices( 0 )
    , m_fTime( 0.0f )
    , m_fArrowBob( 0.0f )
    , m_fLineScale( 1.0f )
{
}

// $Barnyard: FUNCTION 0044e6a0
// $Barnyard: FUNCTION 0044ef40
ASpeechChoiceDialog::~ASpeechChoiceDialog()
{
	for ( TINT i = 0; i < MAX_NUM_CHOICES; i++ )
		m_aChoices[ i ].RemoveSelf();

	m_oDialog.RemoveSelf();
}

// $Barnyard: FUNCTION 0044e750
void ASpeechChoiceDialog::SetSelection( TINT a_iSelection )
{
	m_iSelection = a_iSelection;
	TMath::Clip( m_iSelection, 0, m_iNumChoices );

	for ( TINT i = 0; i < MAX_NUM_CHOICES; i++ )
		m_aChoices[ i ].SetColour( m_uiColour );

	m_aChoices[ m_iSelection ].SetColour( m_uiSelectedColour );
}

// $Barnyard: FUNCTION 0044e7d0
void ASpeechChoiceDialog::Hide()
{
	for ( TINT i = 0; i < MAX_NUM_CHOICES; i++ )
		m_aChoices[ i ].SetText( TNULL );

	m_oDialog.RemoveSelf();
	m_fTime = 0.0f;
}

// $Barnyard: FUNCTION 0044e810
void ASpeechChoiceDialog::Update( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;

	if ( m_pFont )
	{
		TFLOAT fBob = TMath::Sin( m_fTime * 6.0f );
		TFLOAT fY   = m_pFont->GetLineHeight() * m_iSelection * m_fLineScale + 18.0f;

		m_oArrow.SetTransform( fBob * m_fArrowBob + 15.0f, fY - m_oArrow.GetWidth() * 0.5f, 0.0f );
	}
}

// NOTE: folded with ASpeechDialog::SetScale (0044e590), both classes have the dialog at +0x04
void ASpeechChoiceDialog::SetScale( TFLOAT a_fScale )
{
	if ( a_fScale < 0.01f )
		a_fScale = 0.01f;

	m_oDialog.GetTransform().SetIdentity();
	m_oDialog.GetTransform().Scale( a_fScale, a_fScale );
}
