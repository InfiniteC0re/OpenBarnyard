#include "pch.h"
#include "SaveLoad.h"
#include "Locale/ALocaleManager.h"
#include "Helpers/AHighScoreManager.h"
#include "AGameStateController.h"
#include "AFrontEndMainMenuState2.h"
#include "AHackedQuickSave.h"
#include "GUI/AGUI2.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHintManager.h"
#include "Input/AInputMapManager.h"
#include "Tasks/ARootTask.h"
#include "AOptions.h"

#include <GUI/T2GUIFontManager.h>
#include <Toshi/TDateTime.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( DialogInputMap );

TDEFINE_CLASS_NORUNTIME( SaveLoad );

// $Barnyard: FUNCTION 00652270
SaveLoad::SaveLoad( APlayerProgress* a_pProgress, TBOOL a_bBackground )
{
	m_bBackground       = a_bBackground;
	m_pTextBox          = TNULL;
	m_pFont             = TNULL;
	m_fTextWidth        = 0.0f;
	m_fMessageTime      = 0.0f;
	m_pBackground       = TNULL;
	m_pDialog           = TNULL;
	m_pProgress         = a_pProgress;
	m_bUseSavedLanguage = TFALSE;

	T2GUIElement* pRoot = AGUI2::GetRootElement();

	TFLOAT fWidth, fHeight;
	pRoot->GetDimensions( fWidth, fHeight );

	if ( a_bBackground )
	{
		// Black background over the whole screen
		m_pBackground = new T2GUIRectangle();
		m_pBackground->SetDimensions( fWidth, fHeight );
		m_pBackground->SetColour( 0xff000000 );
		m_pBackground->Hide();
		pRoot->AddChildTail( *m_pBackground );
	}

	T2GUIElement* pRoot2 = AGUI2::GetRootElement();
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	m_pDialog = new AGUI2Dialog();
	m_pDialog->Create();
	m_pDialog->SetDialogSize( fWidth, fHeight );
	m_pDialog->Hide();
	pRoot2->AddChildTail( *m_pDialog );

	m_pDialog->GetDimensions( fWidth, fHeight );
	m_fTextWidth = fWidth * 0.8f;
	m_pFont      = T2GUIFontManager::FindFont( "Rekord18" );

	m_pTextBox = new AGUI2TextBox();
	m_pTextBox->Create( m_pFont, m_fTextWidth );
	m_pTextBox->SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_pTextBox->SetShadow( TTRUE, 0xa0000000 );
	m_pTextBox->SetShadowAlpha( 0.6f );

	TFLOAT fScale = m_pTextBox->GetScale();
	m_pTextBox->SetShadowOffset( fScale * 5.0f, fScale * 5.0f );

	if ( m_pDialog )
	{
		m_pDialog->AddChildTail( *m_pTextBox );
	}
	else
	{
		pRoot2->AddChildTail( *m_pTextBox );
		m_pTextBox->Hide();
	}

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( DialogInputMap ) );
}

// $Barnyard: FUNCTION 00651f10
// $Barnyard: FUNCTION 00651fd0
SaveLoad::~SaveLoad()
{
	if ( m_pTextBox )
	{
		m_pTextBox->RemoveSelf();
		delete m_pTextBox;
		m_pTextBox = TNULL;
	}

	if ( m_pBackground )
	{
		m_pBackground->RemoveSelf();
		delete m_pBackground;
	}

	if ( m_pDialog )
	{
		m_pDialog->RemoveSelf();
		delete m_pDialog;
	}
}

// $Barnyard: FUNCTION 00651ff0
void SaveLoad::OnInsertion()
{
	m_bWasPausedByMenu                     = AGameStateController::ms_bPausedByMenu;
	AGameStateController::ms_bPausedByMenu = TTRUE;
	m_bWasPaused                           = TTRUE;

	if ( !AFrontEndMainMenuState2::ms_bIsInserted && !ARootTask::GetSingleton()->IsPaused() )
	{
		m_bWasPaused = TFALSE;
		ARootTask::GetSingleton()->SetPaused( TTRUE );
	}

	AGameState::OnInsertion();

	if ( m_pBackground )
		m_pBackground->Show();
}

