#include "pch.h"
#include "AHintDisplayState.h"
#include "GUI/AGUI2.h"
#include "Helpers/AHintManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Tasks/ARootTask.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00448c80, CreateObject 00449330, CreateObjectInPlace 00449390
TDEFINE_CLASS( AHintDisplayState );

TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 00448c90
AHintDisplayState::AHintDisplayState()
    : m_eInput( INPUT_NONE )
    , m_iUnkA00( 0 )
    , m_iUnkA04( 0 )
{
	SetInputMap( TPS8( DialogInputMap ) );
	m_iHint        = -1;
	m_bCollectHint = TTRUE;
	m_fTime        = 0.0f;
	m_eInput       = INPUT_NONE;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: CreateGUI measures the page number before it's formatted
	m_wszPageNumber[ 0 ] = L'\0';
#endif
}

// $Barnyard: FUNCTION 00448e10
AHintDisplayState::AHintDisplayState( TBOOL a_bCollectHint )
    : m_eInput( INPUT_NONE )
    , m_iUnkA00( 0 )
    , m_iUnkA04( 0 )
{
	SetInputMap( TPS8( DialogInputMap ) );
	m_iHint        = -1;
	m_fTime        = 0.0f;
	m_eInput       = INPUT_NONE;
	m_bCollectHint = a_bCollectHint;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: CreateGUI measures the page number before it's formatted
	m_wszPageNumber[ 0 ] = L'\0';
#endif
}

// $Barnyard: FUNCTION 00448fa0
// $Barnyard: FUNCTION 004493d0
AHintDisplayState::~AHintDisplayState()
{
}

// $Barnyard: FUNCTION 00449120
TBOOL AHintDisplayState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand == AInputCommand_Move )
	{
		// The analog stick turns the pages
		if ( m_fTime > 1.0f )
		{
			TFLOAT fX = a_pInputEvent->GetMagnitudeFloat( 0 );
			TFLOAT fY = a_pInputEvent->GetMagnitudeFloat( 1 );

			if ( fX < -0.5f )
				m_eInput |= INPUT_PREVIOUS;
			else if ( fX > 0.5f )
				m_eInput |= INPUT_NEXT;

			if ( fY < -0.5f )
				m_eInput |= INPUT_UP;
			else if ( fY > 0.5f )
				m_eInput |= INPUT_DOWN;

			if ( fX < -0.5f || fX > 0.5f || fY > 0.5f || fY < -0.5f )
			{
				m_fTime = 0.0f;
				return TTRUE;
			}
		}

		return TFALSE;
	}

	if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN && a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_REPEAT )
		return TFALSE;

	switch ( a_eInputCommand )
	{
		case AInputCommand_OK:
		case AInputCommand_Shoot:
			m_eInput |= INPUT_OK;
			return TTRUE;
		case AInputCommand_Cancel:
			m_eInput |= INPUT_CANCEL;
			return TTRUE;
		case AInputCommand_Up:
		case AInputCommand_Forward:
			m_eInput |= INPUT_UP;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		case AInputCommand_Down:
		case AInputCommand_Backward:
			m_eInput |= INPUT_DOWN;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		case AInputCommand_Left:
			m_eInput |= INPUT_PREVIOUS;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		case AInputCommand_Right:
			m_eInput |= INPUT_NEXT;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		default:
			return TFALSE;
	}
}

// $Barnyard: FUNCTION 0044a010
TBOOL AHintDisplayState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;

	if ( m_eInput & INPUT_PREVIOUS )
	{
		if ( !m_bCollectHint )
		{
			// The page before the first hint is the progress page
			TINT iHint = m_iHint;
			if ( iHint < 0 ) iHint = AHintManager::GetNumCollectedHints();

			m_iHint = iHint - 1;
			UpdatePage();
		}
	}
	else if ( m_eInput & INPUT_NEXT )
	{
		if ( !m_bCollectHint )
		{
			TINT iHint = m_iHint + 1;
			if ( iHint >= AHintManager::GetNumCollectedHints() ) iHint = -1;

			m_iHint = iHint;
			UpdatePage();
		}
	}
	else if ( m_eInput & ( INPUT_OK | INPUT_CANCEL ) )
	{
		Remove();
		return TTRUE;
	}

	m_eInput = INPUT_NONE;
	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00449f30
