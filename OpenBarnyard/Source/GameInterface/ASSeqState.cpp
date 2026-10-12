#include "pch.h"
#include "ASSeqState.h"
#include "FX/AGameTimeFXManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "AGameStateController.h"
#include "ALoadScreen.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Animation/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ASSeqCameraHelper.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGateManager.h"
#include "HUD/AHudManager.h"
#include "MiniGame/AMiniGameManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/ASSeq.h"
#include "Units/ASSeqActorData.h"
#include "Units/ASSeqController.h"
#include "Units/AUnitActor.h"
#include "Units/AUnitFactory.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "Units/AUnitType.h"
#include "Quests/AQuestManager.h"
#include "Player/APlayerProfileManager.h"
#include "Tasks/ARootTask.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundManager.h"
#include "Units/AAnimatedUnit.h"

#include <Input/TInputDeviceController.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceMouse.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00440e10
TDEFINE_CLASS_NORUNTIME( ASSeqState );

// $Barnyard: FUNCTION 00441320
ASSeqState::ASSeqState( const TCHAR* a_szScriptFile, const TCHAR* a_szName, const TCHAR* a_szCamera, const TVector4& a_rcPosition, TINT a_iUnk, TBOOL a_bRemoveWhenOver, TBOOL a_bDestroyUnits, TBOOL a_bRestartMusic, TBOOL a_bLoadNextState, TBOOL a_bRestoreGameTimeFX, TBOOL a_bShowChapterTitle, TBOOL a_bFade, TBOOL a_bStartImmediately )
    : m_strScriptFile( a_szScriptFile )
    , m_strName( a_szName )
    , m_strCamera( a_szCamera )
    , m_bUnk1( TFALSE )
    , m_pPreviousCameraHelper( TNULL )
    , m_vecPosition( a_rcPosition )
    , m_fTime( 0.0f )
    , m_bRemoveWhenOver( a_bRemoveWhenOver )
    , m_bCanSkip( TTRUE )
    , m_bDestroyUnits( a_bDestroyUnits )
    , m_bRestartMusic( a_bRestartMusic )
    , m_bLoadNextState( a_bLoadNextState )
    , m_bRestoreGameTimeFX( a_bRestoreGameTimeFX )
    , m_iNumUnits( 0 )
    , m_fStartTime( 0.0f )
    , m_bStarted( TFALSE )
    , m_bShowChapterTitle( a_bShowChapterTitle )
    , m_bFade( a_bFade )
    , m_bStartImmediately( a_bStartImmediately )
    , m_fSyncSpeed( -1.0f )
    , m_iUnk2( -1 )
    , m_iUnk3( -1 )
    , m_bLoading( TFALSE )
{
	m_apBarnModels[ 0 ] = TNULL;
	m_apBarnModels[ 1 ] = TNULL;
	m_apBarnModels[ 2 ] = TNULL;
	m_apBarnModels[ 3 ] = TNULL;

	// Leaves more time to read the chapter title
	m_fStartDelay = ( a_bShowChapterTitle == TTRUE ) ? 3.0f : 1.0f;

	ms_bSkipped          = TFALSE;
	ms_bFinishOnActivate = TFALSE;

	if ( a_iUnk == 1 )
	{
		m_iUnk2      = 2;
		m_fSyncSpeed = 0.1f;
	}
}

// $Barnyard: FUNCTION 004414f0
// $Barnyard: FUNCTION 00441170
ASSeqState::~ASSeqState()
{
}

