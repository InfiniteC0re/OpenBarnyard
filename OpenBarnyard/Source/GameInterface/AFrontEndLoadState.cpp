#include "pch.h"
#include "AFrontEndLoadState.h"
#include "AFrontEndNewGameSelectState.h"
#include "AGameStateController.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerProfileManager.h"
#include "Helpers/AGameLoader.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00408420
TDEFINE_CLASS( AFrontEndLoadState );

// $Barnyard: FUNCTION 00408430
AFrontEndLoadState::AFrontEndLoadState()
    : m_iActivatedSlot( -1 )
    , m_iNumSlots( 0 )
{
}

// $Barnyard: FUNCTION 00408500
// $Barnyard: FUNCTION 00408680
AFrontEndLoadState::~AFrontEndLoadState()
{
}

// NOTE: shares 00407aa0 with AOptionsState::ProcessCommand
TBOOL AFrontEndLoadState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 004086a0
void AFrontEndLoadState::OnInsertion()
{
	m_bFocusedElementBloated = TFALSE;

	T2GUIFont* pFont     = T2GUIFontManager::FindFont( "Rekord26" );
	T2GUIFont* pDateFont = T2GUIFontManager::FindFont( "Rekord18" );
	TINT       iLastSlot = -1;
	TFLOAT     fMaxImage = 0.0f;
	TFLOAT     fMaxText  = 0.0f;

#ifdef BARNYARD_COMMUNITY_PATCH
	TINT iFirstSlot = -1;
#endif

	APlayerProgress* pProgress = APlayerProgress::GetSingleton();

	for ( TINT i = 0; i < APlayerProgress::NUM_SAVE_SLOTS; i++ )
	{
		APlayerProgress::SaveSlot* pSlot = pProgress->GetSlot( i );

		// Only the used slots can be loaded
		if ( pSlot->bOccupied != 1 )
			continue;

		TINT iGender, iCostume;
		AFrontEnd_ReadSaveSlot( pSlot, m_awszSlotNames[ i ], m_awszSlotDates[ i ], iGender, iCostume );

		// The player character of the slot, left of the button
		const TCHAR* szPicture = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( iCostume, iGender );
		m_aSlotPictures[ i ].SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( szPicture ) );
		m_aSlotPictures[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
		m_aSlotPictures[ i ].SetTransform( 0.0f, pDateFont->GetLineHeight() * -0.5f );
		m_aSlotButtons[ i ].AddChildTail( m_aSlotPictures[ i ] );

		fMaxImage = TMath::Max( fMaxImage, m_aSlotPictures[ i ].GetWidth() );
		fMaxText  = TMath::Max( fMaxText, pFont->GetTextWidth( m_awszSlotNames[ i ] ) );

		TFLOAT fGap = TMath::Max( ( m_aSlotPictures[ i ].GetHeight() - pFont->GetLineHeight() ) * 0.25f + 4.0f, 0.0f );

		AGUI2Button& rButton = m_aSlotButtons[ i ];
		rButton.Create( pFont, pFont->GetTextWidth( m_awszSlotNames[ i ] ), m_awszSlotNames[ i ], fGap );
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
		m_iNumSlots++;
		iLastSlot = i;

#ifdef BARNYARD_COMMUNITY_PATCH
		if ( iFirstSlot == -1 )
			iFirstSlot = i;
#endif
	}

	m_oDialogTitle.SetText( ALocaleManager::Interface()->GetString( 293 ) );

	if ( m_iNumSlots != 0 )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: the original focuses the button of the first slot even if it's not used
		m_oMenu.SetFocusAt( m_aSlotButtons[ iFirstSlot ] );
#else
		m_oMenu.SetFocusAt( m_aSlotButtons[ 0 ] );
#endif
		m_oMenu.SetFocus( TFALSE );

		// Make the dialog wide enough for the pictures
		AGUI2TextBox& rLastText = m_aSlotButtons[ iLastSlot ].GetTextBox();
		TFLOAT        fWidth    = rLastText.GetWidth();
		rLastText.SetWidth( fMaxImage * 0.5f + fMaxText );

		BaseClass::OnInsertion();

		rLastText.SetWidth( fWidth );
	}
	else
	{
		// No saved game
		TFLOAT fRootWidth, fRootHeight;
		AGUI2::GetRootElement()->GetDimensions( fRootWidth, fRootHeight );

		ALocale::FormatInputTags( m_wszNoSaves, TARRAYSIZE( m_wszNoSaves ), 243 );

		m_oNoSavesButton.Create( pFont, fRootWidth * 0.5f, m_wszNoSaves, 0.0f );
		m_oNoSavesButton.SetFocusedColour( 0xffffffff );
		m_oNoSavesButton.SetEnabledColour( 0xffb56f22 );
		m_oNoSavesButton.SetDisabledColour( 0xff4b4b4b );
		m_oNoSavesButton.SetFocusedScale( 1.0f );
		m_oNoSavesButton.OnFocusLost();
		m_oNoSavesButton.SetTextShadow( TTRUE, 0xa0000000, 0.6f );
		m_oNoSavesButton.SetTextShadowOffset( m_oNoSavesButton.GetTextBox().GetScale() * 5.0f );
		m_oMenu.AddMenuItem( m_oNoSavesButton );

		BaseClass::OnInsertion();
	}
}

// $Barnyard: FUNCTION 004085e0
void AFrontEndLoadState::OnRemoval()
{
	m_oNoSavesButton.RemoveSelf();

	for ( TINT i = 0; i < APlayerProgress::NUM_SAVE_SLOTS; i++ )
	{
		m_aSlotPictures[ i ].RemoveSelf();
		m_aSlotButtons[ i ].RemoveSelf();
	}

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00408650
void AFrontEndLoadState::OnActivate()
{
	m_iActivatedSlot = -1;
	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 004085b0
void AFrontEndLoadState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	// The "no saved game" button does nothing
	if ( a_pMenuItem == &m_oNoSavesButton )
		return;

	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, a_pMenuItem );
	m_iActivatedSlot     = pButton->GetId();

	OnButtonActivated( pButton );
}

// $Barnyard: FUNCTION 00408db0
void AFrontEndLoadState::OnMenuClose()
{
	if ( m_iActivatedSlot == -1 )
	{
		Remove();
		return;
	}

	APlayerProgress::GetSingleton()->SetCurrentSlotIndex( m_iActivatedSlot );
	AGameStateController::GetSingleton()->PopState( this );

	// Starts the main game from the slot once the screen has faded out
	AGameLoader::LoadSavedGame( TFALSE );
}
