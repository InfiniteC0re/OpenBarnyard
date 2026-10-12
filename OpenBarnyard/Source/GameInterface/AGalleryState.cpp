#include "pch.h"
#include "AGalleryState.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUISystem.h"
#include "GUI/T2GUIFontManager.h"
#include "HUD/AHudManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Render/TRenderInterface.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0040cfc0, CreateObject 0040d0e0, CreateObjectInPlace 0040d140 (the constructor is inlined in both)
TDEFINE_CLASS( AGalleryState );

TPSTRING8_DECLARE( DialogInputMap );

static const TCHAR* s_aCowsPictures[]     = { "bkg_Gallery_Cows_01", "bkg_Gallery_Cows_02", "bkg_Gallery_Cows_03", "bkg_Gallery_Cows_04", "bkg_Gallery_Cows_05", "bkg_Gallery_Cows_06" };
static const TCHAR* s_aHorsesPictures[]   = { "bkg_Gallery_Horses_01", "bkg_Gallery_Horses_02", "bkg_Gallery_Horses_03", "bkg_Gallery_Horses_04", "bkg_Gallery_Horses_05", "bkg_Gallery_Horses_06" };
static const TCHAR* s_aPigsPictures[]     = { "bkg_Gallery_Pigs_01", "bkg_Gallery_Pigs_02", "bkg_Gallery_Pigs_03", "bkg_Gallery_Pigs_04", "bkg_Gallery_Pigs_05", "bkg_Gallery_Pigs_06" };
static const TCHAR* s_aBarnyardPictures[] = { "bkg_Gallery_Barnyard_01", "bkg_Gallery_Barnyard_02", "bkg_Gallery_Barnyard_03", "bkg_Gallery_Barnyard_04", "bkg_Gallery_Barnyard_05", "bkg_Gallery_Barnyard_06", "bkg_Gallery_Barnyard_07" };
static const TCHAR* s_aStoryPictures[]    = { "bkg_Gallery_Story_01", "bkg_Gallery_Story_02", "bkg_Gallery_Story_03", "bkg_Gallery_Story_04", "bkg_Gallery_Story_05", "bkg_Gallery_Story_06", "bkg_Gallery_Story_07", "bkg_Gallery_Story_08", "bkg_Gallery_Story_09", "bkg_Gallery_Story_10", "bkg_Gallery_Story_11", "bkg_Gallery_Story_12" };
static const TCHAR* s_aMinigamePictures[] = { "bkg_Gallery_Minigame_01", "bkg_Gallery_Minigame_02", "bkg_Gallery_Minigame_03", "bkg_Gallery_Minigame_04", "bkg_Gallery_Minigame_05", "bkg_Gallery_Minigame_06" };
static const TCHAR* s_aOriginalPictures[] = { "bkg_Gallery_Original_01", "bkg_Gallery_Original_02", "bkg_Gallery_Original_03", "bkg_Gallery_Original_04", "bkg_Gallery_Original_05", "bkg_Gallery_Original_06", "bkg_Gallery_Original_07", "bkg_Gallery_Original_08", "bkg_Gallery_Original_09" };
static const TCHAR* s_aWorldPictures[]    = { "bkg_Gallery_World_01", "bkg_Gallery_World_02", "bkg_Gallery_World_03", "bkg_Gallery_World_04", "bkg_Gallery_World_05", "bkg_Gallery_World_06", "bkg_Gallery_World_07" };
static const TCHAR* s_aClothesPictures[]  = { "bkg_Gallery_Clothes_01", "bkg_Gallery_Clothes_02", "bkg_Gallery_Clothes_03", "bkg_Gallery_Clothes_04", "bkg_Gallery_Clothes_05", "bkg_Gallery_Clothes_06" };

const TCHAR* const* const AGalleryState::ms_aPictures[ NUM_CATEGORIES ] = {
	s_aCowsPictures,
	s_aHorsesPictures,
	s_aPigsPictures,
	s_aBarnyardPictures,
	s_aStoryPictures,
	s_aMinigamePictures,
	s_aOriginalPictures,
	s_aWorldPictures,
	s_aClothesPictures,
};

// $Barnyard: FUNCTION 0040cfd0
AGalleryState::AGalleryState( TINT a_iCategory )
{
	m_iCategory    = a_iCategory;
	m_iPicture     = 0;
	m_wszPage[ 0 ] = L'\0';
}

