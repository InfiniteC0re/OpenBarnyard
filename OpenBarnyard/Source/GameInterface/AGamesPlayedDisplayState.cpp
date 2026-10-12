#include "pch.h"
#include "AGamesPlayedDisplayState.h"
#include "GUI/AGUI2.h"
#include "Helpers/AHighScoreManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0044c6b0, CreateObject 0044cf60, CreateObjectInPlace 0044cfc0 (the constructor is inlined in both)
TDEFINE_CLASS( AGamesPlayedDisplayState );

TPSTRING8_DECLARE( DialogInputMap );

// Texture sections of the grades (AHighScoreManager::GRADE)
static const TCHAR* s_aGradeTextures[ AHighScoreManager::GRADE_NUMOF ] = {
	"Grade_Star",
	"Grade_A",
	"Grade_B",
	"Grade_C",
	"Grade_D",
	"Grade_F",
}; // 0x0077361c

// $Barnyard: FUNCTION 0044c6c0
AGamesPlayedDisplayState::AGamesPlayedDisplayState()
    : m_eInput( INPUT_NONE )
    , m_iUnk2BFC( 0 )
    , m_iUnk2C00( 0 )
{
	SetInputMap( TPS8( DialogInputMap ) );
	m_iNumShownRecords  = 0;
	m_iNumRecords       = 0;
	m_iNumPlayedRecords = 0;
	m_fTime             = 0.0f;
	m_fFinishScoresTime = 0.0f;
	m_eInput            = INPUT_NONE;
	m_fTableWidth       = 0.0f;
	m_iRecord           = -1;
	m_iShownRecord      = -1;
	m_bOnlyPlayed       = TTRUE;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: CreateGUI measures the page number before it's formatted
	m_wszPageNumber[ 0 ] = L'\0';
#endif
}

// $Barnyard: FUNCTION 0044c950
// $Barnyard: FUNCTION 0044d000
AGamesPlayedDisplayState::~AGamesPlayedDisplayState()
{
}

// $Barnyard: FUNCTION 0044cd50
TBOOL AGamesPlayedDisplayState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
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