// $Barnyard: FUNCTION 00652050
void SaveLoad::OnRemoval()
{
	if ( !m_bWasPaused )
		ARootTask::GetSingleton()->SetPaused( TFALSE );

	if ( m_pBackground )
		m_pBackground->Hide();

	AGameStateController::ms_bPausedByMenu = m_bWasPausedByMenu;
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00652090
TBOOL SaveLoad::SaveToSlot( TBOOL a_bDontSave, TBOOL a_bTakeSnapshot )
{
	APlayerProgress::Header& rHeader = m_pProgress->GetSaveData()->oHeader;
	rHeader.uiUnk10                  = m_pProgress->GetUnk24();
	rHeader.uiUnlockedFlags          = m_pProgress->GetUnlockedFlags();
	AOptions::GetSingleton()->SaveSettings( rHeader.aOptions, AOptions::SAVED_SETTINGS_SIZE );

	if ( a_bDontSave )
		return a_bDontSave;

	AHackedQuickSave* pQuickSave = AHackedQuickSave::GetSingletonSafe();

	if ( !pQuickSave )
		return TFALSE;

	if ( a_bTakeSnapshot )
	{
		if ( AGameLoader::IsMainGameLoaded() )
		{
			pQuickSave->ResetBuffers( TFALSE );
			pQuickSave->SaveGame();
		}
		else if ( AGameLoader::IsGameStateLibraryLoaded() )
		{
			pQuickSave->GetBuffer( AHackedQuickSave::BUFFER_TIME )->Reset();
			pQuickSave->GetBuffer( AHackedQuickSave::BUFFER_PROFILE )->Reset();
			pQuickSave->SaveTime();
			pQuickSave->SaveProfile();
		}
	}

	APlayerProgress::SaveSlot* pSlot = m_pProgress->GetCurrentSlot();

	if ( !pSlot )
		return TFALSE;

	pQuickSave->SaveToMemory( pSlot->aQuickSave, sizeof( pSlot->aQuickSave ) );
	AHintManager::SaveState( pSlot->aHints, TREINTERPRETCAST( void*, sizeof( pSlot->aHints ) ) );

	AHighScoreManager::Save( pSlot->aHighScores, sizeof( pSlot->aHighScores ) );

	AGameLoader::SaveProgressInfo( &pSlot->iGender );

	// Date and time of the save
	TDateTime oDateTime;
	TDateTime::GetLocalTime( oDateTime );
	oDateTime.Pack( pSlot->uiDate, pSlot->uiTime );

	pSlot->bOccupied = 1;
	return TTRUE;
}

// $Barnyard: FUNCTION 006521c0
void SaveLoad::ApplyLoadedSave()
{
	AOptions*             pOptions  = AOptions::GetSingleton();
	Toshi::T2Locale::Lang eLanguage = ALocaleManager::Lang_UNKNOWN;

	if ( !m_bUseSavedLanguage && pOptions->GetLanguage() != ALocaleManager::Lang_UNKNOWN )
		eLanguage = pOptions->GetLanguage();

	pOptions->LoadSavedSettings( m_pProgress->GetSaveData()->oHeader.aOptions, AOptions::SAVED_SETTINGS_SIZE );

	if ( m_bUseSavedLanguage && !ALocaleManager::IsLanguageSupported( pOptions->GetLanguage() ) )
	{
		Toshi::T2Locale::Lang eDefaultLanguage = ALocaleManager::GetDefaultLanguage();

		if ( eDefaultLanguage == ALocaleManager::Lang_UNKNOWN || !ALocaleManager::IsLanguageSupported( eDefaultLanguage ) )
			eDefaultLanguage = ALocaleManager::GetDefaultLanguage();

		if ( eDefaultLanguage != ALocaleManager::Lang_UNKNOWN )
			eLanguage = eDefaultLanguage;
	}

	if ( eLanguage != ALocaleManager::Lang_UNKNOWN )
		pOptions->SetLanguage( eLanguage );

	pOptions->ApplySettings();

	m_pProgress->SetUnk24( m_pProgress->GetUnk24() | m_pProgress->GetSaveData()->oHeader.uiUnk10 );
	m_pProgress->UnlockFlags( m_pProgress->GetSaveData()->oHeader.uiUnlockedFlags );
}
