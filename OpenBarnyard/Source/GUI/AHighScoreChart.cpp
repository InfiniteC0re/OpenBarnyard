#include "pch.h"
#include "AHighScoreChart.h"
#include "AGUI2.h"
#include "Helpers/AHighScoreManager.h"
#include "Locale/ALocaleManager.h"
#include "MiniGame/AMiniGameManager.h"
#include "Player/APlayerProfileManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AHighScoreChart ) == 0x367c );

//-----------------------------------------------------------------------------
// Dead code: a table of four GUI effect callbacks at 0x00739e6c (right after the
// AGUI2FloatHudElement vtable, in this file's data). Nothing references the table
// or the functions in retail and the owner of the state they animate is unknown.
// Each callback updates a_pState and returns TTRUE when the effect is over.
// NOTE: all the names are invented.
//-----------------------------------------------------------------------------
struct AHighScoreEffectState
{
	TFLOAT                unk0;       // 0x00
	TFLOAT                fTime;      // 0x04
	TFLOAT                unk8;       // 0x08
	TFLOAT                fAlpha;     // 0x0c
	Toshi::T2GUITransform oTransform; // 0x10
};

// Waits until fTime runs out, then resets the transform
// $Barnyard: FUNCTION 0063ad30
static TBOOL HighScoreEffect_Wait( void* a_pUnused, AHighScoreEffectState* a_pState, TFLOAT a_fDeltaTime )
{
	a_pState->fTime -= a_fDeltaTime;

	if ( a_pState->fTime >= 0.0f )
		return TFALSE;

	a_pState->oTransform.SetIdentity();
	a_pState->oTransform.SetTranslation( 0.0f, 0.0f );
	a_pState->fTime = 0.0f;
	return TTRUE;
}

// Pulses fAlpha forever (|cos| of a time running at 4 rad/s)
// $Barnyard: FUNCTION 0063ad80
static TBOOL HighScoreEffect_Pulse( void* a_pUnused, AHighScoreEffectState* a_pState, TFLOAT a_fDeltaTime )
{
	a_pState->fTime += a_fDeltaTime * 4.0f;
	TMath::Wrap( a_pState->fTime, 0.0f, TMath::PI );

	TFLOAT fAlpha = TMath::Abs( TMath::Cos( a_pState->fTime ) );
	TMath::Clip( fAlpha, 0.0f, 1.0f );
	a_pState->fAlpha = fAlpha;

	return TFALSE;
}

// Same as HighScoreEffect_Pulse, but ends when fAlpha gets to 0
// $Barnyard: FUNCTION 0063add0
static TBOOL HighScoreEffect_PulseOut( void* a_pUnused, AHighScoreEffectState* a_pState, TFLOAT a_fDeltaTime )
{
	a_pState->fTime += a_fDeltaTime * 4.0f;
	TMath::Wrap( a_pState->fTime, 0.0f, TMath::PI );

	TFLOAT fAlpha = TMath::Abs( TMath::Cos( a_pState->fTime ) );
	TMath::Clip( fAlpha, 0.0f, 1.0f );
	a_pState->fAlpha = fAlpha;

	return fAlpha < 0.01f;
}

// The fourth entry is the shared RETURNS_FALSE stub (006528c0)
static TBOOL HighScoreEffect_None( void* a_pUnused, AHighScoreEffectState* a_pState, TFLOAT a_fDeltaTime )
{
	return TFALSE;
}

using t_HighScoreEffect = TBOOL ( * )( void* a_pUnused, AHighScoreEffectState* a_pState, TFLOAT a_fDeltaTime );

[[maybe_unused]] static const t_HighScoreEffect s_aHighScoreEffects[] = {
	HighScoreEffect_Wait,
	HighScoreEffect_Pulse,
	HighScoreEffect_PulseOut,
	HighScoreEffect_None,
}; // 0x00739e6c

// Texture sections of the grades (AHighScoreManager::GRADE)
static const TCHAR* s_aGradeTextures[ AHighScoreManager::GRADE_NUMOF ] = {
	"Grade_Star",
	"Grade_A",
	"Grade_B",
	"Grade_C",
	"Grade_D",
	"Grade_F",
}; // 0x00779820

// $Barnyard: FUNCTION 0063ae40
AHighScoreChart::AHighScoreChart()
{
	m_bUnk16C     = TFALSE;
	m_iBlinkRank  = -1;
	m_fBlinkTimer = 0.0f;
}