// $Barnyard: FUNCTION 0044e100
TBOOL AGamesPlayedDisplayState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;

	if ( m_eInput & INPUT_PREVIOUS )
	{
		// The page before the first one is the last one
		if ( m_iRecord < 0 )
			m_iRecord = GetPageRecord( GetNumPages() );
		else
			m_iRecord = FindPreviousRecord( m_iRecord, m_iRecordsPerPage );

		// Without the summary page, wrap around
		if ( m_bOnlyPlayed && m_iRecord < 0 )
			m_iRecord = GetPageRecord( GetNumPages() );

		UpdatePage();
	}
	else if ( m_eInput & INPUT_NEXT )
	{
		if ( m_iRecord < 0 )
			m_iRecord = GetPageRecord( 1 );
		else
			m_iRecord = FindNextRecord( m_iRecord, m_iRecordsPerPage );

		if ( m_bOnlyPlayed && m_iRecord < 0 )
			m_iRecord = GetPageRecord( 1 );

		UpdatePage();
	}
	else if ( m_eInput & INPUT_OK )
	{
		// NOTE: INPUT_CANCEL doesn't close the scorebook
		Remove();
		return TTRUE;
	}

	m_eInput = INPUT_NONE;

	if ( m_fFinishScoresTime > 0.0f )
	{
		m_fFinishScoresTime -= a_fDeltaTime;
	}
	else
	{
		for ( TINT i = NUM_ROWS - 1; i >= 0; i-- )
		{
			if ( m_aScores[ i ].IsSpinning() )
				m_aScores[ i ].ShowFinalScore();
		}
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0044e070
void AGamesPlayedDisplayState::OnInsertion()
{
	m_iRecordsPerPage   = NUM_ROWS;
	m_iNumShownRecords  = 0;
	m_iNumRecords       = 0;
	m_iNumPlayedRecords = 0;

	for ( TINT i = AHighScoreManager::NUM_RECORDS - 1; i >= 0; i-- )
	{
		AHighScoreManager::Record* pRecord = AHighScoreManager::GetRecord( i );

		if ( pRecord->iHidden == 0 )
		{
			m_iNumRecords++;

			if ( pRecord->bPlayed )
				m_iNumPlayedRecords++;
		}
	}

	m_iNumShownRecords = m_iNumPlayedRecords;
	m_iShownRecord     = -2;
	m_bOnlyPlayed      = TTRUE;

	if ( m_iNumPlayedRecords <= 0 )
		m_bOnlyPlayed = TFALSE;

	CreateGUI();
	UpdatePage();
	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0044cb70
void AGamesPlayedDisplayState::OnRemoval()
{
	m_oTitle.RemoveSelf();
	m_oPrompt.RemoveSelf();
	m_oRightArrow.RemoveSelf();
	m_oPageLabel.RemoveSelf();
	m_oPageNumber.RemoveSelf();
	m_oPageNumberArea.RemoveSelf();
	m_oLeftArrow.RemoveSelf();
	m_oNumStarsLabel.RemoveSelf();
	m_oNumPlayedLabel.RemoveSelf();
	m_oNumGamesLabel.RemoveSelf();
	m_oNumStars.RemoveSelf();
	m_oNumPlayed.RemoveSelf();
	m_oNumGames.RemoveSelf();
	m_oNumStarsRow.RemoveSelf();
	m_oNumPlayedRow.RemoveSelf();
	m_oNumGamesRow.RemoveSelf();
	m_oSummary.RemoveSelf();

	for ( TINT i = NUM_ROWS - 1; i >= 0; i-- )
	{
		m_aScores[ i ].RemoveSelf();
		m_aGrades[ i ].RemoveSelf();
		m_aGameNames[ i ].RemoveSelf();
		m_aGameNames[ i ].RemoveSelf();
		m_aRows[ i ].RemoveSelf();
	}

	m_oNavigation.RemoveSelf();
	m_oTable.RemoveSelf();
	m_oScoreCard.RemoveSelf();
	m_oBackground.RemoveSelf();

	AGameState::OnRemoval();
}

// NOTE: the original uses AHintDisplayState::OnActivate (004490f0) and OnDeactivate (00449110),
// the identical functions got merged since m_oBackground is at the same offset in both classes
void AGamesPlayedDisplayState::OnActivate()
{
	m_oBackground.T2GUIElement::Show();
	SetInputMap( TPS8( DialogInputMap ) );
	AGameState::OnActivate();
}

void AGamesPlayedDisplayState::OnDeactivate()
{
	m_oBackground.T2GUIElement::Hide();
	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 0044d020
void AGamesPlayedDisplayState::CreateGUI()
{
	TFLOAT fWidth  = AGUI2::GetRootElement()->GetWidth() * 0.95f;
	TFLOAT fHeight = AGUI2::GetRootElement()->GetHeight() * 0.95f;

	m_oBackground.SetDimensions( fWidth, fHeight );
	m_oBackground.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	// The title at the top
	TFLOAT fTextWidth = fWidth * 0.95f;
	m_oTitle.Create( T2GUIFontManager::FindFont( "Rekord26" ), fTextWidth );
	m_oTitle.SetText( ALocaleManager::Interface()->GetString( 1856 ) );
	m_oTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitle.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oTitle.SetScale( 1.2f );
	AGUI2::GetRootElement()->AddChildTail( m_oTitle );

	// What to press at the bottom
	T2GUIFont* pFont18 = T2GUIFontManager::FindFont( "Rekord18" );
	ALocale::FormatInputTags( m_wszPrompt, TARRAYSIZE( m_wszPrompt ), ALocaleManager::Interface()->GetString( 1776 ) );
	m_oPrompt.Create( pFont18, fTextWidth );
	m_oPrompt.SetText( m_wszPrompt );
	m_oPrompt.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oPrompt.SetTextAlign( T2GUIFont::TextAlign_Center );
	AGUI2::GetRootElement()->AddChildTail( m_oPrompt );

	// The score card in the middle
	T2GUITextureSection* pScoreCard = T2GUITextureSectionManager::GetTextureSection( "Score_Card" );
	m_oScoreCard.SetDimensions( fTextWidth, fHeight * 0.85f );
	m_oScoreCard.SetTextureSection( pScoreCard );
	m_oScoreCard.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oScoreCard.SetColour( 0xFFFFFFFF );
	m_oScoreCard.SetAlpha( 1.0f );
	m_oBackground.AddChildTail( m_oScoreCard );

	// The navigation: "< Page n/m >"
	T2GUIFont*           pFont26 = T2GUIFontManager::FindFont( "Rekord26" );
	T2GUITextureSection* pArrow  = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );

	m_oLeftArrow.SetTextureSection( *pArrow );
	m_oLeftArrow.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );

	m_oPageLabel.Create( pFont26, pFont26->GetTextWidth( m_wszPageNumber, 1.0f ) );
	m_oPageLabel.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 1855 ) );
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
	TFLOAT fNavHeight   = ( pFont26->GetLineHeight() >= fArrowHeight ) ? pFont26->GetLineHeight() : pArrow->GetHeight();

	m_oPageNumberArea.SetDimensions( m_oPageLabel.GetWidth() + m_oPageNumber.GetWidth(), fNavHeight );
	m_oPageNumberArea.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oPageNumberArea.AddChildTail( m_oPageLabel );
	m_oPageNumberArea.AddChildTail( m_oPageNumber );
	m_oPageNumberArea.SetTranslation( 0.0f, 9.0f );
	m_oPageNumberArea.SetAlpha( 0.0f );

	m_oNavigation.SetDimensions( m_oPageNumberArea.GetWidth() + pArrow->GetWidth() * 4.0f, fNavHeight );
	m_oNavigation.AddChildTail( m_oLeftArrow );
	m_oNavigation.AddChildTail( m_oPageNumberArea );
	m_oNavigation.AddChildTail( m_oRightArrow );
	m_oNavigation.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oNavigation.SetAlpha( 0.0f );
	m_oNavigation.SetTranslation( 0.0f, 26.0f );
	m_oScoreCard.AddChildTail( m_oNavigation );

	// The table of the games
	T2GUIFont* pNameFont = T2GUIFontManager::FindFont( "Rekord18" );

	TFLOAT fTableHeight = m_oScoreCard.GetHeight() - 35.0f - m_oNavigation.GetHeight();
	TFLOAT fTableWidth  = m_oScoreCard.GetWidth() * 0.85f;

	m_oTable.SetDimensions( fTableWidth, fTableHeight );
	m_oTable.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTable.SetAlpha( 0.0f );
	m_fTableWidth = fTableWidth;

	TFLOAT fRowHeight   = fTableHeight * 0.2f;
	TFLOAT fGradeSize   = fRowHeight * 0.75f;
	TFLOAT fScoreHeight = fRowHeight * 0.98f;

	for ( TINT i = NUM_ROWS - 1; i >= 0; i-- )
	{
		m_aRows[ i ].SetDimensions( fTableWidth, fRowHeight );
		m_aRows[ i ].SetAlpha( 0.0f );
		m_aRows[ i ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		m_aRows[ i ].SetTranslation( 0.0f, TFLOAT( i ) * fRowHeight );
		m_oTable.AddChildTail( m_aRows[ i ] );

		// The name of the game on the left...
		m_aGameNames[ i ].Create( pNameFont, 10.0f );
		m_aGameNames[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		m_aGameNames[ i ].SetTextAlign( T2GUIFont::TextAlign_Left );
		m_aRows[ i ].AddChildTail( m_aGameNames[ i ] );

		// ...the grade on the right...
		m_aGrades[ i ].SetDimensions( fGradeSize, fGradeSize );
		m_aGrades[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
		m_aRows[ i ].AddChildTail( m_aGrades[ i ] );

		// ...and the score left of the grade
		m_aScores[ i ].SetLayout( fScoreHeight, -5.0f, 0.0f );
		m_aScores[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
		m_aGrades[ i ].AddChildTail( m_aScores[ i ] );
	}

	m_oNavigation.AddChildTail( m_oTable );

	// The summary page
	pFont18 = T2GUIFontManager::FindFont( "Rekord18" );
	pFont26 = T2GUIFontManager::FindFont( "Rekord26" );

	TStringManager::String16Format( m_wszNumGames, TARRAYSIZE( m_wszNumGames ), L"%i", 9999 );
	TStringManager::String16Format( m_wszNumPlayed, TARRAYSIZE( m_wszNumPlayed ), L"%i", 9999 );
	TStringManager::String16Format( m_wszNumStars, TARRAYSIZE( m_wszNumStars ), L"%i%%", 1000 );

	TFLOAT fSummaryWidth = fWidth * 0.6f;
	m_oSummary.SetDimensions( fSummaryWidth, ( m_oScoreCard.GetHeight() - m_oNavigation.GetHeight() ) * 0.75f );
	m_oSummary.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oSummary.SetTransform( 0.0f, -24.0f );
	m_oSummary.SetAlpha( 0.0f );
	m_oScoreCard.AddChildTail( m_oSummary );

	// Number of games
	m_oNumGamesRow.SetDimensions( fSummaryWidth, pFont18->GetLineHeight() * 2.0f );
	m_oNumGamesRow.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oNumGamesRow.SetAlpha( 0.0f );
	m_oSummary.AddChildTail( m_oNumGamesRow );

	TFLOAT fLabelWidth = fWidth * 0.75f;
	m_oNumGamesLabel.Create( pFont18, fLabelWidth );
	m_oNumGamesLabel.SetText( ALocaleManager::Interface()->GetString( 1857 ) );
	m_oNumGamesLabel.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oNumGamesLabel.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oNumGamesRow.AddChildTail( m_oNumGamesLabel );

	m_oNumGames.Create( pFont18, fWidth * 0.25f );
	m_oNumGames.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oNumGames.SetTextAlign( T2GUIFont::TextAlign_Right );
	m_oNumGames.SetTextAndFitWidth( m_wszNumGames );
	m_oNumGames.SetColour( 0xFFFF5500 );
	m_oNumGames.SetScale( 1.5f );
	m_oNumGamesRow.AddChildTail( m_oNumGames );

	// Number of played games, below the number of games
	m_oNumPlayedRow.SetDimensions( fSummaryWidth, pFont18->GetLineHeight() * 2.0f );
	m_oNumPlayedRow.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oNumPlayedRow.SetAlpha( 0.0f );
	m_oNumGamesRow.AddChildTail( m_oNumPlayedRow );

	m_oNumPlayedLabel.Create( pFont18, fLabelWidth );
	m_oNumPlayedLabel.SetText( ALocaleManager::Interface()->GetString( 1858 ) );
	m_oNumPlayedLabel.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oNumPlayedLabel.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oNumPlayedLabel.SetTransform( 0.0f, 10.0f );
	m_oNumPlayedRow.AddChildTail( m_oNumPlayedLabel );

	m_oNumPlayed.Create( pFont18, fLabelWidth );
	m_oNumPlayed.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oNumPlayed.SetTextAlign( T2GUIFont::TextAlign_Right );
	m_oNumPlayed.SetTextAndFitWidth( m_wszNumPlayed );
	m_oNumPlayed.SetColour( 0xFFFF5500 );
	m_oNumPlayed.SetScale( 1.5f );
	m_oNumPlayed.SetTransform( 0.0f, 10.0f );
	m_oNumPlayedRow.AddChildTail( m_oNumPlayed );

	// Number of stars, at the bottom
	m_oNumStarsRow.SetDimensions( fSummaryWidth, pFont26->GetLineHeight() * 2.0f );
	m_oNumStarsRow.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oNumStarsRow.SetAlpha( 0.0f );
	m_oSummary.AddChildTail( m_oNumStarsRow );

	m_oNumStarsLabel.Create( pFont18, fLabelWidth );
	m_oNumStarsLabel.SetText( ALocaleManager::Interface()->GetString( 1859 ) );
	m_oNumStarsLabel.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oNumStarsLabel.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oNumStarsRow.AddChildTail( m_oNumStarsLabel );

	m_oNumStars.Create( pFont26, fLabelWidth );
	m_oNumStars.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oNumStars.SetTextAlign( T2GUIFont::TextAlign_Right );
	m_oNumStars.SetTextAndFitWidth( m_wszNumStars );
	m_oNumStars.SetColour( 0xFFFF5500 );
	m_oNumStars.SetScale( 1.5f );
	m_oNumStarsRow.AddChildTail( m_oNumStars );

	m_oBackground.SetAlpha( 0.0f );
	AGUI2::GetRootElement()->AddChildTail( m_oBackground );
	m_oBackground.T2GUIElement::Hide();
}

// $Barnyard: FUNCTION 0044de80
void AGamesPlayedDisplayState::UpdatePage()
{
	if ( m_bOnlyPlayed && m_iRecord < 0 )
		m_iRecord = GetPageRecord( 1 );

	if ( m_bOnlyPlayed && m_iShownRecord == m_iRecord )
		return;

	m_iShownRecord = m_iRecord;
	TStringManager::String16Format( m_wszPageNumber, TARRAYSIZE( m_wszPageNumber ), L"%i/%i", GetCurrentPage(), GetNumPages() );

	// No paging without games to list
	if ( m_iNumShownRecords != 0 )
	{
		m_oLeftArrow.T2GUIElement::Show();
		m_oRightArrow.T2GUIElement::Show();
	}
	else
	{
		m_oLeftArrow.T2GUIElement::Hide();
		m_oRightArrow.T2GUIElement::Hide();
	}

	if ( m_iRecord < 0 )
	{
		// The summary page
		TStringManager::String16Format( m_wszNumGames, TARRAYSIZE( m_wszNumGames ), L"%i", m_iNumRecords );
		TStringManager::String16Format( m_wszNumPlayed, TARRAYSIZE( m_wszNumPlayed ), L"%i", m_iNumPlayedRecords );

		TINT iNumStars = AHighScoreManager::GetNumStars( AHighScoreManager::PLAYER_1 );
		iNumStars += AHighScoreManager::GetNumStars( AHighScoreManager::PLAYER_2 );
		TStringManager::String16Format( m_wszNumStars, TARRAYSIZE( m_wszNumStars ), L"%i", iNumStars );

		m_oSummary.T2GUIElement::Show();
		m_oTable.T2GUIElement::Hide();
	}
	else
	{
		// A page of games
		m_oSummary.T2GUIElement::Hide();
		m_oTable.T2GUIElement::Show();

		for ( TINT i = NUM_ROWS - 1; i >= 0; i-- )
			m_aPageRecords[ i ] = -1;

		m_aPageRecords[ 0 ] = m_iRecord;

		for ( TINT i = 1; i < NUM_ROWS; i++ )
		{
			m_aPageRecords[ i ] = FindNextRecord( m_aPageRecords[ i - 1 ], 1 );

			if ( m_aPageRecords[ i ] < 0 )
				break;
		}

		for ( TINT i = NUM_ROWS - 1; i >= 0; i-- )
			SetRow( i, m_aPageRecords[ i ] );

		m_fFinishScoresTime = 0.5f;
	}
}

// $Barnyard: FUNCTION 0044da50
void AGamesPlayedDisplayState::SetRow( TINT a_iRow, TINT a_iRecord )
{
	if ( a_iRecord < 0 )
	{
		m_aRows[ a_iRow ].T2GUIElement::Hide();
		return;
	}

	AHighScoreManager::Record* pRecord = AHighScoreManager::GetRecord( a_iRecord );
	AGUI2TextBox*              pName   = &m_aGameNames[ a_iRow ];
	AGUI2ScoreTickerAdvanced*  pScore  = &m_aScores[ a_iRow ];

	pName->SetTextAndFitWidth( L"" );
	pScore->Destroy();
	pName->SetScale( 1.0f );

	// The name of the game
	TINT iNameId = pRecord->iShortNameId;
	if ( iNameId >= 0 || ( iNameId = pRecord->iNameId ) >= 0 )
		pName->SetTextAndFitWidth( ALocaleManager::Interface()->GetString( iNameId ) );

	// The score of the player
	TINT iScore = -1;
	for ( TINT i = 0; i < AHighScoreManager::NUM_SCORES; i++ )
	{
		if ( pRecord->aPlayers[ i ] < 0 )
		{
			iScore = i;
			break;
		}
	}

	if ( pRecord->eType == AHighScoreRecord::TYPE_INTEGER_LOW || pRecord->eType == AHighScoreRecord::TYPE_INTEGER_HIGH )
		pScore->CreateInteger( 3, TNULL, -1.0f, -1.0f );
	else if ( pRecord->eType == AHighScoreRecord::TYPE_TIME_LOW || pRecord->eType == AHighScoreRecord::TYPE_TIME_HIGH )
		pScore->CreateTime( 2, 2, 2, TNULL, -1.0f, -1.0f );
	else
		pScore->CreateDecimal( 2, 2, TNULL, -1.0f, -1.0f );

	pScore->Reset();

	AHighScoreManager::GRADE eGrade = AHighScoreManager::GRADE_F;

	if ( iScore < 0 )
	{
		pScore->SetScore( 0.0f );
	}
	else
	{
		eGrade = AHighScoreManager::GetGrade( pRecord->iIndex, pRecord->aScores[ iScore ] );
		pScore->SetScore( pRecord->aScores[ iScore ] );
	}

	pScore->SetSpinning( TTRUE );
	m_aGrades[ a_iRow ].SetTextureSection( T2GUITextureSectionManager::GetTextureSection( s_aGradeTextures[ eGrade ] ) );

	// Make the name fit
	TFLOAT fMaxNameWidth = m_fTableWidth - m_aGrades[ a_iRow ].GetWidth();
	fMaxNameWidth        = fMaxNameWidth - pScore->GetWidth() - 7.0f;

	TFLOAT fNameWidth = pName->GetWidth();
	TFLOAT fScale     = fMaxNameWidth / fNameWidth;

	if ( fNameWidth > fMaxNameWidth && fNameWidth > 0.0f )
	{
		if ( fScale > 0.8f )
			pName->SetScale( fScale );
		else
			pName->SetWidth( fMaxNameWidth );
	}

	m_aRows[ a_iRow ].T2GUIElement::Show();
}

// $Barnyard: FUNCTION 0044dcb0
TINT AGamesPlayedDisplayState::FindNextRecord( TINT a_iRecord, TINT a_iCount ) const
{
	TINT iRecord = a_iRecord + 1;
	if ( iRecord < 0 ) iRecord = 0;

	for ( ; iRecord < AHighScoreManager::NUM_RECORDS; iRecord++ )
	{
		if ( IsRecordShown( iRecord ) && --a_iCount <= 0 )
			return iRecord;
	}

	return -1;
}

// $Barnyard: FUNCTION 0044dd10
TINT AGamesPlayedDisplayState::FindPreviousRecord( TINT a_iRecord, TINT a_iCount ) const
{
	for ( TINT iRecord = a_iRecord - 1; iRecord >= 0; iRecord-- )
	{
		if ( IsRecordShown( iRecord ) && --a_iCount <= 0 )
			return iRecord;
	}

	return -1;
}

// $Barnyard: FUNCTION 0044dd70
TINT AGamesPlayedDisplayState::GetCurrentPage() const
{
	if ( m_iRecord < 0 )
		return 0;

	TINT iNumRecords = 0;

	for ( TINT i = 0; i <= m_iRecord; i++ )
	{
		if ( IsRecordShown( i ) )
			iNumRecords++;
	}

	return TMath::CeilToInt( TFLOAT( iNumRecords ) / TFLOAT( m_iRecordsPerPage ) );
}

// $Barnyard: FUNCTION 0044dde0
TINT AGamesPlayedDisplayState::GetPageRecord( TINT a_iPage ) const
{
	if ( a_iPage <= 0 || a_iPage > GetNumPages() )
		return -1;

	TINT iNumRecords = 0;

	for ( TINT i = 0; i < AHighScoreManager::NUM_RECORDS; i++ )
	{
		if ( IsRecordShown( i ) && TMath::CeilToInt( TFLOAT( ++iNumRecords ) / TFLOAT( m_iRecordsPerPage ) ) == a_iPage )
			return i;
	}

	return -1;
}

// NOTE: inlined in the original
TBOOL AGamesPlayedDisplayState::IsRecordShown( TINT a_iRecord ) const
{
	AHighScoreManager::Record* pRecord = AHighScoreManager::GetRecord( a_iRecord );
	return pRecord->iHidden == 0 && ( pRecord->bPlayed || !m_bOnlyPlayed );
}
