#include "pch.h"
#include "SaveLoadSKU.h"
#include "AGameStateController.h"
#include "Helpers/APlayerProgress.h"
#include "Locale/ALocaleManager.h"

#include <Toshi/TStringManager.h>

#include <windows.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: the original can create it at runtime (00652590) but it needs constructor arguments
// NOTE: GetClass 00652580, CreateObject 00652590, CreateObjectInPlace 00652610
TDEFINE_CLASS_NORUNTIME( SaveLoadSKU );

static constexpr const TCHAR* s_szSaveFileName = "savedata.by"; // 0x00779940

// Text ids of the messages
static constexpr TINT TEXT_LOADING     = 1463;
static constexpr TINT TEXT_LOADED      = 1464;
static constexpr TINT TEXT_LOAD_FAILED = 1465;
static constexpr TINT TEXT_SAVING      = 1466;
static constexpr TINT TEXT_SAVED       = 1467;
static constexpr TINT TEXT_SAVE_FAILED = 1468;

// Is a_szPath an existing directory (NOTE: _findfirst/_findclose in the original)
static TBOOL IsDirectory( const TCHAR* a_szPath, TBOOL& a_rExists )
{
	_finddata_t oData;
	intptr_t    hFind = _findfirst( a_szPath, &oData );
	_findclose( hFind );

	a_rExists = hFind >= 0;
	return a_rExists && ( oData.attrib & _A_SUBDIR );
}

// $Barnyard: FUNCTION 00652a90
SaveLoadSKU::Step* SaveLoadSKU::CheckFileStep::Update( TFLOAT a_fDeltaTime )
{
	m_pOwner->m_eResult = RESULT_SUCCESS;

	TBOOL bExists;

	if ( !IsDirectory( m_pOwner->m_szSaveDir, bExists ) )
	{
		// Nothing has been saved yet
		delete this;
		return TNULL;
	}

	_finddata_t oData;
	intptr_t    hFind = _findfirst( m_pOwner->m_szSavePath, &oData );
	_findclose( hFind );

	if ( hFind < 0 )
	{
		delete this;
		return new MessageStep( TEXT_LOAD_FAILED );
	}

	delete this;
	return new LoadStep();
}

// $Barnyard: FUNCTION 00652be0
SaveLoadSKU::MessageStep::MessageStep( TINT a_iTextId )
{
	m_pOwner->SetMessage( ALocaleManager::Interface()->GetString( a_iTextId ) );
	m_fTime = 0.0f;
}

// $Barnyard: FUNCTION 00652c50
SaveLoadSKU::Step* SaveLoadSKU::MessageStep::Update( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;

	if ( m_fTime < m_pOwner->m_fMessageTime )
		return this;

	delete this;
	return TNULL;
}

// $Barnyard: FUNCTION 00652ca0
SaveLoadSKU::Step* SaveLoadSKU::LoadStep::Update( TFLOAT a_fDeltaTime )
{
	APlayerProgress* pProgress = m_pOwner->m_pProgress;
	FILE*            pFile     = fopen( m_pOwner->m_szSavePath, "rb" );
	TBOOL            bLoaded   = TFALSE;

	if ( pFile )
	{
		TUINT uiSize = pProgress->GetSaveSize();

		if ( uiSize != fread( pProgress->GetSaveBuffer(), 1, uiSize, pFile ) )
		{
			fclose( pFile );
			pProgress->ClearAllSlots();
		}
		else
		{
			fclose( pFile );
			bLoaded = pProgress->Validate();
		}
	}

	if ( !bLoaded )
	{
		m_pOwner->m_eResult = RESULT_FAILED;
		delete this;
		return new MessageStep( TEXT_LOAD_FAILED );
	}

	m_pOwner->m_eResult = RESULT_SUCCESS;
	m_pOwner->OnLoaded();
	m_pOwner->m_eResult = RESULT_SUCCESS;

	delete this;
	return new MessageStep( TEXT_LOADED );
}

// $Barnyard: FUNCTION 00652e20
SaveLoadSKU::Step* SaveLoadSKU::SaveStep::Update( TFLOAT a_fDeltaTime )
{
	APlayerProgress* pProgress = m_pOwner->m_pProgress;

	TBOOL bExists;
	TBOOL bSaved = TFALSE;

	if ( IsDirectory( m_pOwner->m_szSaveDir, bExists ) || _mkdir( m_pOwner->m_szSaveDir ) == 0 )
	{
		FILE* pFile = fopen( m_pOwner->m_szSavePath, "wb" );

		if ( pFile )
		{
			TUINT uiSize = pProgress->GetSaveSize();
			bSaved       = uiSize == fwrite( pProgress->GetSaveBuffer(), 1, uiSize, pFile );
			fclose( pFile );
		}
	}

	if ( !bSaved )
	{
		delete this;
		return new MessageStep( TEXT_SAVE_FAILED );
	}

	m_pOwner->m_eResult = RESULT_SUCCESS;
	delete this;
	return new MessageStep( TEXT_SAVED );
}

