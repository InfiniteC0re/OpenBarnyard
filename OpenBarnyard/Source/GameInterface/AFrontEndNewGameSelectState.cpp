#include "pch.h"
#include "AFrontEndNewGameSelectState.h"
#include "AGameStateController.h"
#include "AHackedQuickSave.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerProfileManager.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/AHintManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Mod/AModEvents.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TDateTime.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0040c140
TDEFINE_CLASS( AFrontEndNewGameSelectState );

void AFrontEnd_ReadSaveSlot( const APlayerProgress::SaveSlot* a_pSlot, TWCHAR* a_wszOutName, TWCHAR* a_wszOutDate, TINT& a_rOutGender, TINT& a_rOutCostume )
{
	TDateTime oDateTime;
	oDateTime.UnPack( a_pSlot->uiDate, a_pSlot->uiTime );

	const TINT iYear   = oDateTime.uiYear;
	const TINT iMonth  = oDateTime.uiMonth;
	const TINT iDay    = oDateTime.uiDay;
	const TINT iHour   = oDateTime.uiHour;
	const TINT iMinute = oDateTime.uiMinute;

	// The profile buffer of the quick save starts with the gender, the costume and the name of the player
	const TUINT8* pProfile = a_pSlot->aQuickSave + AHackedQuickSave::GetBufferDataOffset( AHackedQuickSave::BUFFER_PROFILE );
	a_rOutGender           = AHackedQuickSave::ReadInt( pProfile );
	a_rOutCostume          = AHackedQuickSave::ReadInt( pProfile + 4 );

	a_wszOutName[ 0 ] = L'\0';
	AHackedQuickSave::ReadWString( pProfile + 8, a_wszOutName, 32 );

	const TINT iHour12 = ( iHour == 0 ) ? 12 : ( ( iHour > 12 ) ? iHour - 12 : iHour );

	switch ( ALocaleManager::Interface()->GetLanguage() )
	{
		case ALocaleManager::Lang_English:
			// month/day/year
			TStringManager::String16Format( a_wszOutDate, 32, L"  (%d/%d/%02d %2d:%02d", iMonth + 1, iDay, iYear % 100, iHour12, iMinute );
			break;
		case ALocaleManager::Lang_Japanese:
			// year/month/day
			TStringManager::String16Format( a_wszOutDate, 32, L"  (%02d/%d/%d %2d:%02d", iYear % 100, iMonth + 1, iDay, iHour12, iMinute );
			break;
		case ALocaleManager::Lang_German:
		case ALocaleManager::Lang_Italian:
		case ALocaleManager::Lang_Spanish:
		case ALocaleManager::Lang_French:
			// day/month/year, 24 hours
			TStringManager::String16Format( a_wszOutDate, 32, L"  (%d/%d/%02d %2d:%02d)", iDay, iMonth + 1, iYear % 100, iHour, iMinute );
			return;
		default:
			// day/month/year
			TStringManager::String16Format( a_wszOutDate, 32, L"  (%d/%d/%02d %2d:%02d", iDay, iMonth + 1, iYear % 100, iHour12, iMinute );
			break;
	}

	// AM / PM
	TINT iLength = TStringManager::String16Length( a_wszOutDate );
	TStringManager::String16Format( a_wszOutDate + iLength, 32 - iLength, L"%s)", ALocaleManager::Interface()->GetString( ( iHour > 11 ) ? 1175 : 1176 ) );
}

// $Barnyard: FUNCTION 0040c260
AFrontEndNewGameSelectState::AFrontEndNewGameSelectState()
    : m_iActivatedSlot( -1 )
    , m_bOverwrite( TFALSE )
{
}

// $Barnyard: FUNCTION 0040c150
// $Barnyard: FUNCTION 0040c320
AFrontEndNewGameSelectState::~AFrontEndNewGameSelectState()
{
}

