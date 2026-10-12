#include "pch.h"
#include "AStatuesHud.h"
#include "Locale/ALocaleManager.h"
#include "Locale/ALocale.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: these are globals in the original (never written)
static TFLOAT s_fKeySize    = 24.0f; // 0x007738b0, size of a key icon and height of the pose names
static TFLOAT s_fKeyOffsetX = 96.0f; // 0x007738b4
static TFLOAT s_fKeySpacing = 32.0f; // 0x007738b8

// $Barnyard: FUNCTION 00485e40
AStatuesHud::AStatuesHud()
    : m_pFont( TNULL )
    , m_pFont2( TNULL )
    , m_bCreated( TFALSE )
{
}

// $Barnyard: FUNCTION 00485ca0
AStatuesHud::~AStatuesHud()
{
	if ( m_bCreated )
		Destroy();
}

// $Barnyard: FUNCTION 00484af0
void AStatuesHud::Create( T2GUIElement* a_pParent )
{
	m_iNumStars   = 0;
	m_iCurrentRow = -1;
	m_iCurrentKey = 0;
	m_pFont       = T2GUIFontManager::FindFont( "rekord26" );
	m_pFont2      = T2GUIFontManager::FindFont( "Rekord26" );

	// Panel with the stars of the current pose
	T2GUITextureSection* pPanelTexSection = T2GUITextureSectionManager::GetTextureSection( "5_Star_Panel" );
	TFLOAT               fPanelWidth      = pPanelTexSection->GetWidth() * 1.5f;
	TFLOAT               fPanelHeight     = pPanelTexSection->GetHeight() * 1.5f;

	m_oStarPanel.Create( fPanelWidth, fPanelHeight );
	m_oStarPanel.SetTextureSection( pPanelTexSection );
	m_oStarPanel.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oStarPanel.Hide();
	a_pParent->AddChildTail( m_oStarPanel );

	T2GUITextureSection* pStarTexSection = T2GUITextureSectionManager::GetTextureSection( "Statue_GoldStar" );
	TFLOAT               fStarWidth      = pStarTexSection->GetWidth() * 0.5f;
	TFLOAT               fStarHeight     = pStarTexSection->GetHeight() * 0.5f;

	for ( TINT i = 0; i < MAX_STARS; i++ )
	{
		m_aStars[ i ].Create( fStarWidth, fStarHeight );
		m_aStars[ i ].SetTextureSection( pStarTexSection );
		m_aStars[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
		m_aStars[ i ].SetTransform( TFLOAT( i - 2 ) * fPanelWidth * 0.185f, 0.0f );
		m_oStarPanel.AddChildTail( m_aStars[ i ] );
		m_aStars[ i ].Hide();

		// "+%d" popup of the star
		m_aStarTexts[ i ].SetFont( m_pFont2 );
		m_aStarTexts[ i ].SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
		m_aStarTexts[ i ].SetColour( 0xFFFFFF00 );
		m_aStarTexts[ i ].Hide();
		m_aStarStrings[ i ] = "";
		m_aStarTexts[ i ].SetText( m_aStarStrings[ i ] );
		m_aStars[ i ].AddChildTail( m_aStarTexts[ i ] );
	}

	m_oStarPanel.SetTransform( 0.0f, fStarHeight * -0.5f );

	// Timer
	m_oTimer.Create( 0.0f, 0.0f );
	TFLOAT fTimerHeight = m_oTimer.GetHeight() * 0.75f;
	TFLOAT fTimerWidth  = m_oTimer.GetWidth() * 0.75f;
	m_oTimer.SetDimensions( fTimerWidth, fTimerHeight );

	// Score
	m_oScoreTicker.Create( 3, "darts_wheel_window", 1.0f, 8.0f );
	m_oScoreTicker.Show();
	m_oScoreTicker.SetSpinning( TFALSE );
	m_oScoreTicker.SetScore( 0 );

	T2GUITextureSection* pWoodTexSection = T2GUITextureSectionManager::GetTextureSection( "Score_Wood" );
	TFLOAT               fWoodHeight     = pWoodTexSection->GetHeight() * 1.1f;
	m_oScoreWood.Create( m_oTimer.GetWidth() * 0.5f + pWoodTexSection->GetWidth(), fWoodHeight );
	m_oScoreWood.SetTextureSection( pWoodTexSection );
	m_oScoreWood.AddChildTail( m_oScoreTicker );

	m_oScorePanel.AddChildTail( m_oScoreWood );
	m_oScorePanel.AddChildTail( m_oTimer );

	TFLOAT fWoodPanelHeight = m_oScoreWood.GetHeight();
	m_oScorePanel.SetDimensions( m_oScoreWood.GetWidth(), fWoodPanelHeight );

	m_oScoreWood.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oScoreTicker.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oScoreTicker.SetTransform( m_oTimer.GetWidth(), -2.0f );

	// Squash the ticker vertically to fit the wood.
	// NOTE: the original divides the unscaled height of the "Score_Wood" section
	TFLOAT fTickerScale = pWoodTexSection->GetHeight() / ( m_oScoreTicker.GetHeight() * 1.2f );

	T2GUITransform oScale;
	oScale.m_aMatrixRows[ 0 ] = TVector2( 1.0f, 0.0f );
	oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, fTickerScale );
	oScale.m_vecTranslation   = TVector2( 0.0f, 0.0f );
	m_oScoreTicker.GetTransform().PreMultiply( oScale );

	m_oTimer.SetTransform( 0.0f, m_oTimer.GetHeight() * 0.1f );
	m_oScorePanel.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oTimer.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oScoreWood.SetWidth( m_oScoreWood.GetWidth() * 1.1f );
	m_oScoreWood.SetTransform( 3.0f, 12.0f );
	a_pParent->AddChildTail( m_oScorePanel );

	// Input panel with the three rows of keys
	T2GUITextureSection* pInputTexSection = T2GUITextureSectionManager::GetTextureSection( "Statue_Input_base" );
	m_oInputPanel.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_oInputPanel.SetDimensions( 384.0f, 96.0f );
	a_pParent->AddChildTail( m_oInputPanel );

	m_oInputBase.Create( 384.0f, 96.0f );
	m_oInputBase.SetTextureSection( pInputTexSection );
	m_oInputBase.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oInputBase.SetAlpha( 0.7f );
	m_oInputPanel.AddChildTail( m_oInputBase );

	m_oCompletedPoseName.SetFont( m_pFont );
	m_oCompletedPoseName.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oInputPanel.AddChildTail( m_oCompletedPoseName );

	m_bInputHidden = TFALSE;

	for ( TINT iRow = 0; iRow < NUM_ROWS; iRow++ )
	{
		TFLOAT fRowY = ( TFLOAT( iRow ) - 1.0f ) * 32.0f;

		m_aPoseNames[ iRow ].SetFont( m_pFont );
		m_aPoseNames[ iRow ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
		m_aPoseNames[ iRow ].SetTransform( -96.0f, fRowY * 0.5f );
		m_aPoseNames[ iRow ].SetColour( 0xFFFFFFFF );
		m_aPoseNames[ iRow ].SetScale( s_fKeySize / m_pFont->GetLineHeight() * 0.5f );
		m_oInputBase.AddChildTail( m_aPoseNames[ iRow ] );

		for ( TINT i = 0; i < MAX_KEYS; i++ )
		{
			T2GUIRectangle& rKey = m_aKeyRows[ iRow ].aKeys[ i ];

			rKey.Create( s_fKeySize, s_fKeySize );
			rKey.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
			rKey.SetColour( 0xFFFFFFFF );
			rKey.SetTransform( TFLOAT( i - 2 ) * s_fKeySpacing + s_fKeyOffsetX, fRowY );
			m_oInputBase.AddChildTail( rKey );
			rKey.Hide();
		}
	}

	// Instructions
	m_oInstructions.Create( m_pFont, 256.0f );
	m_oInstructions.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
	m_oInstructions.SetTransform( -8.0f, -8.0f );
	a_pParent->AddChildTail( m_oInstructions );

	ALocale::FormatInputTags( m_wszInstructions, 64, 1017 );

	m_oInstructions.SetText( m_wszInstructions );
	m_oInstructions.SetScale( 16.0f / m_pFont->GetLineHeight() );

	m_bCreated = TTRUE;
}

// $Barnyard: FUNCTION 00485170
void AStatuesHud::Destroy()
{
	for ( TINT i = 0; i < NUM_ROWS; i++ )
		m_aPoseNames[ i ].RemoveSelf();

	m_oCompletedPoseName.RemoveSelf();
	m_oInstructions.RemoveSelf();
	m_oScoreWood.RemoveSelf();
	m_oStarPanel.RemoveSelf();
	m_oInputBase.RemoveSelf();

	for ( TINT i = 0; i < MAX_STARS; i++ )
		m_aStars[ i ].RemoveSelf();

	m_oTimer.RemoveSelf();
	m_oScoreTicker.RemoveSelf();

	T2GUIFontManager::ReleaseFont( m_pFont );
	T2GUIFontManager::ReleaseFont( m_pFont2 );
	m_bCreated = TFALSE;
}

// $Barnyard: FUNCTION 00485220
void AStatuesHud::Update( TFLOAT a_fDeltaTime )
{
	TFLOAT fStarWidth  = T2GUITextureSectionManager::GetTextureSection( "Statue_GoldStar" )->GetWidth() * 0.8f;
	TFLOAT fStarHeight = T2GUITextureSectionManager::GetTextureSection( "Statue_GoldStar" )->GetHeight() * 0.8f;
	TFLOAT fInvWidth   = 1.0f / fStarWidth;
	TFLOAT fShrink     = a_fDeltaTime + a_fDeltaTime;

	// Shrink the popped up stars back to their size and fade the "+%d" texts out
	for ( TINT i = 0; i < MAX_STARS; i++ )
	{
		TFLOAT fScale;
		TFLOAT fHeight;
		m_aStars[ i ].GetDimensions( fScale, fHeight );
		fScale = fInvWidth * fScale;

		// The fifth star pops up the next one while it shrinks
		if ( m_iNumStars == 5 && i != 4 && fScale > 1.3f && fScale - 1.3f <= fShrink )
			m_aStars[ i + 1 ].SetDimensions( fStarWidth * 1.5f, fStarHeight * 1.5f );

		fScale = TMath::Max( fScale - fShrink, 1.0f );
		m_aStars[ i ].SetDimensions( fScale * fStarWidth, fScale * fStarHeight );

		if ( m_aStarTexts[ i ].IsVisible() )
		{
			TFLOAT fAlpha = m_aStarTexts[ i ].GetAlpha() - a_fDeltaTime * 0.75f;

			if ( fAlpha <= 0.0f )
			{
				m_aStarTexts[ i ].Hide();
				fAlpha = 1.0f;
			}

			m_aStarTexts[ i ].SetAlpha( fAlpha );
		}
	}

	// Slide the input panel out while it's hidden and back in when it's visible
	static TFLOAT s_fInputSlideSpeed = m_oInputBase.GetWidth() * 4.0f;

	TFLOAT fInputX = TFLOAT( m_bInputHidden ? 1 : -1 ) * s_fInputSlideSpeed * a_fDeltaTime + m_oInputBase.GetTransform().GetTranslation().x;
	TFLOAT fMaxX   = m_oInputBase.GetWidth() + 16.0f;
	TMath::Clip( fInputX, 0.0f, fMaxX );
	m_oInputBase.SetTransform( fInputX, 0.0f );

	if ( m_bInputHidden )
	{
		// Fly the name of the completed pose to the centre of the panel and scale it up
		m_oCompletedPoseName.Show();

		const TVector2& rcPos = m_oCompletedPoseName.GetTransform().GetTranslation();
		TFLOAT          fX    = rcPos.x;
		TFLOAT          fY    = rcPos.y;
		TFLOAT          fAbsX = TMath::Abs( fX );
		TFLOAT          fTime = fAbsX * ( 1.0f / 128.0f );
		TFLOAT          fNewX = fX;
		TFLOAT          fNewY = fY;

		if ( fX != 0.0f )
		{
			TFLOAT fStep = a_fDeltaTime * 128.0f;

			if ( fStep <= fAbsX )
			{
				fNewY = fY - TMath::Sign( fY ) * TMath::Abs( fY / fX ) * a_fDeltaTime * 128.0f;
				fNewX = fX - TMath::Sign( fX ) * fStep;
			}
			else
			{
				fNewX = 0.0f;
				fNewY = 0.0f;
			}
		}

		m_oCompletedPoseName.SetTransform( fNewX, fNewY );

		if ( fTime > 0.0f )
		{
			TFLOAT fScale = m_oCompletedPoseName.GetScale();
			m_oCompletedPoseName.SetScale( TMath::Min( ( 1.0f - fScale ) / fTime * a_fDeltaTime + fScale, 1.0f ) );
		}
	}
	else
	{
		m_oCompletedPoseName.Hide();

		for ( TINT i = 0; i < NUM_ROWS; i++ )
			m_aPoseNames[ i ].Show();
	}
}

// $Barnyard: FUNCTION 00485630
void AStatuesHud::Hide()
{
	m_oScoreWood.Hide();
	m_oStarPanel.Hide();
	m_oInputBase.Hide();
	m_oInstructions.Hide();
	m_oCompletedPoseName.Hide();
	m_oTimer.SetVisible( TFALSE );
	m_oScoreTicker.Hide();
}

// $Barnyard: FUNCTION 00485670
void AStatuesHud::Show()
{
	m_oScoreWood.Show();
	m_oStarPanel.Show();
	m_oInputBase.Show();
	m_oStarPanel.SetVisible( m_bStarPanelVisible == TTRUE );
	m_oInstructions.Show();
	m_oCompletedPoseName.SetVisible( m_bInputHidden == TTRUE );
	m_oTimer.SetVisible( TTRUE );
	m_oScoreTicker.Show();
}

// $Barnyard: FUNCTION 004856e0
void AStatuesHud::SetNumStars( TINT a_iNumStars, TINT a_iPoints )
{
	for ( TINT i = 0; i < MAX_STARS; i++ )
		m_aStars[ i ].SetVisible( i < a_iNumStars );

	if ( a_iNumStars > m_iNumStars )
	{
		// Pop the new star up, Update shrinks it back
		TFLOAT fSize  = T2GUITextureSectionManager::GetTextureSection( "Statue_GoldStar" )->GetWidth();
		TINT   iIndex = a_iNumStars - 1;
		m_aStars[ iIndex ].SetDimensions( fSize, fSize );

		if ( a_iPoints != 0 )
		{
			m_aStarStrings[ iIndex ].Format( L"+%d", a_iPoints );
			m_aStarTexts[ iIndex ].SetText( m_aStarStrings[ iIndex ] );
			m_aStarTexts[ iIndex ].Show();
			m_aStarTexts[ iIndex ].SetAlpha( 1.0f );
		}

		// The fifth star starts a chain of pops from the first one
		if ( a_iNumStars == 5 )
			m_aStars[ 0 ].SetDimensions( fSize * 0.75f, fSize * 0.75f );
	}

	m_iNumStars = a_iNumStars;
}

// $Barnyard: FUNCTION 004857d0
void AStatuesHud::SetStarPanel( TINT a_iNumStars )
{
	if ( a_iNumStars != 0 )
	{
		TString8 strTexSection( 16 );
		strTexSection.Format( "%d_Star_Panel", a_iNumStars );

		m_oStarPanel.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( strTexSection ) );
		m_oStarPanel.Show();
		m_bStarPanelVisible = TTRUE;
	}
	else
	{
		m_oStarPanel.Hide();
		m_bStarPanelVisible = TFALSE;
	}
}