// $Barnyard: FUNCTION 00440e30
TBOOL ASSeqState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	ASSeqController* pController = ASSeqController::GetSingleton();

	if ( !pController->IsSeqPlaying() )
		return AGameState::ProcessInput( a_pInputEvent );

	ASSeqUI& rUI = pController->FindScript( m_strName, TNULL )->GetUI();

	if ( !rUI.IsActive() )
	{
		// Skipping the sequence
		if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN && m_fTime >= 1.0f && m_bCanSkip == TTRUE &&
		     ( a_pInputEvent->GetDoodad() == TInputDeviceKeyboard::KEY_ESCAPE || a_pInputEvent->GetDoodad() == TInputDeviceController::BUTTON_START ) )
		{
			if ( AGameLoader::GetScriptedSequence() != 0 || ms_bSkipped )
			{
				if ( m_bRemoveWhenOver == TTRUE )
				{
					if ( m_bLoadNextState == TTRUE )
					{
						pController->StopAllScripts();
						pController->DestroyActors();

						AGameLoader::LeaveMiniGame( TFALSE );
					}
					else
					{
						Remove();
					}
				}
				else
				{
					AGameLoader::LeaveScriptedSequence( TFALSE );
				}

				return TTRUE;
			}

			// The first skip jumps to the end of the sequence
			ms_bSkipped = TTRUE;
			pController->FindScript( m_strName, TNULL )->SkipToFlag( 3 );
			pController->ShowChapterTitle( TFALSE );
			return TTRUE;
		}

		return AGameState::ProcessInput( a_pInputEvent );
	}

	ms_bSkipped = TTRUE;

	return rUI.ProcessInputEvent( a_pInputEvent );
}

