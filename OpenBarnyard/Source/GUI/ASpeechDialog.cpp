#include "pch.h"
#include "ASpeechDialog.h"
#include "Helpers/ASpeechDialogManager.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TFLOAT s_fTextX             = 20.0f; // 0x00773634
static TFLOAT s_fTextY             = 10.0f; // 0x00773638, also the top scroll limit
static TFLOAT s_fScrollSpeed       = 20.0f; // 0x0077363c, set by SetText
static TFLOAT s_fManualScrollSpeed = 60.0f; // 0x00773640
static TFLOAT s_fVisibleTextHeight;         // 0x00781e30, set by Create

// NOTE: retail reads the attachment of the text from two zero initialised globals that are never written (0x00781e34, 0x00781e38)
static T2GUIATTACHMENT s_eTextAnchor = T2GUIATTACHMENT_TOPLEFT;
static T2GUIATTACHMENT s_eTextPivot  = T2GUIATTACHMENT_TOPLEFT;

// $Barnyard: FUNCTION 0044e280
ASpeechDialog::ASpeechDialog()
    : m_fPromptBob( 0.0f )
    , m_pOwner( TNULL )
    , m_bScrollDone( TTRUE )
{
}

// $Barnyard: FUNCTION 0044e330
// $Barnyard: FUNCTION 0044e8a0
ASpeechDialog::~ASpeechDialog()
{
	m_oText.RemoveSelf();
	m_oName.RemoveSelf();
	m_oDialog.RemoveSelf();
	m_aArrows[ 0 ].RemoveSelf();
	m_aArrows[ 1 ].RemoveSelf();
}

// $Barnyard: FUNCTION 0044e8c0
void ASpeechDialog::Create( const TCHAR* a_szNameFont, T2GUIFont* a_pTextFont )
{
	TFLOAT fScreenWidth, fScreenHeight;
	T2GUI::GetContext()->GetRootElement()->GetDimensions( fScreenWidth, fScreenHeight );

	m_fTime = 0.0f;
	m_oDialog.Create();
	m_oDialog.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oDialog.SetDialogSize( fScreenWidth - 4.0f, fScreenHeight * 0.3f );
	m_oDialog.GetTransform().SetIdentity();
	m_oDialog.GetTransform().Scale( 1.0f, 1.0f );

	TFLOAT fWidth, fHeight;
	m_oDialog.GetContentDimensions( fWidth, fHeight );

	// The name
	T2GUIFont* pNameFont = T2GUIFontManager::FindFont( a_szNameFont );
	m_oName.Create( pNameFont, fWidth * 0.5f );
	m_oName.SetScale( 1.4f );
	m_oName.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oName.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oName.SetTransform( -6.0f, 0.0f, 0.0f );

	TFLOAT fNameHeight = pNameFont->GetLineHeight();
	m_oName.SetSize( fWidth * 0.5f, fNameHeight, TTRUE );

	// The text
	m_pFont = a_pTextFont;
	m_oText.Create( a_pTextFont, 100.0f );
	m_oText.SetAttachment( s_eTextAnchor, s_eTextPivot );
	m_oText.SetTextAlign( T2GUIFont::TextAlign_Left );

	// The prompt
	ALocale::FormatInputTags( m_wszPrompt, TARRAYSIZE( m_wszPrompt ), 1049 );
	m_oPrompt.Create( pNameFont, fWidth * 0.5f );
	m_oPrompt.SetText( m_wszPrompt );
	m_oPrompt.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_oPrompt.SetTextAlign( T2GUIFont::TextAlign_Right );
	m_oPrompt.SetTransform( -10.0f, 0.0f, 0.0f );
	m_oPrompt.Hide();
	m_fPromptBob = fScreenHeight * 0.002f;
	m_oDialog.AddChildToRectangle( &m_oPrompt, T2GUIATTACHMENT_BOTTOMCENTER );

	// Fit as many lines of text as possible
	TFLOAT fLineHeight   = m_pFont->GetLineHeight() + m_pFont->GetLinesMargin();
	s_fVisibleTextHeight = TINT( ( fHeight - fNameHeight ) / fLineHeight ) * fLineHeight;
	m_fTextHeight        = s_fVisibleTextHeight;
	m_oText.SetSize( fWidth - 60.0f, 500.0f, TTRUE );
	m_oText.SetTransform( s_fTextX, s_fTextY, 0.0f );

	// The arrows
	T2GUITextureSection* pArrow  = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	TFLOAT               fArrowW = TMath::Abs( TFLOAT( pArrow->GetPoint2X() - pArrow->GetPoint1X() ) ) * 0.45f;
	TFLOAT               fArrowH = TMath::Abs( TFLOAT( pArrow->GetPoint2Y() - pArrow->GetPoint1Y() ) ) * 0.45f;

	m_aArrows[ 0 ].SetDimensions( fArrowW, fArrowH );
	m_aArrows[ 0 ].SetTextureSection( pArrow );
	m_aArrows[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_aArrows[ 0 ].SetTransform( -39.0f, -15.0f, TMath::HALF_PI );
	m_aArrows[ 0 ].Hide();

	m_aArrows[ 1 ].SetDimensions( fArrowW, fArrowH );
	m_aArrows[ 1 ].SetTextureSection( pArrow );
	m_aArrows[ 1 ].SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aArrows[ 1 ].SetTransform( -38.0f, -57.0f, -TMath::HALF_PI );
	m_aArrows[ 1 ].Hide();

	m_oDialog.AddChildToRectangle( &m_aArrows[ 0 ], T2GUIATTACHMENT_MIDDLERIGHT );
	m_oDialog.AddChildToRectangle( &m_aArrows[ 1 ], T2GUIATTACHMENT_BOTTOMRIGHT );
	m_oDialog.AddChildToRectangle( &m_oText, T2GUIATTACHMENT_MIDDLECENTER );
	m_oDialog.AddChildToRectangle( &m_oName, T2GUIATTACHMENT_TOPCENTER );
	m_oDialog.SetUseScissor( TTRUE );
}

// $Barnyard: FUNCTION 0044ec80
void ASpeechDialog::Reset()
{
	m_fTime = 0.0f;
	m_oText.SetText( TNULL );
	m_oDialog.GetTransform().SetIdentity();
	m_oDialog.GetTransform().Scale( 1.0f, 1.0f );
	m_oText.SetTransform( s_fTextX, s_fTextY, 0.0f );
	m_fTextHeight = s_fVisibleTextHeight;
	m_bScrollDone = TTRUE;
	m_aArrows[ 0 ].Hide();
	m_aArrows[ 1 ].Hide();
	m_oPrompt.Hide();
}

// $Barnyard: FUNCTION 0044e520
void ASpeechDialog::Show()
{
	T2GUI::GetContext()->GetRootElement()->AddChildTail( m_oDialog );
	m_fTime = 0.0f;
}

// $Barnyard: FUNCTION 0044e550
void ASpeechDialog::Hide()
{
	m_oName.SetText( TNULL );
	m_oText.SetText( TNULL );
	m_oDialog.RemoveSelf();
	m_fTime = 0.0f;
}

// $Barnyard: FUNCTION 0044e5e0
void ASpeechDialog::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible == TTRUE )
		m_oDialog.Show();
	else
		m_oDialog.Hide();
}