// $Barnyard: FUNCTION 004858a0
void AStatuesHud::SetKeyIcon( TINT a_iRow, TINT a_iIndex, STATUESKEY a_eKey, TBOOL a_bPressed )
{
	TString8 strTexSection( 16 );

	switch ( a_eKey )
	{
		case STATUESKEY_UP:
			strTexSection = "3_Key";
			break;
		case STATUESKEY_LEFT:
			strTexSection = "1_Key";
			break;
		case STATUESKEY_RIGHT:
			strTexSection = "2_Key";
			break;
		case STATUESKEY_DOWN:
			strTexSection = "Space_Key";
			break;
		case STATUESKEY_NONE:
			m_aKeyRows[ a_iRow ].aKeys[ a_iIndex ].Hide();
			return;
		default:
			return;
	}

	T2GUIRectangle& rKey = m_aKeyRows[ a_iRow ].aKeys[ a_iIndex ];
	rKey.Show();

	TFLOAT fWidth;
	TFLOAT fHeight;
	m_oInputBase.GetDimensions( fWidth, fHeight );

	// The pressed keys are bigger
	if ( a_bPressed )
		fHeight *= 1.5f;

	TFLOAT fSize = fHeight * 0.25f;
	rKey.SetDimensions( fSize, fSize );
	rKey.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( strTexSection ) );
}