// $Barnyard: FUNCTION 00440f70
TBOOL ASSeqState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	ASSeqUI& rUI = ASSeqController::GetSingleton()->FindScript( m_strName, TNULL )->GetUI();

	if ( rUI.IsActive() == TTRUE )
		return rUI.ProcessInputCommand( a_eInputCommand, a_pInputEvent );

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00440fd0
void ASSeqState::OnDeactivate()
{
	m_HUDParams.SetFlags( 0 );

	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetForceHide( TFALSE );

	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00441000
void ASSeqState::Finish()
{
	if ( m_bRemoveWhenOver != TTRUE )
	{
		AGameLoader::LeaveScriptedSequence( TTRUE );
		return;
	}

	if ( m_bLoadNextState != TTRUE )
	{
		Remove();
		return;
	}

	ASSeqController::GetSingleton()->StopAllScripts();
	ASSeqController::GetSingleton()->DestroyActors();

	AGameLoader::LeaveMiniGame( TTRUE );
}

// $Barnyard: FUNCTION 00441040
TBOOL ASSeqState::OnUpdate( TFLOAT a_fDeltaTime )
{
	ASSeqController* pController = ASSeqController::GetSingleton();

	if ( !m_bStarted )
	{
		m_fStartTime += a_fDeltaTime;

		if ( m_fStartTime < m_fStartDelay )
			return AGameState::OnUpdate( a_fDeltaTime );

		pController->StartScript( m_strName, TNULL, 0.0f, m_fSyncSpeed );
		m_bStarted = TTRUE;
	}

	if ( m_bShowChapterTitle == TTRUE )
		pController->SetChapterTitleAlpha( pController->GetChapterTitleAlpha() - a_fDeltaTime * 1.5f );

	m_fTime += a_fDeltaTime;

	ASSeqUI& rUI = ASSeqController::GetSingleton()->FindScript( m_strName, TNULL )->GetUI();

	if ( rUI.IsActive() == TTRUE )
	{
		rUI.Update( a_fDeltaTime );
		return AGameState::OnUpdate( a_fDeltaTime );
	}

	if ( ASSeqController::GetSingleton()->IsSeqPlaying() )
		return AGameState::OnUpdate( a_fDeltaTime );

	Finish();
	return TTRUE;
}

// $Barnyard: FUNCTION 00441210
void ASSeqState::OnActivate()
{
	AGameState::OnActivate();
	m_HUDParams.SetFlags( AHudElement::MASK_LETTERBOX );

	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetForceHide( TTRUE );

	if ( ms_bFinishOnActivate )
		Finish();
}

// $Barnyard: FUNCTION 00441290
void ASSeqState::ReleaseUnits()
{
	for ( TINT i = 0; i < m_iNumUnits; i++ )
		m_aUnits[ i ] = TNULL;

	m_iNumUnits = 0;
}

// $Barnyard: FUNCTION 00441510
// $BarnyardWiiJP: FUNCTION 80150c54
void ASSeqState::OnInsertion()
{
	ATerrainInterface::GetSingleton()->SetGetCurrentVISGroupCallback( ASSeqCameraHelper::GetVISGroup );

	if ( m_bLoading )
	{
		ARootTask::GetSingleton()->SetRenderWorld( TFALSE );
		g_oLoadScreen.StartLoading( 100, TTRUE );
		AGameLoader::Terrain_Load();
	}
	else
	{
		ATerrainInterface::GetSingleton()->WaitUntilLoaded();
	}

	ASSeqController* pController = ASSeqController::GetSingleton();

	if ( !pController->IsActive() )
		pController->Activate( TTRUE );

	m_HUDParams.SetFlags( AHudElement::MASK_LETTERBOX );
	AHudManager::GetSingleton()->AddElement( &m_oLetterBox, TFALSE, TTRUE, TTRUE );

	if ( m_bLoading )
		g_oLoadScreen.Update( 1.0f, TTRUE );

	pController->CreateActors( ASSeqActorData::NUM_ACTORS );

	TString8 strFileName( m_strScriptFile );
	strFileName += ".tso";
	pController->LoadScript( strFileName, m_vecPosition );

	if ( m_bLoading )
		g_oLoadScreen.Update( 1.0f, TTRUE );

	ASSeq* pSSeq      = pController->FindScript( m_strName, TNULL );
	TINT   iNumTracks = pSSeq->GetNumTracks();

	for ( TINT i = 0; i < iNumTracks; i++ )
	{
		TINT iActor = pSSeq->GetTrackActor( i );

		if ( m_bLoading )
			g_oLoadScreen.Update( 1.0f, TTRUE );

		if ( iActor == 2 || iActor == 14 || iActor == 25 || iActor == 27 || iActor == 28 || iActor == 30 || iActor == 33 || iActor == 35 ||
		     iActor == 37 || iActor == 39 || iActor == 41 || iActor == 43 || iActor == 46 || iActor == 86 || iActor == 91 || iActor == 98 ||
		     iActor == 102 || iActor == 108 || iActor == 115 || iActor == 124 || iActor == 132 || iActor == 128 || iActor == 136 || iActor == 141 ||
		     iActor == 152 || iActor == 166 || iActor == 168 || iActor == 170 || iActor == 194 || iActor == 206 || iActor == 213 || iActor == 222 ||
		     iActor == 177 || iActor == 229 || iActor == 236 || iActor == 241
#ifdef BARNYARD_LATEST
		     // The cameras of the Wii sequences (sscuoc, ssbff, ss06 merged, ssul)
		     || iActor == 273 || iActor == 275 || iActor == 279 || iActor == 282
#endif
		)
		{
			// The camera of the sequence
			ACameraManager* pCameraManager = ACameraManager::GetSingleton();
			m_pPreviousCameraHelper        = pCameraManager->GetCurrentCameraHelper();

			ASSeqCameraHelper* pCameraHelper = TSTATICCAST( ASSeqCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_SSEQ ) );

			TString8 strCameraModel( "data\\models\\" );
			strCameraModel += m_strCamera;
			strCameraModel.MakeLower();

			pCameraHelper->Setup( strCameraModel, pSSeq, m_vecPosition );
			pCameraManager->SetCameraHelper( pCameraHelper );

			pController->SetActor( iActor, pCameraHelper ? TSTATICCAST( ASSeqActorInterface, pCameraHelper ) : TNULL, TFALSE );
		}
		else if ( iActor == 0 )
		{
			// The sequence itself
			pController->SetActor( 0, TNULL, TFALSE );
		}
		else if ( iActor == 1 || iActor == 13 || iActor == 24 || iActor == 26 || iActor == 257 || iActor == 29 || iActor == 32 || iActor == 34 || iActor == 36 || iActor == 38 || iActor == 40 || iActor == 42 || iActor == 45 || iActor == 85 || iActor == 90 || iActor == 97 || iActor == 101 || iActor == 107 || iActor == 114 || iActor == 123 || iActor == 131 || iActor == 127 || iActor == 135 || iActor == 140 || iActor == 151 || iActor == 165 || iActor == 167 || iActor == 169 || iActor == 193 || iActor == 205 || iActor == 212 || iActor == 221 || iActor == 176 || iActor == 228 || iActor == 235 || iActor == 240
#ifdef BARNYARD_LATEST
		          // The roots of the Wii sequences (sscuoc, ssbff, ss06 merged, ssul)
		          || iActor == 272 || iActor == 274 || iActor == 280 || iActor == 281
#endif
		)
		{
			// The model the other actors are attached to
			ASSeqAttachmentManager& rAttachmentManager = pController->GetAttachmentManager();
			rAttachmentManager.SetPosition( m_vecPosition );

			pController->SetActor( iActor, &rAttachmentManager, TFALSE );
		}
		else if ( AUnit* pUnit = AUnitManager::GetSingleton()->FindActorBySSeqId( iActor ) )
		{
			pController->SetActor( iActor, TSTATICCAST( AUnitActor, pUnit ), TFALSE );
		}
		else
		{
			// Spawn the actor
			TStringManager::String8Format( ms_szActorTypeName, sizeof( ms_szActorTypeName ), "actor_%s", ASSeqActorData::GetActorName( iActor ) );

			TPString8            strActorType( ms_szActorTypeName );
			AUnitType*           pActorType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( strActorType );
			ASSeqActorInterface* pActor     = TNULL;

			if ( pActorType )
			{
				AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, -1, -1, TTRUE );

				TCHAR szTypeName[ 100 ];
				TStringManager::String8Copy( szTypeName, pActorType->GetNameString() );
				TStringManager::String8ToLowerCase( szTypeName );

				TBOOL bPlayer;

				if ( TStringManager::String8FindString( szTypeName, "player" ) || TStringManager::String8FindString( szTypeName, "american" ) )
				{
					bPlayer = TTRUE;

					// The gender of the player
					oTransform.m_iModelIndex = APlayerProfileManager::GetSingleton()->GetMainGender() != APlayerProfile::GENDER_MALE;
				}
				else
				{
					bPlayer = TFALSE;

					if ( TStringManager::String8FindString( szTypeName, "ssccdef_genrac" ) )
					{
						// The coyote variant of Chick Coop Defender
						oTransform.m_iModelIndex = AMiniGameManager::GetSingleton()->GetMiniGameVariant();
					}
					else if ( AGameLoader::IsMainGameLoaded() && ( TStringManager::String8Compare( "actor_duke_anims", szTypeName ) == 0 || TStringManager::String8Compare( "actor_sim_otis", szTypeName ) == 0 || TStringManager::String8Compare( "actor_nightbarn_bull_anims", szTypeName ) == 0 || TStringManager::String8Compare( "actor_sim_root", szTypeName ) == 0 || TStringManager::String8Compare( "actor_bull_anims", szTypeName ) == 0 ) )
					{
						// The skin of the act
						TINT iAct = AQuestManager::GetSingleton()->GetAct();

						if ( iAct > 1 )
							oTransform.m_iModelIndex = iAct - 1;
					}
				}

				AUnit* pUnit = AUnitManager::GetSingleton()->SpawnUnit( strActorType, &oTransform );

				TSTATICCAST( AVisibleUnit, pUnit )->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

				if ( TStringManager::String8Compare( "actor_ss_truck", szTypeName ) == 0 || TStringManager::String8Compare( "actor_ss_truck_ntrailer", szTypeName ) == 0 )
				{
					TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetModelInstance()->SetUnlit();
				}

				if ( bPlayer )
				{
					TSTATICCAST( AAnimatedUnit, pUnit )->ApplyMaterial( APlayerProfileManager::GetSingleton()->GetMainCostume(), TTRUE );
				}

				m_aUnits[ m_iNumUnits++ ] = pUnit->GetSelfProxy();
				pActor                    = TSTATICCAST( AUnitActor, pUnit );
			}

			pController->SetActor( iActor, pActor, TFALSE );
		}
	}

	if ( m_bFade == TTRUE )
		pController->GetFade()->Reset( TTRUE );

	m_bStarted   = TFALSE;
	m_fStartTime = 0.0f;
	pController->ShowChapterTitle( m_bShowChapterTitle );

	if ( m_bShowChapterTitle == TTRUE )
		pController->SetChapterTitle( pSSeq->GetName() );

	if ( m_bLoading )
		g_oLoadScreen.Update( 1.0f, TTRUE );

	if ( strFileName.Find( "SS10", 0 ) != -1 )
	{
		// The barn of the end
		AKeyFrameLibraryManager::GetSingleton()->LoadLibrary( TPString8( "barn_objs" ), TNULL );
		AMaterialLibraryManager::GetSingleton()->LoadLibrary( TPString8( "barn_objs" ), TNULL, TTRUE );

		const TCHAR* aBarnModels[ NUM_BARN_MODELS ] = {
			"data\\models\\bn_saloon_bar_add",
			"data\\models\\bn_mirror_add",
			"data\\models\\bn_cows_milk",
			"data\\models\\bn_bar_cabinet"
		};

		for ( TINT i = 0; i < NUM_BARN_MODELS; i++ )
		{
			m_apBarnModels[ i ] = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( aBarnModels[ i ] ), TNULL );
			m_apBarnModels[ i ]->SetVisible( TTRUE );
			m_apBarnModels[ i ]->SetSkeletonUpdating( TTRUE );

			m_apBarnModels[ i ]->SetUnlit();

			m_apBarnModels[ i ]->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

			if ( m_bLoading )
				g_oLoadScreen.Update( 1.0f, TTRUE );
		}
	}

	if ( m_bStartImmediately == TTRUE )
	{
		pController->StartScript( m_strName, TNULL, 0.0f, m_fSyncSpeed );
		m_fStartTime = m_fStartDelay;
		m_bStarted   = TTRUE;
	}

	AGameState::OnInsertion();

	if ( m_bLoading )
	{
		g_oLoadScreen.Reset();
		g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );

		ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
	}

	ms_bIsPlaying = TTRUE;
}