// $Barnyard: FUNCTION 00652680
SaveLoadSKU::SaveLoadSKU( APlayerProgress* a_pProgress, TBOOL a_bBackground, AGameState* a_pNextState )
    : SaveLoad( a_pProgress, a_bBackground )
{
	m_eMode      = MODE_NONE;
	m_pStep      = TNULL;
	m_pNextState = a_pNextState;
	m_eResult    = RESULT_NONE;
	ms_pInstance = this;

	// <exe directory>Save
	GetModuleFileNameA( NULL, m_szSaveDir, 0x103 );
	TCHAR* pSlash = strrchr( m_szSaveDir, '\\' );

	if ( pSlash )
		pSlash[ 1 ] = '\0';

	strcat( m_szSaveDir, "Save" );

	// <exe directory>Save\savedata.by
	strcpy( m_szSavePath, m_szSaveDir );
	strcat( m_szSavePath, "\\" );
	strcat( m_szSavePath, s_szSaveFileName );

#ifdef BARNYARD_COMMUNITY_PATCH
	if ( ms_szDevSavePath[ 0 ] != '\0' )
	{
		// Development: save somewhere else
		strcpy( m_szSavePath, ms_szDevSavePath );
		strcpy( m_szSaveDir, ms_szDevSavePath );

		// Accept both separators (a "-devsavefile C:/x/y.by" path would otherwise become the directory)
		pSlash               = strrchr( m_szSaveDir, '\\' );
		TCHAR* pForwardSlash = strrchr( m_szSaveDir, '/' );

		if ( !pSlash || ( pForwardSlash && pForwardSlash > pSlash ) )
			pSlash = pForwardSlash;

		if ( pSlash )
			pSlash[ 0 ] = '\0';
	}
#endif
}

// $Barnyard: FUNCTION 00652780
// $Barnyard: FUNCTION 006528d0
SaveLoadSKU::~SaveLoadSKU()
{
	ms_pInstance = TNULL;
}

// $Barnyard: FUNCTION 006529c0
TBOOL SaveLoadSKU::OnUpdate( TFLOAT a_fDeltaTime )
{
	AGameState::OnUpdate( a_fDeltaTime );

	if ( m_pStep )
	{
		m_pStep = m_pStep->Update( a_fDeltaTime );
		return TTRUE;
	}

	if ( m_pNextState )
	{
		if ( m_eMode != MODE_2 || m_eResult == RESULT_SUCCESS )
		{
			AGameStateController::GetSingleton()->ReplaceState( m_pNextState );
			return TTRUE;
		}

		delete m_pNextState;
		m_pNextState = TNULL;
		m_pProgress->ClearAllSlots();
	}

	Remove();
	return TTRUE;
}

// $Barnyard: FUNCTION 006528f0
void SaveLoadSKU::OnInsertion()
{
	SaveLoad::OnInsertion();

	TINT iTextId;

	if ( m_eMode > 0 && m_eMode <= MODE_3 )
	{
		m_pStep = new CheckFileStep();
		iTextId = TEXT_LOADING;
	}
	else if ( m_eMode == MODE_SAVE )
	{
		m_pStep = new SaveStep();
		iTextId = TEXT_SAVING;
	}
	else
	{
		return;
	}

	SetMessage( ALocaleManager::Interface()->GetString( iTextId ) );
}

// $Barnyard: FUNCTION 006527a0
void SaveLoadSKU::OnRemoval()
{
	SaveLoad::OnRemoval();
}

// $Barnyard: FUNCTION 00652570
void SaveLoadSKU::Cancel()
{
	m_eResult = RESULT_FAILED;
}

// $Barnyard: FUNCTION 006527b0
void SaveLoadSKU::StartLoad()
{
	m_eMode = MODE_LOAD;
}

// $Barnyard: FUNCTION 006527c0
void SaveLoadSKU::StartSave( TBOOL a_bUnused, TBOOL a_bDontSave, TBOOL a_bTakeSnapshot )
{
	SaveToSlot( a_bDontSave, a_bTakeSnapshot );
	m_pProgress->SetupHeader();
	m_pProgress->UpdateChecksum();
	m_bDontSave = a_bDontSave;
	m_eMode     = MODE_SAVE;
}

// $Barnyard: FUNCTION 00652800
void SaveLoadSKU::StartMode2()
{
	m_eMode = MODE_2;
}

// $Barnyard: FUNCTION 00652810
void SaveLoadSKU::StartMode3()
{
	m_eMode = MODE_3;
}

// $Barnyard: FUNCTION 00652820
TINT SaveLoadSKU::GetResult()
{
	return m_eResult;
}

// $Barnyard: FUNCTION 00652830
void SaveLoadSKU::SetMessage( const TWCHAR* a_wszText )
{
	if ( m_eMode < MODE_2 || m_eMode == MODE_3 || m_eMode == MODE_2 || !a_wszText )
	{
		// Nothing is shown while loading
		T2GUIElement* pElement = m_pDialog ? TSTATICCAST( T2GUIElement, m_pDialog ) : TSTATICCAST( T2GUIElement, m_pTextBox );
		pElement->Hide();
		m_fMessageTime = 0.0f;
	}
	else
	{
		m_pTextBox->SetWidth( m_fTextWidth );
		m_pTextBox->SetText( a_wszText );

		T2GUIElement* pElement = m_pDialog ? TSTATICCAST( T2GUIElement, m_pDialog ) : TSTATICCAST( T2GUIElement, m_pTextBox );
		pElement->Show();
		m_fMessageTime = 1.25f;
	}
}

#ifdef BARNYARD_COMMUNITY_PATCH

void SaveLoadSKU::Dev_SetSavePath( const TCHAR* a_szPath )
{
	if ( a_szPath )
		TStringManager::String8Copy( ms_szDevSavePath, a_szPath, sizeof( ms_szDevSavePath ) );
	else
		ms_szDevSavePath[ 0 ] = '\0';
}

#endif