// $Barnyard: FUNCTION 004859f0
void AStatuesHud::SetRowPose( TINT a_iRow, AStatuesPose* a_pPose )
{
	for ( TINT i = 0; i < MAX_KEYS; i++ )
		SetKeyIcon( a_iRow, i, a_pPose->aKeys[ i ], TFALSE );

	if ( a_pPose->iLocaleName >= 0 )
	{
		m_aPoseNames[ a_iRow ].SetText( g_pLocaleManager->GetString( a_pPose->iLocaleName ) );
		m_aPoseNames[ a_iRow ].SetColour( 0xFFFFFF00 );
	}

	m_apRowPoses[ a_iRow ] = a_pPose;

	for ( TINT i = 0; i < NUM_ROWS; i++ )
		m_aPoseNames[ i ].SetAlpha( 1.0f );
}

// $Barnyard: FUNCTION 00485a80
void AStatuesHud::ResetRows()
{
	for ( TINT i = 0; i < NUM_ROWS; i++ )
		SetRowPose( i, m_apRowPoses[ i ] );

	m_iCurrentRow = -1;
}

// $Barnyard: FUNCTION 00485ac0
AStatuesPose* AStatuesHud::HandleKey( STATUESKEY a_eKey )
{
	// NOTE: only Up, Left and Right can start or continue a combo, anything else resets the rows
	if ( a_eKey < STATUESKEY_DOWN )
	{
		// The first key chooses the row
		if ( m_iCurrentRow == -1 )
		{
			m_iCurrentRow = a_eKey;
			m_iCurrentKey = 0;
		}

		if ( m_apRowPoses[ m_iCurrentRow ]->aKeys[ m_iCurrentKey ] == a_eKey )
		{
			// Highlight the row being typed and dim the others
			for ( TINT i = 0; i < NUM_ROWS; i++ )
			{
				if ( i == m_iCurrentRow )
					m_aPoseNames[ i ].SetColour( 0xFFFFFFFF );
				else
					m_aPoseNames[ i ].SetAlpha( 0.5f );
			}

			SetKeyIcon( m_iCurrentRow, m_iCurrentKey, a_eKey, TTRUE );
			m_iCurrentKey++;

			if ( m_iCurrentKey < MAX_KEYS && m_apRowPoses[ m_iCurrentRow ]->aKeys[ m_iCurrentKey ] != STATUESKEY_NONE )
				return TNULL;

			// The combo is complete, the name of the pose flies from its row to the centre of the panel
			m_oCompletedPoseName.Show();
			m_oCompletedPoseName.SetText( g_pLocaleManager->GetString( m_apRowPoses[ m_iCurrentRow ]->iLocaleName ) );
			m_oCompletedPoseName.SetScale( s_fKeySize / m_pFont->GetLineHeight() * 0.5f );

			TFLOAT fRowY = ( TFLOAT( m_iCurrentRow ) - 1.0f ) * 32.0f;

			TFLOAT fNameWidth;
			TFLOAT fNameHeight;
			m_oCompletedPoseName.GetDimensions( fNameWidth, fNameHeight );

			m_oCompletedPoseName.SetTransform( m_oInputPanel.GetWidth() * 0.25f - 96.0f - fNameWidth * 0.25f, fRowY * 0.5f );
			m_aPoseNames[ m_iCurrentRow ].Hide();

			TINT iRow     = m_iCurrentRow;
			m_iCurrentKey = 0;
			m_iCurrentRow = -1;

			return m_apRowPoses[ iRow ];
		}
	}

	ResetRows();
	return &m_oNoPose;
}