// NOTE: shares 00407aa0 with AOptionsState::ProcessCommand
TBOOL AFrontEndNewGameSelectState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0040c340
void AFrontEndNewGameSelectState::OnInsertion()
{
	m_bFocusedElementBloated = TFALSE;

	T2GUIFont* pFont     = T2GUIFontManager::FindFont( "Rekord26" );
	T2GUIFont* pDateFont = T2GUIFontManager::FindFont( "Rekord18" );
	TFLOAT     fMaxText  = 0.0f;
	TFLOAT     fMaxImage = 0.0f;

	APlayerProgress* pProgress = APlayerProgress::GetSingleton();

	for ( TINT i = 0; i < APlayerProgress::NUM_SAVE_SLOTS; i++ )
	{
		APlayerProgress::SaveSlot* pSlot   = pProgress->GetSlot( i );
		const TWCHAR*              wszText = m_awszSlotNames[ i ];

		if ( pSlot->bOccupied == 1 )
		{
			TINT iGender, iCostume;
			AFrontEnd_ReadSaveSlot( pSlot, m_awszSlotNames[ i ], m_awszSlotDates[ i ], iGender, iCostume );

			// The player character of the slot, left of the button
			const TCHAR* szPicture = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( iCostume, iGender );
			m_aSlotPictures[ i ].SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( szPicture ) );
			m_aSlotPictures[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
			m_aSlotPictures[ i ].SetTransform( 0.0f, pDateFont->GetLineHeight() * -0.5f );
			m_aSlotButtons[ i ].AddChildTail( m_aSlotPictures[ i ] );

			fMaxImage = TMath::Max( fMaxImage, m_aSlotPictures[ i ].GetWidth() );
		}
		else
		{
			// Empty
			wszText                   = ALocaleManager::Interface()->GetString( 1283 );
			m_awszSlotDates[ i ][ 0 ] = L'\0';
		}

		fMaxText = TMath::Max( fMaxText, pFont->GetTextWidth( wszText ) );

		TFLOAT fGap = TMath::Max( ( m_aSlotPictures[ i ].GetHeight() - pFont->GetLineHeight() ) * 0.25f, 0.0f ) + 4.0f;

		AGUI2Button& rButton = m_aSlotButtons[ i ];
		rButton.Create( pFont, pFont->GetTextWidth( wszText ), wszText, fGap );
		rButton.SetFocusedColour( 0xffffffff );
		rButton.SetUnkColour( 0xffffc800 );
		rButton.SetEnabledColour( 0xffb56f22 );
		rButton.SetDisabledColour( 0xff4b4b4b );
		rButton.SetFocusedScale( 1.0f );
		rButton.SetScale( 0.8f );
		rButton.SetId( i );
		rButton.SetScaleOnFocus( TTRUE );
		rButton.GetTextBox().SetTextAlign( T2GUIFont::TextAlign_Left );
		rButton.SetTextShadow( TTRUE, 0xa0000000, 0.6f );
		rButton.SetTextShadowOffset( rButton.GetTextBox().GetScale() * 5.0f );

		// The save date, below the name
		AGUI2TextBox& rDate = m_aSlotDateTexts[ i ];
		rDate.Create( pDateFont, pDateFont->GetTextWidth( m_awszSlotDates[ i ] ) );
		rDate.SetText( m_awszSlotDates[ i ] );
		rDate.SetScale( rButton.GetTextBox().GetScale() );
		rDate.SetShadow( TTRUE, 0xa0000000 );
		rDate.SetShadowAlpha( 0.6f );
		rDate.SetShadowOffset( rButton.GetTextBox().GetScale() * 5.0f, rButton.GetTextBox().GetScale() * 5.0f );
		rDate.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
		rDate.SetTransform( 0.0f, 8.0f - pDateFont->GetLineHeight() * 0.5f );
		rButton.GetTextBox().AddChildTail( rDate );

		rButton.OnFocusLost();
		m_oMenu.AddMenuItem( rButton );
	}

	m_oMenu.SetFocusAt( m_aSlotButtons[ 0 ] );
	m_oMenu.SetFocus( TFALSE );
	m_oDialogTitle.SetText( ALocaleManager::Interface()->GetString( 11 ) );

	// Make the dialog wide enough for the pictures
	AGUI2TextBox& rFirstText = m_aSlotButtons[ 0 ].GetTextBox();
	TFLOAT        fWidth     = rFirstText.GetWidth();
	rFirstText.SetWidth( fMaxImage * 0.5f + fMaxText );

	BaseClass::OnInsertion();

	rFirstText.SetWidth( fWidth );
}