// $Barnyard: FUNCTION 0063b2d0
AHighScoreChart::~AHighScoreChart()
{
	Unlink();
}

// $Barnyard: FUNCTION 0063afe0
void AHighScoreChart::Unlink()
{
	m_oTitle.RemoveSelf();
	m_oTitleSuffix.RemoveSelf();
	m_oBoard.RemoveSelf();
	m_oIcon.RemoveSelf();
	m_oShortName.RemoveSelf();

	for ( TINT i = 0; i < AHighScoreRecord::NUM_SCORES; i++ )
	{
		m_aRankItems[ i ].RemoveSelf();
		m_aFaceItems[ i ].RemoveSelf();
		m_aNameItems[ i ].RemoveSelf();
		m_aScoreItems[ i ].RemoveSelf();
		m_aGradeItems[ i ].RemoveSelf();
		m_aBackings[ i ].RemoveSelf();
	}

	m_oGrid.RemoveSelf();
}

// $Barnyard: FUNCTION 0063b440
TBOOL AHighScoreChart::Create()
{
	AHighScoreManager::GetSingleton()->HideChart();

	m_oGrid.CreateGrid( 5, 5, 5.0f, 5.0f );

	T2GUIFontManager::FindFont( "Rekord18" );
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	m_oShortName.Create( pFont, 0.0f );
	m_oShortName.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oGrid.AddChildTail( m_oShortName );
	m_oGrid.GetTransform().SetTranslation( 0.0f, -10.0f );

	for ( TINT i = 0; i < AHighScoreRecord::NUM_SCORES; i++ )
	{
		// The rows: rank, face, name, score and grade
		m_aBackings[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		m_aRankItems[ i ].AddChildTail( m_aBackings[ i ] );

		m_aRankItems[ i ].Create( pFont, 0.0f, TNULL, 0.0f );
		m_oGrid.AddMenuGridItem( m_aRankItems[ i ] );
		m_oGrid.AddMenuGridItem( m_aFaceItems[ i ] );

		m_aNameItems[ i ].Create( pFont, 0.0f, TNULL, 0.0f );
		m_oGrid.AddMenuGridItem( m_aNameItems[ i ] );
		m_oGrid.AddMenuGridItem( m_aScoreItems[ i ] );
		m_oGrid.AddMenuGridItem( m_aGradeItems[ i ] );
	}

	AGUI2::GetRootElement()->AddChildTail( m_oGrid );
	m_oGrid.Hide();

	m_oGrid.SetPaddingLeft( 10.0f );
	m_oGrid.SetColumnWidth( 0, 0.2f );
	m_oGrid.SetColumnWidth( 1, 0.7f );
	m_oGrid.SetColumnWidth( 2, 1.1f );
	m_oGrid.SetColumnWidth( 3, 1.0f );
	m_oGrid.SetColumnWidth( 4, 0.5f );

	return TTRUE;
}

// $Barnyard: FUNCTION 0063b110
void AHighScoreChart::SetVisible( TBOOL a_bVisible )
{
	m_oBoard.SetVisible( a_bVisible == TTRUE );
	m_oGrid.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 0063b280
void AHighScoreChart::Clear()
{
	for ( TINT i = 0; i < AHighScoreRecord::NUM_SCORES; i++ )
	{
		m_aRankItems[ i ].SetText( TNULL );
		m_aNameItems[ i ].SetText( TNULL );
		m_aScoreItems[ i ].GetTicker().Destroy();
	}
}

// $Barnyard: FUNCTION 0063b180
void AHighScoreChart::Update( TFLOAT a_fDeltaTime )
{
	if ( m_iBlinkRank == -1 )
		return;

	m_fBlinkTimer += a_fDeltaTime;

	if ( m_fBlinkTimer < ms_fBlinkTime )
		return;

	m_fBlinkTimer = 0.0f;

	// The rank and the name of the last added score blink
	AGUI2TextBox& rRank = m_aRankItems[ m_iBlinkRank ].GetTextBox();
	rRank.SetColour( ( rRank.GetColour() == 0xffffffff ) ? 0xffffc800 : 0xffffffff );

	AGUI2TextBox& rName = m_aNameItems[ m_iBlinkRank ].GetTextBox();
	rName.SetColour( ( rName.GetColour() == 0xffffffff ) ? 0xffffc800 : 0xffffffff );
}

// $Barnyard: FUNCTION 0063b610
T2GUITextureSection* AHighScoreChart::GetPlayerTexture( TINT a_iPlayer )
{
	switch ( a_iPlayer )
	{
		case AHighScoreManager::PLAYER_1:
		case AHighScoreManager::PLAYER_2:
			return T2GUITextureSectionManager::GetTextureSection( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ) );
		case 130:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Peck" );
		case 131:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Root" );
		case 132:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Duke" );
		case 133:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Otis" );
		case 134:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Bessy" );
		case 135:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Pig" );
		case 136:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Freddy" );
		case 138:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Miles" );
		case 139:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Ben" );
		case 141:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Eddy" );
		case 142:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Bud" );
		case 143:
			return T2GUITextureSectionManager::GetTextureSection( "Score_Igg" );
		default:
			return T2GUITextureSectionManager::GetTextureSection( "Place_Holder" );
	}
}

// $Barnyard: FUNCTION 0063b7c0
void AHighScoreChart::Show( AHighScoreRecord* a_pRecord, TBOOL a_bUnused )
{
	T2GUIFontManager::FindFont( "Rekord18" );
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	m_oBoard.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Result_Wood" ) );

	// The name of the game
	m_oTitle.Create( pFont, pFont->GetTextWidth( ALocaleManager::Interface()->GetString( a_pRecord->iNameId ) ) );
	m_oTitle.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oTitle.SetText( ALocaleManager::Interface()->GetString( a_pRecord->iNameId ) );
	m_oTitle.SetColour( 0xffffffff );
	m_oTitle.GetTransform().SetIdentity();
	m_oTitleSuffix.GetTransform().SetIdentity();
	m_oBoard.GetTransform().SetIdentity();

	const TWCHAR* wszSuffix = ALocaleManager::Interface()->GetString( 277 );
	m_oTitleSuffix.Create( pFont, pFont->GetTextWidth( wszSuffix ) );
	m_oTitleSuffix.SetText( wszSuffix );
	m_oTitleSuffix.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oTitleSuffix.SetColour( 0xfffff0af );
	m_oTitle.AddChildTail( m_oTitleSuffix );

	static TFLOAT s_fBoardWidth = AGUI2::GetRootElement()->GetWidth() * 0.8f; // 0x007b5050

	m_oBoard.AddChildTail( m_oTitle );
	m_oBoard.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_oBoard.SetWidth( s_fBoardWidth );

	T2GUIElement* pRoot = AGUI2::GetRootElement();
	m_oBoard.SetTransform( pRoot->GetWidth() * -0.02f, pRoot->GetHeight() * 0.02f );
	m_oTitle.SetTransform( m_oBoard.GetWidth() * 0.05f, 0.0f );
	m_oTitleSuffix.SetTransform( 25.0f, 0.0f );

	// Fits the name in the board
	TFLOAT fTitleScale = m_oBoard.GetWidth() / ( m_oTitleSuffix.GetWidth() + m_oTitle.GetWidth() + 25.0f ) * 0.9f;
	TMath::Clip( fTitleScale, 0.0f, 1.0f );
	m_oTitle.GetTransform().Scale( fTitleScale, fTitleScale );

	AGUI2::GetRootElement()->AddChildTail( m_oBoard );

	// The icon of the minigame
	TINT iTitle = a_pRecord->iNameId;

	if ( iTitle == 785 )
		iTitle = 619;

	TINT            iMiniGame = AMiniGameManager::GetSingleton()->FindMiniGameByTitle( iTitle );
	T2GUIATTACHMENT eBoardAttachment;

	if ( iMiniGame == -1 )
	{
		m_oIcon.RemoveSelf();
		eBoardAttachment = T2GUIATTACHMENT_TOPCENTER;
	}
	else
	{
		m_oIcon.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( AMiniGameManager::GetSingleton()->GetMiniGame( iMiniGame )->GetIconName() ) );
		m_oIcon.GetTransform().SetTranslation( -10.0f, 0.0f );
		m_oIcon.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPRIGHT );
		m_oBoard.AddChildTail( m_oIcon );
		eBoardAttachment = T2GUIATTACHMENT_TOPRIGHT;
	}

	m_oBoard.SetAttachment( eBoardAttachment, eBoardAttachment );

	// The short name over the rows
	if ( a_pRecord->iShortNameId < 0 )
	{
		m_oShortName.Hide();
	}
	else
	{
		const TWCHAR* wszShortName = ALocaleManager::Interface()->GetString( a_pRecord->iShortNameId );
		m_oShortName.Create( pFont, pFont->GetTextWidth( wszShortName ) );
		m_oShortName.SetText( wszShortName );
		m_oShortName.Show();
	}

	m_oGrid.SetMaxItemWidth( pFont->GetTextWidth( L"WWWWWWW" ) );

	for ( TINT i = 0; i < AHighScoreRecord::NUM_SCORES; i++ )
	{
		m_aBackings[ i ].SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "ScoreBacking" ) );

		// Rank
		TStringManager::String16Format( m_awszRanks[ i ], 7, L"%i", i + 1 );
		m_aRankItems[ i ].Create( pFont, pFont->GetTextWidth( m_awszRanks[ i ] ), m_awszRanks[ i ], 0.0f );
		m_aRankItems[ i ].GetTextBox().SetColour( 0xffffc800 );
		m_aRankItems[ i ].GetTextBox().SetTextAlign( T2GUIFont::TextAlign_Center );
		m_oGrid.UpdateMaxItemDimensions( m_aRankItems[ i ] );

		// Face
		m_aFaceItems[ i ].Create( GetPlayerTexture( a_pRecord->aPlayers[ i ] ), 50.0f, 50.0f );
		m_oGrid.UpdateMaxItemDimensions( m_aFaceItems[ i ] );

		// Name, the human players have the name of the profile
		const TWCHAR* wszName;

		if ( a_pRecord->aPlayers[ i ] > 0 )
			wszName = ALocaleManager::Interface()->GetString( a_pRecord->aPlayers[ i ] );
		else
			wszName = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString();

		TFLOAT fNameScale = m_oGrid.GetMaxItemWidth() / pFont->GetTextWidth( wszName );
		TMath::Clip( fNameScale, 0.0f, 1.0f );

		m_aNameItems[ i ].Create( pFont, pFont->GetTextWidth( wszName ), wszName, 0.0f );

		T2GUITransform oNameScale;
		oNameScale.Set( TVector2( fNameScale, 0.0f ), TVector2( 0.0f, fNameScale ), TVector2::VEC_ZERO );

		AGUI2TextBox& rNameBox = m_aNameItems[ i ].GetTextBox();
		rNameBox.GetTransform().SetIdentity();
		rNameBox.GetTransform().PreMultiply( oNameScale );
		rNameBox.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		rNameBox.SetColour( 0xffffc800 );
		rNameBox.SetTextAlign( T2GUIFont::TextAlign_Left );
		m_oGrid.UpdateMaxItemDimensions( m_aNameItems[ i ] );

		// Score
		m_aScoreItems[ i ].Create( TINT( a_pRecord->aScores[ i ] ) );
		m_oGrid.UpdateMaxItemDimensions( m_aScoreItems[ i ] );

		// Grade
		AHighScoreManager::GRADE eGrade = AHighScoreManager::GetGrade( a_pRecord->iIndex, a_pRecord->aScores[ i ] );
		m_aGradeItems[ i ].Create( T2GUITextureSectionManager::GetTextureSection( s_aGradeTextures[ eGrade ] ), 50.0f, 50.0f );
		m_oGrid.UpdateMaxItemDimensions( m_aGradeItems[ i ] );
	}

	m_oGrid.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oGrid.GetTransform().SetTranslation( 0.0f, AGUI2::GetRootElement()->GetHeight() * 0.25f );
	m_oGrid.ReflowChildren();

	// The backings of the rows are a bit larger than the grid
	for ( TINT i = 0; i < AHighScoreRecord::NUM_SCORES; i++ )
	{
		m_aBackings[ i ].SetDimensions( m_oGrid.GetWidth() * 1.1f, m_oGrid.GetMaxItemDimensions().y * 1.3f );
		m_aBackings[ i ].GetTransform().SetTranslation( m_oGrid.GetWidth() * -0.05f, m_aBackings[ i ].GetHeight() * 0.05f );
	}

	m_iBlinkRank = a_pRecord->iLastRank;
}
