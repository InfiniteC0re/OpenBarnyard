#include "pch.h"
#include <Render/TCameraObject.h>
#include "AEndGameState.h"
#include "AGameStateController.h"
#include "ACreditsState.h"
#include "ASSeqState.h"
#include "SaveLoadSKU.h"
#include "FX/AGameTimeFXManager.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2MessageBoxState.h"
#include "FSM/AMiscFunctions.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/AWeatherManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AVOStream.h"
#include "SoundBank/ui.h"
#include "Terrain/ATerrainInterface.h"
#include "Animation/AAnimatableObject.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ASSeqController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass, the create and the create-in-place functions of the TClass
// $Barnyard: FUNCTION 00539360
// $Barnyard: FUNCTION 00539370
// $Barnyard: FUNCTION 005393e0
TDEFINE_CLASS( AEndGameState );

// NOTE: inlined into the create functions of the TClass (00539370, 005393e0)
AEndGameState::AEndGameState()
{
}

// $Barnyard: FUNCTION 00539430
// $Barnyard: FUNCTION 00539450
AEndGameState::~AEndGameState()
{
}

// $Barnyard: FUNCTION 00539460
void AEndGameState::OnInsertion()
{
	BaseClass::OnInsertion();
	m_iStep = STEP_CREDITS;

	AGameLoader::Terrain_Set( AGameLoader::Terrain_ss10, TFALSE, TTRUE, 0, 0, 0, 0 );
	AGameLoader::SpawnSimPlayer();
	AGameTimeFXManager::GetSingleton()->SetDayPhase( AGameTimeManager::DAYPHASE_NIGHT, TFALSE, 0.0f );
	AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );

	ASSeqController::GetSingleton()->LoadScript( "SS10_end.tso", TVector4::VEC_ZERO );
	ASSeqController::GetSingleton()->LoadScript( "SS11_prolog.tso", TVector4::VEC_ZERO );

	// The scripted sequences play the player, hide and freeze the real one
	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
	pPlayer->SetVisible( TFALSE );
	pPlayer->SetMovement( TFALSE );
	pPlayer->SetAIEnabled( TFALSE );
	pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

	AGameLoader::Terrain_Set( AGameLoader::Terrain_ss11, TFALSE, TTRUE, 0, 0, 0, 0 );
	ATerrainInterface::GetSingleton()->WaitUntilLoaded();
}

// $Barnyard: FUNCTION 00539530
void AEndGameState::OnActivate()
{
	BaseClass::OnActivate();

	AMusicManager* pMusicManager = AMusicManager::GetSingleton();
	pMusicManager->SetForcedSound( -1 );
	pMusicManager->ForceUpdate();

	switch ( m_iStep )
	{
		case STEP_CREDITS:
			AGameStateController::GetSingleton()->PushState( new ACreditsState( TFALSE, TTRUE ) );
			break;
		case STEP_SSEQ:
			AVOStream::GetSingleton()->Close( TTRUE );
			AVOStream::GetSingleton()->Open( "data/sound/FSB/ss.fsb", TFALSE, TFALSE );
			AGameTimeFXManager::GetSingleton()->SetDayPhase( AGameTimeManager::DAYPHASE_AFTERNOON, TFALSE, 0.0f );
			AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );

			AGameStateController::GetSingleton()->PushState(
			    new ASSeqState( "SS11_prolog", "SS11", "SS11_cameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
			);
			break;
		case STEP_SAVE:
		{
			AGameTimeFXManager::GetSingleton()->SetDayPhase( AGameTimeManager::DAYPHASE_AFTERNOON, TFALSE, 0.0f );
			AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );

			APlayerProgress* pProgress = APlayerProgress::GetSingleton();

			if ( !pProgress->GetCurrentSlot() )
			{
				// No save slot, back to the front end
				AGameLoader::ReturnToFrontEnd( TNULL, TTRUE, TTRUE );
				break;
			}

			SaveLoadSKU* pSaveState = new SaveLoadSKU( pProgress, TTRUE, TNULL );
			pSaveState->StartSave( TFALSE, TFALSE, TFALSE );
			AGameStateController::GetSingleton()->PushState( pSaveState );
			break;
		}
		case STEP_MESSAGE:
		{
			APlayerProgress* pProgress = APlayerProgress::GetSingleton();
			TBOOL            bChampion = pProgress->IsChampion();
			TBOOL            bDetail   = pProgress->HasCurrentSlotBarnDetail();

			if ( bChampion )
				AMiscFunctions::ShowMessageBox( 1066, bDetail ? 3400 : 3398 );
			else
				AMiscFunctions::ShowMessageBox( 1066, bDetail ? 3397 : 3399 );

			break;
		}
		case STEP_FRONTEND:
			AGameLoader::ReturnToFrontEnd( TNULL, TTRUE, TTRUE );
			break;
	}

	m_iStep++;
}