// $Barnyard: FUNCTION 0040d060
// $Barnyard: FUNCTION 0040d180
AGalleryState::~AGalleryState()
{
}

// $Barnyard: FUNCTION 0040d5b0
TBOOL AGalleryState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	switch ( a_eInputCommand )
	{
		case AInputCommand_OK:
		case AInputCommand_Cancel:
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
			Remove();
			return TTRUE;
		case AInputCommand_Left:
			if ( --m_iPicture < 0 )
				m_iPicture = ms_aNumPictures[ m_iCategory ] - 1;

			ShowPicture( m_iCategory, m_iPicture );
			return TTRUE;
		case AInputCommand_Right:
			if ( ++m_iPicture >= ms_aNumPictures[ m_iCategory ] )
				m_iPicture = 0;

			ShowPicture( m_iCategory, m_iPicture );
			return TTRUE;
		default:
			return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}
}

// $Barnyard: FUNCTION 0040d390
void AGalleryState::OnInsertion()
{
	SetInputMap( TPS8( DialogInputMap ) );
	ShowPicture( m_iCategory, m_iPicture );

	TFLOAT fScreenWidth, fScreenHeight;
	AGUI2::GetRootElement()->GetDimensions( fScreenWidth, fScreenHeight );

	T2GUIFont*    pFont    = T2GUIFontManager::FindFont( "Rekord26" );
	const TWCHAR* wszTitle = g_pLocaleManager->GetString( ms_aCategoryNames[ m_iCategory ] );

	m_oTitle.Create( pFont, pFont->GetTextWidth( wszTitle ) );
	m_oTitle.SetText( wszTitle );
	m_oTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitle.SetTransform( 0.0f, pFont->GetLineHeight() );

	m_oPage.Create( pFont, fScreenWidth * 0.15f );
	m_oPage.SetScale( 0.75f );
	m_oPage.SetText( m_wszPage );
	m_oPage.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oPage.SetTransform( 0.0f, pFont->GetLineHeight() );

	// The arrows on both sides of the page number
	T2GUITextureSection* pTex = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	m_aArrows[ ARROW_LEFT ].SetDimensions( pTex->GetWidth() * 0.6f, pTex->GetHeight() * 0.6f );
	m_aArrows[ ARROW_LEFT ].SetTextureSection( pTex );
	m_aArrows[ ARROW_LEFT ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_aArrows[ ARROW_LEFT ].SetTransform( 0.0f, -5.0f );

	pTex = T2GUITextureSectionManager::GetTextureSection( "left_arrow2" );
	m_aArrows[ ARROW_RIGHT ].SetDimensions( pTex->GetWidth() * 0.6f, pTex->GetHeight() * 0.6f );
	m_aArrows[ ARROW_RIGHT ].SetTextureSection( pTex );
	m_aArrows[ ARROW_RIGHT ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aArrows[ ARROW_RIGHT ].SetTransform( 0.0f, -5.0f );

	m_oPage.AddChildTail( m_aArrows[ ARROW_LEFT ] );
	m_oPage.AddChildTail( m_aArrows[ ARROW_RIGHT ] );
	AGUI2::GetRootElement()->AddChildTail( m_oPage );

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0040d210
void AGalleryState::OnRemoval()
{
	m_aArrows[ ARROW_LEFT ].RemoveSelf();
	m_aArrows[ ARROW_RIGHT ].RemoveSelf();
	m_oPage.RemoveSelf();
	m_oTitle.RemoveSelf();

	AGUISystem::GetSingleton()->SetPicture( TPString8() );
	AGUISystem::GetSingleton()->ResetPicture();

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0040d1a0
void AGalleryState::OnActivate()
{
	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetTexts( -1, 1048, -1 );
	m_HUDParams.SetFlags( AHudElement::MASK_MENU_NAV );

	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 0040d1e0
void AGalleryState::OnDeactivate()
{
	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetDefaultTexts();

	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 0040d2d0
void AGalleryState::ShowPicture( TINT a_iCategory, TINT a_iPicture )
{
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	AGUISystem::GetSingleton()->SetPicture( TPString8( ms_aPictures[ a_iCategory ][ a_iPicture ] ) );
	TStringManager::String16Format( m_wszPage, TARRAYSIZE( m_wszPage ), L"%d/%d", a_iPicture + 1, ms_aNumPictures[ a_iCategory ] );
}