void AHintDisplayState::OnInsertion()
{
	ARootTask::GetSingleton()->SetPaused( TTRUE );

	// Darken the screen
	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	T2GUIElement* pRoot = AGUI2::GetRootElement();
	m_oFade.SetDimensions( fWidth, fHeight );
	m_oFade.SetColour( 0x7F000000 );
	pRoot->AddChildTail( m_oFade );

	if ( m_bCollectHint )
	{
		m_iHint = AHintManager::GetSingleton()->CollectHint();

		if ( m_iHint < 0 || m_iHint >= AHintManager::GetNumCollectedHints() )
			m_bCollectHint = TFALSE;
	}
	else if ( AHintManager::GetNumCollectedHints() > 0 )
	{
		m_iHint = 0;
	}

	CreateGUI();

	if ( m_bCollectHint )
		m_oTitle.T2GUIElement::Hide();

	UpdatePage();
	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00449ae0
void AHintDisplayState::OnRemoval()
{
	m_oFade.RemoveSelf();
	m_oHint.RemoveSelf();
	m_oTitle.RemoveSelf();
	m_oPrompt.RemoveSelf();
	m_oRightArrow.RemoveSelf();
	m_oPageLabel.RemoveSelf();
	m_oPageNumber.RemoveSelf();
	m_oPageNumberArea.RemoveSelf();
	m_oLeftArrow.RemoveSelf();
	m_oProgressTitle.RemoveSelf();
	m_oProgress.RemoveSelf();
	m_oProgressArea.RemoveSelf();
	m_oNavigation.RemoveSelf();
	m_oHintArea.RemoveSelf();
	m_oPage.RemoveSelf();
	m_oBackground.RemoveSelf();

	ARootTask::GetSingleton()->SetPaused( TFALSE );
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 004490f0
void AHintDisplayState::OnActivate()
{
	m_oBackground.T2GUIElement::Show();
	SetInputMap( TPS8( DialogInputMap ) );
	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 00449110
void AHintDisplayState::OnDeactivate()
{
	m_oBackground.T2GUIElement::Hide();
	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 004493f0
void AHintDisplayState::CreateGUI()
{
	T2GUIElement* pRoot         = AGUI2::GetRootElement();
	TFLOAT        fScreenWidth  = pRoot->GetWidth() * 0.85f;
	TFLOAT        fScreenHeight = pRoot->GetHeight() * 0.85f;

	m_oBackground.SetDimensions( fScreenWidth, fScreenHeight );
	m_oBackground.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	// The title at the top
	TFLOAT fTextWidth = fScreenWidth * 0.95f;
	m_oTitle.Create( T2GUIFontManager::FindFont( "Rekord26" ), fTextWidth );
	m_oTitle.SetText( ALocaleManager::Interface()->GetString( 452 ) );
	m_oTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitle.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oTitle.SetScale( 1.5f );
	m_oBackground.AddChildTail( m_oTitle );

	// What to press at the bottom
	T2GUIFont* pFont18 = T2GUIFontManager::FindFont( "Rekord18" );
	ALocale::FormatInputTags( m_wszPrompt, TARRAYSIZE( m_wszPrompt ), ALocaleManager::Interface()->GetString( 1776 ) );
	m_oPrompt.Create( pFont18, fTextWidth );
	m_oPrompt.SetText( m_wszPrompt );
	m_oPrompt.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oPrompt.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oBackground.AddChildTail( m_oPrompt );

	// The page of the notebook in the middle
	T2GUITextureSection* pPage = T2GUITextureSectionManager::GetTextureSection( "Recipie_Page" );
	m_oPage.SetDimensions( fScreenWidth * 0.75f, fScreenHeight * 0.75f );
	m_oPage.SetTextureSection( pPage );
	m_oPage.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oPage.SetAlpha( 1.0f );
	m_oBackground.AddChildTail( m_oPage );

	// The navigation: "< Page n/m >"
	T2GUIFont*           pFont26 = T2GUIFontManager::FindFont( "Rekord26" );
	T2GUITextureSection* pArrow  = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );

	m_oLeftArrow.SetTextureSection( *pArrow );
	m_oLeftArrow.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );

	m_oPageLabel.Create( pFont26, pFont26->GetTextWidth( m_wszPageNumber, 1.0f ) );
	m_oPageLabel.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 1789 ) );
	m_oPageLabel.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oPageLabel.SetColour( 0xFFFFFFDD );

	TStringManager::String16Format( m_wszPageNumber, TARRAYSIZE( m_wszPageNumber ), L"%i/%i", 99, 999 );
	m_oPageNumber.Create( pFont26, pFont26->GetTextWidth( m_wszPageNumber, 1.0f ) );
	m_oPageNumber.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oPageNumber.SetText( m_wszPageNumber );
	m_oPageNumber.SetColour( 0xFFFFFF00 );

	m_oRightArrow.SetTextureSection( *pArrow );
	m_oRightArrow.FlipU();
	m_oRightArrow.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );

	TFLOAT fArrowHeight = pArrow->GetHeight();
	TFLOAT fHeight      = ( pFont26->GetLineHeight() >= fArrowHeight ) ? pFont26->GetLineHeight() : fArrowHeight;

	m_oPageNumberArea.SetDimensions( m_oPageNumber.GetWidth() + m_oPageLabel.GetWidth(), fHeight );
	m_oPageNumberArea.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oPageNumberArea.AddChildTail( m_oPageLabel );
	m_oPageNumberArea.AddChildTail( m_oPageNumber );
	m_oPageNumberArea.SetTranslation( 0.0f, 9.0f );
	m_oPageNumberArea.SetAlpha( 0.0f );

	m_oNavigation.SetDimensions( m_oPageNumberArea.GetWidth() + pArrow->GetWidth() * 4.0f, fHeight );
	m_oNavigation.AddChildTail( m_oLeftArrow );
	m_oNavigation.AddChildTail( m_oPageNumberArea );
	m_oNavigation.AddChildTail( m_oRightArrow );
	m_oNavigation.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oNavigation.SetTransform( 0.0f, 24.0f );
	m_oNavigation.SetAlpha( 0.0f );
	m_oPage.AddChildTail( m_oNavigation );

	// The hint
	pFont26 = T2GUIFontManager::FindFont( "Rekord26" );

	TFLOAT fHintAreaHeight = m_oPage.GetHeight() - m_oNavigation.GetHeight();
	TFLOAT fHintAreaWidth  = m_oPage.GetWidth() * 0.8f;
	m_oHintArea.SetDimensions( fHintAreaWidth, fHintAreaHeight );
	m_oHintArea.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );

	m_oHint.Create( pFont26, fHintAreaWidth );
	m_oHint.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oHint.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oHint.SetTransform( 0.0f, 10.0f );
	m_oHint.SetColour( 0xFFFFC800 );
	m_oHintArea.AddChildTail( m_oHint );
	m_oHintArea.SetTransform( 0.0f, 6.0f );
	m_oHintArea.SetAlpha( 0.0f );
	m_oNavigation.AddChildTail( m_oHintArea );

	// The progress page
	T2GUIFontManager::FindFont( "Rekord18" );
	pFont26 = T2GUIFontManager::FindFont( "Rekord26" );

	TStringManager::String16Format( m_wszUnk9CC, TARRAYSIZE( m_wszUnk9CC ), L"%i", 9999 );
	TStringManager::String16Format( m_wszUnk9DC, TARRAYSIZE( m_wszUnk9DC ), L"%i", 9999 );
	TStringManager::String16Format( m_wszProgress, TARRAYSIZE( m_wszProgress ), L"%i%%", 1000 );

	m_oProgressArea.SetDimensions( fScreenWidth * 0.6f, ( m_oPage.GetHeight() - m_oNavigation.GetHeight() ) * 0.75f );
	m_oProgressArea.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oProgressArea.SetTransform( 0.0f, -24.0f );
	m_oProgressArea.SetAlpha( 0.0f );
	m_oPage.AddChildTail( m_oProgressArea );

	m_oProgressTitle.Create( pFont26, m_oProgressArea.GetWidth() );
	m_oProgressTitle.SetText( ALocaleManager::Interface()->GetString( 1778 ) );
	m_oProgressTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oProgressTitle.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oProgressTitle.SetColour( 0xFFFFD800 );
	m_oProgressTitle.SetShadowEnabled( TTRUE );
	m_oProgressTitle.SetTranslation( 0.0f, 10.0f );
	m_oProgressArea.AddChildTail( m_oProgressTitle );

	m_oProgress.Create( pFont26, m_oProgressArea.GetWidth() );
	m_oProgress.SetText( m_wszProgress );
	m_oProgress.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oProgress.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oProgress.SetColour( 0xFFFFFFFF );
	m_oProgressArea.AddChildTail( m_oProgress );

	m_oBackground.SetAlpha( 0.0f );
	AGUI2::GetRootElement()->AddChildTail( m_oBackground );
	m_oBackground.T2GUIElement::Hide();
}