// $Barnyard: FUNCTION 004421c0
// $BarnyardWiiJP: FUNCTION 801519dc
void ASSeqState::OnRemoval()
{
	if ( m_iUnk3 > 0 )
		m_iUnk3 = -1;

	if ( m_apBarnModels[ 0 ] || m_apBarnModels[ 1 ] || m_apBarnModels[ 2 ] || m_apBarnModels[ 3 ] )
	{
		AModelRepos* pModelRepos = AModelRepos::GetSingleton();

		for ( TINT i = 0; i < NUM_BARN_MODELS; i++ )
			pModelRepos->DestroyModelInstance( m_apBarnModels[ i ] );

		pModelRepos->UnloadUnusedModels();

		AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPString8( "barn_objs" ) );
		AMaterialLibraryManager::GetSingleton()->UnrefLibrary( TPString8( "barn_objs" ), TTRUE );
	}

	ASSeqController* pController = ASSeqController::GetSingleton();
	pController->StopAllScripts();
	pController->DestroyActors();
	pController->GetAttachmentManager().Reset();

	if ( m_bDestroyUnits )
	{
		for ( TINT i = 0; i < m_iNumUnits; i++ )
		{
			AUnit* pUnit = m_aUnits[ i ] ? m_aUnits[ i ]->GetUnit() : TNULL;

			if ( pUnit )
				AUnitManager::GetSingleton()->DestroyUnit( pUnit );
		}
	}

	ReleaseUnits();

	if ( m_pPreviousCameraHelper )
		ACameraManager::GetSingleton()->SetCameraHelper( m_pPreviousCameraHelper );
	else
		ACameraManager::GetSingleton()->DetachCameraHelpers();

	m_oLetterBox.Detach();

	// The music starts again
	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMusicManager::GetSingleton()->Reset();
	ASoundManager::GetSingleton()->Reset();

	if ( m_bRestartMusic )
		AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );

	AGameState::OnRemoval();

	if ( m_bRestoreGameTimeFX )
	{
		AGameTimeFXManager::GetSingleton()->ClearForcedFX();
	}

#ifdef BARNYARD_LATEST
	// The Wii sequences that are started from the sim fade back in by themselves
	if ( m_strName == "SSBFF_main" )
	{
		// The birthday cake sequence (see ASimState::StartBirthdayCakeSequence)
		ASSeqController::GetSingleton()->GetFade()->FadeIn( 1.0f );
	}
	else if ( m_strName == "SSUL_main" )
	{
		// The tomato launcher sequence opens the tomato gate
		if ( AGateManager::IsSingletonCreated() )
			AGateManager::GetSingleton()->SetTomatoGateOpen( TTRUE );
		ASSeqController::GetSingleton()->GetFade()->FadeIn( 1.0f );
	}
#endif // BARNYARD_LATEST

	ms_bIsPlaying = TFALSE;
}

TSTATICASSERT( sizeof( ASSeqState ) == 0x29c );