// $Barnyard: FUNCTION 0040c200
void AFrontEndNewGameSelectState::OnRemoval()
{
	for ( TINT i = 0; i < APlayerProgress::NUM_SAVE_SLOTS; i++ )
	{
		m_aSlotPictures[ i ].RemoveSelf();
		m_aSlotButtons[ i ].RemoveSelf();
	}

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 0040c250
void AFrontEndNewGameSelectState::OnActivate()
{
	m_iActivatedSlot = -1;
	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 0040c960
void AFrontEndNewGameSelectState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, a_pMenuItem );
	m_iActivatedSlot     = pButton->GetId();

	AHighScoreManager::ScoreReset( TFALSE );
	AHintManager::ResetState();

	APlayerProgress* pProgress = APlayerProgress::GetSingleton();
	m_bOverwrite               = pProgress->GetSlot( m_iActivatedSlot )->bOccupied == 1;
	pProgress->SetCurrentSlotIndex( m_iActivatedSlot );

	OnButtonActivated( pButton );
}

// $Barnyard: FUNCTION 0040ca40
void AFrontEndNewGameSelectState::OnMenuClose()
{
	if ( m_iActivatedSlot == -1 )
	{
		Remove();
		return;
	}

	if ( m_bOverwrite == TTRUE )
	{
		// The slot is in use, ask before overwriting it
		AMessagePopupState* pPopup = new AMessagePopupState(
		    ALocaleManager::Interface()->GetString( 1064 ),
		    ALocaleManager::Interface()->GetString( 289 ),
		    TPString8(),
		    AInputCommand_No,
		    TFALSE,
		    AInputCommand_Yes,
		    AInputCommand_No,
		    AInputCommand_Empty,
		    AInputCommand_Empty,
		    TFALSE,
		    60.0f,
		    0
		);

		pPopup->SetUnk19b0( TTRUE );
		pPopup->SetUnk19b4( 0.15f );
		pPopup->SetUnk19b8( 0.18f );

		m_oPopupListener.Connect( pPopup->GetResponseEmitter(), this, OnOverwritePopupResponse, 0 );
		AGameStateController::GetSingleton()->PushState( pPopup );
		return;
	}

	StartNewGame();
}

// NOTE: inlined in OnMenuClose and OnOverwritePopupResponse
void AFrontEndNewGameSelectState::StartNewGame()
{
	APlayerProfileManager::GetSingleton()->SetMainGender( APlayerProfile::GENDER_MALE );
	AGameLoader::ResetSimLoadingPictures();
	FIRE_EVENT( AGameLoader::NewGame );

	// Once the screen has faded out, AGameStateController starts the SS01 intro sequence (AGameLoader::StartNewGame)
	AGameStateController* pController = AGameStateController::GetSingleton();
	pController->AddFlags( AGameStateController::FLAGS_START_NEW_GAME );
	pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION | AGameStateController::FLAGS_FADE_SOUND );
}

// $Barnyard: FUNCTION 0040ca20
// $Barnyard: FUNCTION 0040c9c0
TBOOL __stdcall AFrontEndNewGameSelectState::OnOverwritePopupResponse( AFrontEndNewGameSelectState* a_pState, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent )
{
	a_pState->m_oPopupListener.Disconnect();

	if ( a_pEvent->eButton == AInputCommand_Yes )
	{
		APlayerProfileManager::GetSingleton()->SetMainGender( APlayerProfile::GENDER_MALE );
		AGameLoader::ResetSimLoadingPictures();
		FIRE_EVENT( AGameLoader::NewGame );
		APlayerProgress::GetSingleton()->ClearCurrentSlot();

		AGameStateController* pController = AGameStateController::GetSingleton();
		pController->AddFlags( AGameStateController::FLAGS_START_NEW_GAME );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION | AGameStateController::FLAGS_FADE_SOUND );
	}
	else
	{
		a_pPopup->Remove();
	}

	return TFALSE;
}