// $Barnyard: FUNCTION 00449c00
void AHintDisplayState::UpdatePage()
{
	TStringManager::String16Format( m_wszPageNumber, TARRAYSIZE( m_wszPageNumber ), L"%i/%i", m_iHint + 1, AHintManager::GetMaxHints() );

	// No paging while collecting a hint
	m_oLeftArrow.SetVisible( !m_bCollectHint );
	m_oRightArrow.SetVisible( !m_bCollectHint );

	m_oPageNumber.T2GUIElement::Show();
	m_oPageLabel.T2GUIElement::Show();

	if ( AHintManager::GetNumCollectedHints() == 0 )
	{
		m_oLeftArrow.T2GUIElement::Hide();
		m_oRightArrow.T2GUIElement::Hide();
	}

	if ( m_iHint > -1 )
	{
		// Show the hint, with the smaller font if it doesn't fit
		m_oProgressArea.T2GUIElement::Hide();
		m_oHintArea.T2GUIElement::Show();

		T2GUIFont* pFont18 = T2GUIFontManager::FindFont( "Rekord18" );
		T2GUIFont* pFont26 = T2GUIFontManager::FindFont( "Rekord26" );

		TFLOAT fTextWidth = pFont26->GetTextWidth( ALocaleManager::Interface()->GetString( m_iHint + 1791 ), 1.0f );
		TFLOAT fMaxWidth  = m_oPage.GetWidth() * 0.75f;

		if ( fTextWidth <= fMaxWidth )
			m_oHint.Create( pFont26, fMaxWidth );
		else
			m_oHint.Create( pFont18, fMaxWidth );

		m_oHint.SetText( ALocaleManager::Interface()->GetString( m_iHint + 1791 ) );
	}
	else
	{
		// Show how many hints were found
		TINT iPercentage = 0;

		if ( AHintManager::GetMaxHints() > 0 )
			iPercentage = TINT( TFLOAT( AHintManager::GetNumCollectedHints() ) * 100.0f / TFLOAT( AHintManager::GetMaxHints() ) );

		TStringManager::String16Format( m_wszProgress, TARRAYSIZE( m_wszProgress ), L"%i", iPercentage );
		TStringManager::String16Copy( m_wszProgress + TStringManager::String16Length( m_wszProgress ), L"%" );

		TFLOAT fAreaWidth  = m_oProgressArea.GetWidth();
		TFLOAT fAreaHeight = m_oProgressArea.GetHeight() - m_oProgressTitle.GetHeight() - 15.0f;
		TFLOAT fMaxSize    = ( fAreaWidth >= fAreaHeight ) ? fAreaHeight : fAreaWidth;

		m_oProgress.SetScale( 1.0f );
		m_oProgress.SetTextAndFitWidth( m_wszProgress );

		TFLOAT fSize = m_oProgress.GetWidth();
		if ( m_oProgress.GetHeight() > fSize ) fSize = m_oProgress.GetHeight();

		if ( fSize > 0.0f )
		{
			// Make the number as big as the area
			TFLOAT fScale = fMaxSize / fSize;
			TMath::Clip( fScale, 0.01f, 1.5f );

			m_oProgress.SetScale( fScale );
			m_oProgress.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_MIDDLECENTER );
			m_oProgress.SetTranslation( 0.0f, fAreaHeight * -0.5f );
		}

		m_oProgressArea.T2GUIElement::Show();
		m_oHintArea.T2GUIElement::Hide();
	}
}