// $Barnyard: FUNCTION 0044e440
void ASpeechDialog::SetText( const TWCHAR* a_wszText, TUINT32 a_uiColour )
{
	m_oText.SetText( a_wszText );

	TFLOAT fWidth, fHeight;
	m_oText.GetDimensions( fWidth, fHeight );

	TFLOAT fTextHeight = m_pFont->GetTextHeightWrapped( a_wszText, fWidth, 1.0f );

	if ( fTextHeight > m_fTextHeight )
	{
		// The text got taller, scroll it up
		TFLOAT fOverflow = fTextHeight - m_fTextHeight;
		TFLOAT fY        = m_bScrollDone ? m_oText.GetTransform().GetTranslation().y : m_fScrollTarget;

		m_bScrollDone   = TFALSE;
		m_fScrollTarget = fY - fOverflow;
		m_fTextHeight += fOverflow;

		s_fScrollSpeed = m_oText.GetTransform().GetTranslation().y - m_fScrollTarget;
		TMath::Clip( s_fScrollSpeed, 20.0f, s_fScrollSpeed );
	}

	m_oText.SetColour( a_uiColour );
}

// $Barnyard: FUNCTION 0044e420
void ASpeechDialog::SetName( const TWCHAR* a_wszName, TUINT32 a_uiColour )
{
	m_oName.SetText( a_wszName );
	m_oName.SetColour( a_uiColour );
}

// $Barnyard: FUNCTION 0044e590
void ASpeechDialog::SetScale( TFLOAT a_fScale )
{
	if ( a_fScale < 0.01f )
		a_fScale = 0.01f;

	m_oDialog.GetTransform().SetIdentity();
	m_oDialog.GetTransform().Scale( a_fScale, a_fScale );
}

// $Barnyard: FUNCTION 0044ed30
void ASpeechDialog::Update( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;

	TFLOAT fTop        = s_fTextY;
	TFLOAT fScrollable = m_fTextHeight - s_fVisibleTextHeight - s_fTextY;

	if ( !m_bScrollDone )
	{
		// Scroll the text up to m_fScrollTarget
		TFLOAT fY = m_oText.GetTransform().GetTranslation().y - s_fScrollSpeed * a_fDeltaTime;

		if ( fY < m_fScrollTarget )
		{
			TMath::Clip( fY, -fScrollable, s_fTextY );
			m_bScrollDone = TTRUE;
		}

		m_oText.SetTransform( m_oText.GetTransform().GetTranslation().x, fY, 0.0f );
	}
	else
	{
		ASpeechDialogManager::Speech* pSpeech = TSTATICCAST( ASpeechDialogManager::Speech, m_pOwner );

		if ( pSpeech && pSpeech->eState == ASpeechDialogManager::Speech::STATE_SHOWN && m_fTextHeight > s_fVisibleTextHeight )
		{
			// The player can scroll the text
			TFLOAT fDirection = 0.0f;
			m_aArrows[ 0 ].Show();
			m_aArrows[ 1 ].Show();

			TInputDeviceController* pController = AInputHandler::GetSingleton()->GetMainController();

			if ( pController )
			{
				if ( pController->IsDown( TInputDeviceKeyboard::KEY_DOWN ) )
					fDirection = -1.0f;
				else if ( pController->IsDown( TInputDeviceKeyboard::KEY_UP ) )
					fDirection = 1.0f;
			}

			TFLOAT fY = s_fManualScrollSpeed * a_fDeltaTime * fDirection + m_oText.GetTransform().GetTranslation().y;
			TMath::Clip( fY, -fScrollable, fTop );
			m_oText.SetTransform( m_oText.GetTransform().GetTranslation().x, fY, 0.0f );

			if ( fY == fTop )
				m_aArrows[ 0 ].Hide();
			else if ( fY == -fScrollable )
				m_aArrows[ 1 ].Hide();
		}
	}

	if ( m_oPrompt.IsVisible() )
	{
		// Bob the prompt up and down
		m_oPrompt.SetTransform( -10.0f, ( TMath::Sin( m_fTime * 8.0f ) - 1.0f ) * m_fPromptBob + 8.0f, 0.0f );
	}
}
