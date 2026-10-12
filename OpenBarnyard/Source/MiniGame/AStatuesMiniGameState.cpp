#include "pch.h"
#include "AStatuesMiniGameState.h"
#include "Sound/APositionalSoundSet.h"
#include "Helpers/AHighScoreManager.h"
#include "AMiniGameManager.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimationBreakpointCallback.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Terrain/ATerrainInterface.h"
#include "Helpers/AGameLoader.h"
#include "Player/APlayerProfileManager.h"
#include "Render/AShadowManager.h"
#include "Sound/ASoundManager.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AGameTimeFXManager.h"
#include "GUI/AFadeManager.h"
#include "Input/AInputMapManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAnticGameOverState.h"
#include "Locale/ALocaleManager.h"

#include <Render/TRenderInterface.h>
#include <Math/TQuaternion.h>
#include <Math/TMatrix44.h>
#include "SoundBank/mrs_b.h"
#include "SoundBank/statues.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( AStatuesMiniGameState );
TPSTRING8_DECLARE( ATeaseMrsBeadyGame );
TPSTRING8_DECLARE( abuse_cow1 );
TPSTRING8_DECLARE( abuse_cow2 );
TPSTRING8_DECLARE( airguitar_1 );
TPSTRING8_DECLARE( americanf_quad );
TPSTRING8_DECLARE( americanf_statue );
TPSTRING8_DECLARE( americanm_quad );
TPSTRING8_DECLARE( americanm_statue );
TPSTRING8_DECLARE( ballet );
TPSTRING8_DECLARE( c_head );
TPSTRING8_DECLARE( c_l_forearm );
TPSTRING8_DECLARE( c_l_hand );
TPSTRING8_DECLARE( c_l_upperarm );
TPSTRING8_DECLARE( c_neck );
TPSTRING8_DECLARE( c_r_forearm );
TPSTRING8_DECLARE( c_r_hand );
TPSTRING8_DECLARE( c_r_upperarm );
TPSTRING8_DECLARE( c_spine1 );
TPSTRING8_DECLARE( c_spine );
TPSTRING8_DECLARE( cleanandjerk );
TPSTRING8_DECLARE( emt_foot_01 );
TPSTRING8_DECLARE( emt_foot_02 );
TPSTRING8_DECLARE( end_lose_spotted );
TPSTRING8_DECLARE( end_lose_timeout );
TPSTRING8_DECLARE( end_pt1 );
TPSTRING8_DECLARE( end_win );
TPSTRING8_DECLARE( fail2 );
TPSTRING8_DECLARE( fail );
TPSTRING8_DECLARE( fall_asleep );
TPSTRING8_DECLARE( golfswing );
TPSTRING8_DECLARE( gradeabeef );
TPSTRING8_DECLARE( hammertime );
TPSTRING8_DECLARE( hide );
TPSTRING8_DECLARE( idle );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( idle_quad );
TPSTRING8_DECLARE( intro );
TPSTRING8_DECLARE( keypress );
TPSTRING8_DECLARE( keypress_loop );
TPSTRING8_DECLARE( land );
TPSTRING8_DECLARE( laugh );
TPSTRING8_DECLARE( locator_cam );
TPSTRING8_DECLARE( locator_camtarget );
TPSTRING8_DECLARE( loser_start );
TPSTRING8_DECLARE( mailman_anims );
TPSTRING8_DECLARE( mailman_faint );
TPSTRING8_DECLARE( mailman_insert );
TPSTRING8_DECLARE( mailman_leave );
TPSTRING8_DECLARE( mailman_looking );
TPSTRING8_DECLARE( mailman_nothing );
TPSTRING8_DECLARE( mailman_searching );
TPSTRING8_DECLARE( mailman_sorting );
TPSTRING8_DECLARE( mailman_suspect );
TPSTRING8_DECLARE( mailman_takeout );
TPSTRING8_DECLARE( moonwalk );
TPSTRING8_DECLARE( mrbeady_anims );
TPSTRING8_DECLARE( mrsbeady_anims );
TPSTRING8_DECLARE( muscleflex );
TPSTRING8_DECLARE( ninja );
TPSTRING8_DECLARE( nothing );
TPSTRING8_DECLARE( quad_idle );
TPSTRING8_DECLARE( ragingbull_1 );
TPSTRING8_DECLARE( ragingbull_2 );
TPSTRING8_DECLARE( robot );
TPSTRING8_DECLARE( rudeboy );
TPSTRING8_DECLARE( sleep1 );
TPSTRING8_DECLARE( sleep2 );
TPSTRING8_DECLARE( sleep3 );
TPSTRING8_DECLARE( starjumps );
TPSTRING8_DECLARE( statues_freddy_anims );
TPSTRING8_DECLARE( statues_otis_anims );
TPSTRING8_DECLARE( statues_pig_anims );
TPSTRING8_DECLARE( stoplook );
TPSTRING8_DECLARE( success );
TPSTRING8_DECLARE( suspect );
TPSTRING8_DECLARE( talk1 );
TPSTRING8_DECLARE( talk2 );
TPSTRING8_DECLARE( tells_mr );
TPSTRING8_DECLARE( thepharaoh );
TPSTRING8_DECLARE( thinker );
TPSTRING8_DECLARE( unhide );
TPSTRING8_DECLARE( up );
TPSTRING8_DECLARE( waiting );
TPSTRING8_DECLARE( winner_start );
TPSTRING8_DECLARE( yell );

// NOTE: GetClass 00485fe0, CreateObject 00489b00, CreateObjectInPlace 00489b60
TDEFINE_CLASS( AStatuesMiniGameState );

//-----------------------------------------------------------------------------
// Lifecycle
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00489850
AStatuesMiniGameState::AStatuesMiniGameState()
{
	m_fComboTimer    = 0.0f;
	m_fWatcherTimer  = 0.0f;
	m_fSortingTimer  = 0.0f;
	m_fGameOverTimer = -1.0f;
	m_fSleepTimer    = 0.0f;
	m_iPoseCue       = -1;
	m_iWatcherCue    = -1;
	m_iMrsBeadyCue1  = -1;
	m_iMrsBeadyCue2  = -1;

	m_pInputMap       = AInputMapManager::GetSingleton()->FindMap( TPS8( AStatuesMiniGameState ) );
	m_iCharPuffEffect = AParticleEffectManager::FindEffect( "statues_charpuff" );
	m_iDustPuffEffect = AParticleEffectManager::FindEffect( "statues_dustpuff" );
}

// $Barnyard: FUNCTION 00489990
// $Barnyard: FUNCTION 00487d10
AStatuesMiniGameState::~AStatuesMiniGameState()
{
}

// $Barnyard: FUNCTION 00485ff0
TBOOL AStatuesMiniGameState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	switch ( a_eInputCommand )
	{
		case AInputCommand_Quit:
		case AInputCommand_Start:
			// Can't pause before the intro has started
			if ( m_bIntroStarted ) break;
			return TTRUE;
		case AInputCommand_Up:
			m_eKey = STATUESKEY_UP;
			return TTRUE;
		case AInputCommand_Down:
			m_eKey = STATUESKEY_DOWN;
			return TTRUE;
		case AInputCommand_Left:
			m_eKey = STATUESKEY_LEFT;
			return TTRUE;
		case AInputCommand_Right:
			m_eKey = STATUESKEY_RIGHT;
			return TTRUE;
	}

	return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00487dd0
void AStatuesMiniGameState::OnInsertion()
{
	m_bShowPlayerFloats = TFALSE;
	m_bIsMrsBeady       = AMiniGameManager::GetSingleton()->GetNextMiniGame()->GetName() == TPS8( ATeaseMrsBeadyGame );

	if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() != 0 )
		m_iTargetScore = 0;
	else
		m_iTargetScore = m_bIsMrsBeady ? TARGET_SCORE_MRSB : TARGET_SCORE;

	m_bOthersFailed = TFALSE;

	if ( !m_bIsMrsBeady )
	{
		AGameLoader::Terrain_Set( AGameLoader::Terrain_Statue, TFALSE, TTRUE, 0, 0, 0, 0 );

		// Light direction of the statue garden
		TMatrix44 matLight = TRenderInterface::GetSingleton()->GetLightDirection();
		matLight.SetFromQuaternion( TQuaternion( 0.160664f, -0.407868f, -0.836254f, 0.329409f ) );
		TRenderInterface::GetSingleton()->SetLightDirectionMatrix( matLight );

		AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
		pGameTimeFX->SetForcedFX( pGameTimeFX->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_AFTERNOON ) );
	}
	else
	{
		AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvBeadyInt, TFALSE, TTRUE, 0, 0, 0, 0 );

		AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
		pGameTimeFX->SetForcedFX( pGameTimeFX->FindFX( "beadyint" ) );
	}

	m_oHud.Create( &m_GUIElement );
	m_oHud.Hide();

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	APlayerProfileManager*    pProfileManager = APlayerProfileManager::GetSingleton();

	// The player: a statue model and an "on all fours" model
	AAnimatableObjectType* pStatueType = pAnimObjManager->FindType( TPS8( americanm_statue ) );
	AAnimatableObjectType* pQuadType   = pAnimObjManager->FindType( TPS8( americanm_quad ) );

	if ( pProfileManager->GetMainGender() == APlayerProfile::GENDER_FEMALE )
	{
		pStatueType = pAnimObjManager->FindType( TPS8( americanf_statue ) );
		pQuadType   = pAnimObjManager->FindType( TPS8( americanf_quad ) );
	}

	m_pStatue = pAnimObjManager->CreateAnimatableObject( pStatueType, TNULL, 0 );
	m_pStatue->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );

	TVector4    vPlayerPos( 33.750965f, -8.732336f, 5.554926f, 1.0f );
	TQuaternion quatPlayerRot( 0.994019f, 0.0f, -0.10921f, 1.0f );
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "Locator_AmericanM", vPlayerPos, quatPlayerRot );

	// NOTE: retail hands the locator rotation to SetTransform (00610b90) as the direction
	m_pStatue->GetModelInstance()->SetTransform( vPlayerPos, *TREINTERPRETCAST( TVector4*, &quatPlayerRot ) );
	m_pStatue->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pStatue->SetVisible( TTRUE );

	m_pQuad = pAnimObjManager->CreateAnimatableObject( pQuadType, TNULL, 0 );
	m_pQuad->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );
	m_pQuad->GetModelInstance()->SetTransform( vPlayerPos, *TREINTERPRETCAST( TVector4*, &quatPlayerRot ) );
	m_pQuad->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pQuad->SetVisible( TTRUE );

	CreateShadows();

	// NOTE: retail reuses earlier stack variables as the position/rotation of these locators;
	// GetLocatorPosAndRot always overwrites them
	TVector4    vPos;
	TQuaternion quatRot;

	if ( m_bIsMrsBeady )
	{
		m_pWatcher = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( mrbeady_anims ) ), TNULL, 0 );
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "Locator_MrBeady", vPos, quatRot );
		m_pWatcher->GetModelInstance()->SetTransform( vPos, *TREINTERPRETCAST( TVector4*, &quatRot ) );
		m_pWatcher->SetSkeletonUpdating( TTRUE, TTRUE );
		m_pWatcher->SetVisible( TTRUE );

		m_pMrsBeady = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( mrsbeady_anims ) ), TNULL, 0 );
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "Locator_MrsBeady", vPos, quatRot );
		m_pMrsBeady->GetModelInstance()->SetTransform( vPos, *TREINTERPRETCAST( TVector4*, &quatRot ) );
		m_pMrsBeady->SetSkeletonUpdating( TTRUE, TTRUE );
		m_pMrsBeady->SetVisible( TTRUE );
	}
	else
	{
		m_pMrsBeady = TNULL;
		m_pWatcher  = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( mailman_anims ) ), TNULL, 0 );
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "Locator_MailMan", vPos, quatRot );
		m_pWatcher->GetModelInstance()->SetTransform( vPos, *TREINTERPRETCAST( TVector4*, &quatRot ) );
		m_pWatcher->SetSkeletonUpdating( TTRUE, TTRUE );
		m_pWatcher->SetVisible( TTRUE );
	}

	// Animation breakpoint listeners
	AAnimationBreakpointCallback* pBreakpoint = m_pWatcher->GetAnimationEmitter( TPS8( nothing ) );
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail doesn't check if the animation has the breakpoint
	if ( pBreakpoint )
#endif
		m_oWatcherNothingListener.Connect( pBreakpoint->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnWatcherNothing ), 0 );

	pBreakpoint = m_pWatcher->GetAnimationEmitter( TPS8( stoplook ) );
#ifdef BARNYARD_COMMUNITY_PATCH
	if ( pBreakpoint )
#endif
		m_oWatcherStopLookListener.Connect( pBreakpoint->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnWatcherStopLook ), 0 );

	pBreakpoint = m_pStatue->GetAnimationEmitter( TPS8( winner_start ) );
#ifdef BARNYARD_COMMUNITY_PATCH
	if ( pBreakpoint )
#endif
		m_oWinnerStartListener.Connect( pBreakpoint->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnWinnerStart ), 0 );

	if ( !m_bIsMrsBeady )
	{
		// The onlookers (table 007738bc)
		const TPString8* apOtherTypes[] = { &TPS8( statues_pig_anims ), &TPS8( statues_freddy_anims ), &TPS8( statues_otis_anims ) };

		for ( TINT i = 0; i < TARRAYSIZE( apOtherTypes ); i++ )
		{
			AAnimatableObject* pOther = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( *apOtherTypes[ i ] ), TNULL, 0 );

			ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "Locator_OtherCharacters", vPos, quatRot );
			pOther->GetModelInstance()->SetTransform( vPos, *TREINTERPRETCAST( TVector4*, &quatRot ) );
			pOther->SetSkeletonUpdating( TTRUE, TTRUE );
			pOther->SetVisible( TTRUE );
			pOther->PlayAnimation( TPS8( idle ) );

			m_vecOtherCharacters.PushBack( pOther );
		}
	}

	// Camera
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	if ( m_bIsMrsBeady )
	{
		ACamera* pCamera                      = ACameraManager::GetSingleton()->GetCurrentCamera();
		pCamera->GetMatrix().GetTranslation() = ATerrainInterface::GetSingleton()->GetLocatorPos( "Locator_cam" );
		pCamera->LookAtPoint( ATerrainInterface::GetSingleton()->GetLocatorPos( "Locator_camTarget" ) );
	}
	else
	{
		// The camera follows the camera bones of the statue (see UpdateAlways)
		TMatrix44 matBone;

		TINT iCamBone = m_pStatue->GetModelInstance()->GetBoneID( TPS8( locator_cam ) );
		m_pStatue->GetModelInstance()->GetBoneTransform( iCamBone, matBone );
		TVector4 vCameraPos = matBone.GetTranslation();

		TINT iTargetBone = m_pStatue->GetModelInstance()->GetBoneID( TPS8( locator_camtarget ) );
		m_pStatue->GetModelInstance()->GetBoneTransform( iTargetBone, matBone );
		TVector4 vCameraTarget = matBone.GetTranslation();

		ACamera* pCamera                      = ACameraManager::GetSingleton()->GetCurrentCamera();
		pCamera->GetMatrix().GetTranslation() = vCameraPos;
		pCamera->LookAtPoint( vCameraTarget );
		pCamera->SetFOV( 0.75049156f );
	}

	m_bIntroStarted = TFALSE;
	ResetGame();

	// The poses
	const TPString8* apPoseAnims[ NUM_POSES ] = {
		&TPS8( ragingbull_1 ), &TPS8( airguitar_1 ), &TPS8( starjumps ), &TPS8( thinker ),
		&TPS8( cleanandjerk ), &TPS8( rudeboy ), &TPS8( ragingbull_2 ), &TPS8( thepharaoh ),
		&TPS8( golfswing ), &TPS8( muscleflex ), &TPS8( ninja ), &TPS8( robot ),
		&TPS8( ballet ), &TPS8( moonwalk ), &TPS8( gradeabeef ), &TPS8( hammertime )
	};

	for ( TINT i = 0; i < NUM_POSES; i++ )
	{
		AStatuesPose&  rPose = m_aPoses[ i ];
		const PoseDef& rDef  = POSES[ i ];

		for ( TINT k = 0; k < AStatuesPose::MAX_KEYS; k++ )
			rPose.aKeys[ k ] = rDef.aKeys[ k ];

		rPose.strAnim = *apPoseAnims[ i ];

		for ( TINT k = 0; k < AStatuesPose::MAX_STARS; k++ )
			rPose.aScores[ k ] = rDef.aScores[ k ];

		rPose.iLocaleName = rDef.iLocaleName;
		rPose.bUnk        = rDef.bUnk;
		rPose.eSound      = m_bIsMrsBeady ? rDef.eSoundMrsBeady : rDef.eSoundMailman;
	}

	APositionalSoundSet::GetSingleton()->Load( m_bIsMrsBeady ? "Data/Sound/MrsB.trb" : "Data/Sound/Statues.trb" );

	AMiniGameState::OnInsertion();
	m_oTimeLeftHudElement.SetForceHide( TTRUE );

	// Shuffle the poses of each row
	for ( TINT iRow = 0; iRow < NUM_ROWS; iRow++ )
	{
		TINT iNumPoses = 0;

		for ( TINT i = 0; i < NUM_POSES; i++ )
		{
			if ( m_aPoses[ i ].aKeys[ 0 ] == iRow )
				m_aShuffle[ iNumPoses++ ] = i;
		}

		TINT iNumLeft = iNumPoses;

		for ( TINT i = 0; i < MAX_ROW_POSES; i++, iNumLeft-- )
		{
			if ( i >= iNumPoses )
			{
				m_aRowPoseOrder[ iRow ][ i ] = -1;
				continue;
			}

			// Pick a random pose that hasn't been picked yet
			TINT iRandom = ARandom::GetSingleton()->m_oRandom.GetInt( iNumLeft );
			TINT iIndex  = 0;

			for ( ; iRandom > 0 || m_aShuffle[ iIndex ] == -1; iIndex++ )
			{
				if ( m_aShuffle[ iIndex ] != -1 )
					iRandom--;
			}

			m_aRowPoseOrder[ iRow ][ i ] = m_aShuffle[ iIndex ];
			m_aShuffle[ iIndex ]         = -1;
		}
	}

	for ( TINT iRow = 0; iRow < NUM_ROWS; iRow++ )
		m_aRowCursors[ iRow ] = -1;

	// NOTE: AdvanceRowPose is inlined here
	for ( TINT iRow = 0; iRow < NUM_ROWS; iRow++ )
		AdvanceRowPose( iRow );
}

// $Barnyard: FUNCTION 004892f0
void AStatuesMiniGameState::OnRemoval()
{
	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	APositionalSoundSet::GetSingleton()->Unload();

	m_oHud.Destroy();
	RemoveShadows();

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	pAnimObjManager->DestroyAnimatableObject( m_pStatue );
	pAnimObjManager->DestroyAnimatableObject( m_pQuad );
	pAnimObjManager->DestroyAnimatableObject( m_pWatcher );
	pAnimObjManager->DestroyAnimatableObject( m_pMrsBeady );

	while ( m_vecOtherCharacters.Size() != 0 )
	{
		pAnimObjManager->DestroyAnimatableObject( m_vecOtherCharacters[ m_vecOtherCharacters.Size() - 1 ] );
		m_vecOtherCharacters.PopBack();
	}

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();
	AGameLoader::DestroyLevel();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( TMath::PI / 3.0f );
	m_oPoseListener.Disconnect();
	ASoundManager::GetSingleton()->Reset();

	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00486090
void AStatuesMiniGameState::OnActivate()
{
	if ( m_oStartHudElement.IsForceHidden() == TTRUE && !IsGameOver() )
		m_oHud.Show();

	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 004860c0
void AStatuesMiniGameState::OnDeactivate()
{
	m_oHud.Hide();
	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 004860e0
void AStatuesMiniGameState::ResetGame()
{
	m_eKey           = STATUESKEY_NONE;
	m_bComboReset    = TTRUE;
	m_bReshuffleRows = TFALSE;
	m_ePlayerState   = PLAYERSTATE_STANDING;
	m_eWatcherState  = WATCHERSTATE_WAITING;
	m_fSortingTimer  = 0.0f;
	m_fTimeLimit     = TIME_LIMIT;
	m_iScore         = 0;

	for ( TINT i = 0; i < NUM_ROWS; i++ )
		m_aRowScores[ i ] = 0;

	m_iCurrentPose   = NO_POSE;
	m_fStarFadeTimer = 0.0f;

	m_oHud.SetDuration( TIME_LIMIT );
	m_oHud.SetTime( TIME_LIMIT );
}

// $Barnyard: FUNCTION 00487a60
void AStatuesMiniGameState::CreateShadows()
{
	AShadowManager* pShadowManager = AShadowManager::GetSingleton();

	// NOTE: AModelInstance::GetBoneID( const TCHAR* ) (00610e40) inlined
	TINT iPelvisBone = m_pStatue->GetModelInstance()->GetSkeleton()->GetBoneID( "c_pelvis", 0 );
	pShadowManager->AddModelShadow( m_pStatue->GetModelInstance(), 1.5f, iPelvisBone, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );

	TINT iSpineBone = m_pQuad->GetModelInstance()->GetSkeleton()->GetBoneID( "c_spine1", 0 );
	pShadowManager->AddModelShadow( m_pQuad->GetModelInstance(), TVector2( 1.5f, 4.0f ), iSpineBone, -0.1f, AShadowManager::FLAGS_NONUNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );
	pShadowManager->SetShadowEnabled( m_pQuad->GetModelInstance(), TFALSE );
}

// $Barnyard: FUNCTION 00487c40
void AStatuesMiniGameState::RemoveShadows()
{
	AShadowManager::GetSingleton()->RemoveShadow( m_pStatue->GetModelInstance() );
	AShadowManager::GetSingleton()->RemoveShadow( m_pQuad->GetModelInstance() );
}

// $Barnyard: FUNCTION 004861b0
void AStatuesMiniGameState::UpdatePreStart( TFLOAT a_fDeltaTime )
{
	if ( m_bIntroStarted )
		return;

	// NOTE: retail StartFade (0045cb20) has a 4th argument, TTRUE here, that our signature doesn't have yet
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
	m_pStatue->PlayAnimation( TPS8( intro ) );

	if ( m_bIsMrsBeady )
	{
		m_iPoseCue = ASoundManager::GetSingleton()->PlayCue( soundbank::MRS_B_INTRO );
		m_pMrsBeady->PlayAnimation( TPS8( waiting ) );
		m_pWatcher->PlayAnimation( TPS8( sleep1 ) );
		m_bAsleep = TFALSE;
	}
	else
	{
		m_pWatcher->PlayAnimation( TPS8( mailman_searching ) );
		m_iPoseCue = ASoundManager::GetSingleton()->PlayCue( soundbank::STATUES_INTRO );
	}

	m_bIntroStarted = TTRUE;
}

// $Barnyard: FUNCTION 00486160
void AStatuesMiniGameState::OnStartHudFinished()
{
	m_eKey = STATUESKEY_NONE;
	m_oHud.Show();
	AMiniGameState::OnStartHudFinished();
}

// $Barnyard: FUNCTION 00486520
AMiniGameState::S1* AStatuesMiniGameState::GetResults()
{
	m_oResults.iResult        = ( m_iScore < m_iTargetScore || m_iScore <= 0 ) ? -1 : 0;
	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];
	rInfo.bUnk1       = TTRUE;
	rInfo.iUnk2       = m_iScore;
	m_oResults.bWon   = m_oResults.iResult != -1;

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	for ( TINT i = 0; i < NUM_ROWS; i++ )
		rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_ROW_SCORE + i ), m_aRowScores[ i ] } );

	if ( m_eWatcherState == WATCHERSTATE_CAUGHT )
		rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_CAUGHT ), CAUGHT_PENALTY } );

	m_oResults.ComputeTotal( 0 );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		if ( TUINT( AMiniGameManager::GetSingleton()->GetMiniGameVariant() ) <= 1 )
		{
			AHighScoreManager::AddScore( m_bIsMrsBeady ? HIGHSCORE_TABLE_MRSB : HIGHSCORE_TABLE, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
		}
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 00486180
TBOOL AStatuesMiniGameState::IsGameOver()
{
	if ( m_eWatcherState >= WATCHERSTATE_CAUGHT )
		return TTRUE;

	return m_iTargetScore > 0 && m_iScore >= m_iTargetScore && m_ePlayerState != PLAYERSTATE_POSING;
}

//-----------------------------------------------------------------------------
// Gameplay
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00489ba0
void AStatuesMiniGameState::UpdateGame( TFLOAT a_fDeltaTime )
{
	if ( !IsGameOver() )
	{
		m_oHud.Update( a_fDeltaTime );
		m_oHud.SetTime( m_fTimeLimit - m_fElapsedTime );
	}

	if ( m_bIsMrsBeady )
		UpdateMrsBeady( a_fDeltaTime );
	else
		UpdateMailman( a_fDeltaTime );

	m_fComboTimer += a_fDeltaTime;

	if ( m_ePlayerState == PLAYERSTATE_POSING )
	{
		AStatuesPose& rPose = m_aPoses[ m_iCurrentPose ];

		if ( m_pStatue->IsAnimPlaying( rPose.strAnim, 0.0f ) == TTRUE )
		{
			// Stars are earned while the pose animation plays, the breakpoint marks the end of the pose
			TFLOAT fPoseLength = m_pStatue->GetAnimationEmitter( rPose.strAnim )->GetTime();
			TFLOAT fProgress   = m_pStatue->GetAnimationTime( rPose.strAnim ) / fPoseLength;
			fProgress          = TMath::Min( fProgress, 1.0f );

			TINT iMaxStars  = rPose.aKeys[ 0 ] + 3;
			TINT iNumStars  = TINT( iMaxStars * fProgress );
			TINT iPrevStars = m_oHud.GetNumStars();
			TINT iPoints    = 0;

			if ( iNumStars > iPrevStars )
			{
				iPoints = rPose.aScores[ iNumStars - 1 ];

				if ( iNumStars > 1 )
					iPoints -= rPose.aScores[ iNumStars - 2 ];

				if ( iNumStars < iMaxStars )
					ASoundManager::GetSingleton()->PlayCue( soundbank::UI_DING );
			}

			m_oHud.SetNumStars( iNumStars, iPoints );
			m_iScore += iPoints;
			m_aRowScores[ rPose.aKeys[ 0 ] ] += iPoints;
			m_oHud.SetScore( m_iScore );

			// The onlookers laugh once the last star but one is earned
			if ( iMaxStars - 1 == iNumStars && iNumStars != iPrevStars )
			{
				T2_FOREACH( m_vecOtherCharacters, it )
				{
					( *it )->PlayAnimation( TPS8( laugh ) );
				}
			}
		}
	}
	else if ( m_fStarFadeTimer <= 0.0f )
	{
		m_oHud.SetNumStars( 0, 0 );
		m_oHud.SetStarPanel( 0 );
	}
	else
	{
		m_fStarFadeTimer -= a_fDeltaTime;
		m_fStarFadeTimer = TMath::Max( m_fStarFadeTimer, 0.0f );
	}

	if ( m_eKey != STATUESKEY_NONE && m_ePlayerState != PLAYERSTATE_GAME_OVER && m_ePlayerState != PLAYERSTATE_DROPPING )
	{
		if ( m_ePlayerState == PLAYERSTATE_ON_ALL_FOURS && m_eKey != STATUESKEY_DOWN )
		{
			// Get up
			m_pQuad->PlayAnimation( TPS8( up ) );
			StandUp();
			m_ePlayerState = PLAYERSTATE_GETTING_UP;
		}

		if ( m_ePlayerState == PLAYERSTATE_POSING )
		{
			if ( m_eKey == STATUESKEY_DOWN )
			{
				// Cancel the pose
				if ( m_bReshuffleRows )
				{
					m_bReshuffleRows = TFALSE;

					for ( TINT i = 0; i < NUM_ROWS; i++ )
						AdvanceRowPose( i );
				}

				m_oHud.SetInputVisible( TTRUE );
				m_iCurrentPose   = NO_POSE;
				m_fStarFadeTimer = 1.0f;
				m_ePlayerState   = PLAYERSTATE_STANDING;
				m_oPoseListener.Disconnect();
			}
		}
		else
		{
			AStatuesPose* pPose = m_oHud.HandleKey( m_eKey );

			if ( pPose )
			{
				if ( pPose->aKeys[ 0 ] == STATUESKEY_NONE )
				{
					// The key didn't match
					m_fComboTimer = 1.0f;
				}
				else
				{
					StartPose( pPose );
				}
			}
			else
			{
				// The combo goes on
				m_fComboTimer = 0.0f;
				m_bComboReset = TFALSE;

				if ( !m_pStatue->IsAnimPlaying( TPS8( keypress ), 0.0f ) && !m_pStatue->IsAnimPlaying( TPS8( keypress_loop ), 0.0f ) )
					m_pStatue->PlayAnimation( TPS8( keypress ) );
			}
		}
	}

	if ( m_bReshuffleRows && m_fComboTimer > 0.7f )
	{
		m_bReshuffleRows = TFALSE;
		m_oHud.SetInputVisible( TFALSE );

		for ( TINT i = 0; i < NUM_ROWS; i++ )
			AdvanceRowPose( i );
	}

	if ( m_fComboTimer > 0.7f && !m_bComboReset )
	{
		// The combo timed out
		m_bComboReset = TTRUE;
		m_oHud.HandleKey( STATUESKEY_NONE );
		m_oHud.ResetRows();

		if ( m_pStatue->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) == TTRUE ||
		     m_pStatue->IsAnimPlaying( TPS8( keypress ), 0.0f ) ||
		     m_pStatue->IsAnimPlaying( TPS8( keypress_loop ), 0.0f ) )
		{
			m_pStatue->PlayAnimation( TPS8( fail ) );
		}
	}

	if ( m_ePlayerState == PLAYERSTATE_DROPPING && m_pStatue->IsAnimationFinished( TPS8( idle_quad ) ) )
	{
		m_pStatue->SetSkeletonUpdating( TFALSE, TTRUE );
		BecomeQuad();
	}

	if ( m_ePlayerState != PLAYERSTATE_ON_ALL_FOURS && m_ePlayerState != PLAYERSTATE_DROPPING && m_ePlayerState != PLAYERSTATE_GAME_OVER && m_eKey == STATUESKEY_DOWN )
	{
		// Drop onto all fours
		m_ePlayerState = PLAYERSTATE_DROPPING;
		m_oHud.HandleKey( STATUESKEY_NONE );

		ASoundManager::GetSingleton()->StopCue( m_iPoseCue );
		m_iPoseCue    = -1;
		m_fComboTimer = 0.0f;
		m_bComboReset = TTRUE;

		PlayEffectAtBoneName( m_pStatue, TPS8( c_l_upperarm ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_l_forearm ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_l_hand ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_r_upperarm ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_r_forearm ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_r_hand ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_spine ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_spine1 ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_neck ), m_iCharPuffEffect );
		PlayEffectAtBoneName( m_pStatue, TPS8( c_head ), m_iCharPuffEffect );

		m_pStatue->PlayAnimation( TPS8( idle_quad ) );
		m_pQuad->PlayAnimation( TPS8( land ) );
		m_pQuad->SetSkeletonUpdating( TTRUE, TTRUE );
		AShadowManager::GetSingleton()->SetShadowEnabled( m_pQuad->GetModelInstance(), TTRUE );
	}

	m_eKey = STATUESKEY_NONE;
}

// $Barnyard: FUNCTION 00489400
void AStatuesMiniGameState::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	if ( m_eWatcherState < WATCHERSTATE_CAUGHT )
	{
		// The game has just ended (score reached or time out)
		m_oHud.Hide();

		if ( m_ePlayerState == PLAYERSTATE_POSING )
		{
			m_fStarFadeTimer = 1.0f;
			m_oPoseListener.Disconnect();
		}

		m_eWatcherState = WATCHERSTATE_ENDED;
		StandUp();

		if ( m_bIsMrsBeady )
		{
			if ( m_iScore < m_iTargetScore )
			{
				m_pMrsBeady->PlayAnimation( TPS8( end_lose_timeout ) );
				m_pWatcher->PlayAnimation( TPS8( end_lose_timeout ) );
				m_pStatue->PlayAnimation( TPS8( end_lose_timeout ) );
			}
			else
			{
				m_pMrsBeady->PlayAnimation( TPS8( end_win ) );
				m_pWatcher->PlayAnimation( TPS8( end_win ) );
				m_pStatue->PlayAnimation( TPS8( end_win ) );

				ASoundManager::GetSingleton()->StopCue( m_iMrsBeadyCue1 );
				m_iMrsBeadyCue1 = ASoundManager::GetSingleton()->PlayCue( soundbank::MRS_B_MRS_B_SHRIEKS1 );
			}
		}
		else
		{
			m_pWatcher->PlayAnimation( TPS8( mailman_leave ) );
			m_pStatue->PlayAnimation( ( m_iScore < m_iTargetScore ) ? TPS8( end_pt1 ) : TPS8( winner_start ) );
		}

		ASoundManager::GetSingleton()->StopCue( m_iPoseCue );
		ASoundManager::GetSingleton()->StopCue( m_iWatcherCue );

		m_pQuad->SetSkeletonUpdating( TFALSE, TTRUE );
		AShadowManager::GetSingleton()->SetShadowEnabled( m_pQuad->GetModelInstance(), TFALSE );
	}

	m_ePlayerState = PLAYERSTATE_GAME_OVER;

	if ( m_eWatcherState == WATCHERSTATE_CAUGHT )
	{
		if ( m_pWatcher->IsAnimationFinished( TPS8( mailman_faint ) ) && !m_bOthersFailed )
		{
			T2_FOREACH( m_vecOtherCharacters, it )
			{
				( *it )->PlayAnimation( TPS8( fail ) );
			}

			m_bOthersFailed = TTRUE;
		}

		if ( m_pStatue->IsAnimationFinished( TPS8( fail2 ) ) && m_pStatue->IsAnimationFinished( TPS8( end_lose_spotted ) ) )
			AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );
	}
	else if ( m_pStatue->IsAnimationFinished( TPS8( end_pt1 ) ) && m_pStatue->IsAnimationFinished( TPS8( winner_start ) ) && m_fGameOverTimer == -1.0f )
	{
		if ( m_iScore < m_iTargetScore && !m_bIsMrsBeady )
			m_pStatue->PlayAnimation( TPS8( loser_start ) );

		m_fGameOverTimer = 0.0f;
	}
	else if ( m_fGameOverTimer >= 0.0f )
	{
		m_fGameOverTimer += a_fDeltaTime;

		if ( m_fGameOverTimer >= 4.4f )
			AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );
	}
}

// $Barnyard: FUNCTION 00486810
void AStatuesMiniGameState::UpdateAlways( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fMaxRotationSpeed = TMath::PI * ( 1.0f / 3.0f );

	if ( m_bIsMrsBeady )
		return;

	// Move the camera towards the camera bone of the statue and turn it towards the target bone
	TMatrix44 matBone;

	TINT iCamBone = m_pStatue->GetModelInstance()->GetBoneID( TPS8( locator_cam ) );
	m_pStatue->GetModelInstance()->GetBoneTransform( iCamBone, matBone );
	TVector4 vCameraPos = matBone.GetTranslation();

	TINT iTargetBone = m_pStatue->GetModelInstance()->GetBoneID( TPS8( locator_camtarget ) );
	m_pStatue->GetModelInstance()->GetBoneTransform( iTargetBone, matBone );
	TVector4 vCameraTarget = matBone.GetTranslation();

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->SetFOV( 0.75049156f );

	TVector4& rCameraPos = pCamera->GetMatrix().GetTranslation();
	TVector4  vOldPos    = rCameraPos;

	// Position: 5 units per second at most
	TVector4 vDelta( vCameraPos.x - vOldPos.x, vCameraPos.y - vOldPos.y, vCameraPos.z - vOldPos.z, vCameraPos.w );
	TFLOAT   fDistance = vDelta.NormaliseMag();
	TFLOAT   fMaxMove  = a_fDeltaTime * 5.0f;

	TVector4 vMove;
	vMove.Multiply( vDelta, ( fMaxMove < fDistance ) ? fMaxMove : fDistance );

	rCameraPos = TVector4( vMove.x + vOldPos.x, vMove.y + vOldPos.y, vMove.z + vOldPos.z, vOldPos.w );

	// Rotation: s_fMaxRotationSpeed radians per second at most
	TVector4 vForward = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );
	vForward.w        = 1.0f;

	TVector4 vToTarget( vCameraTarget.x - rCameraPos.x, vCameraTarget.y - rCameraPos.y, vCameraTarget.z - rCameraPos.z, vCameraTarget.w );
	vToTarget.Normalise();

	TFLOAT fAngle    = vForward.GetAngleBetweenVector( vToTarget );
	TFLOAT fMaxAngle = s_fMaxRotationSpeed * a_fDeltaTime;
	fAngle           = ( fMaxAngle < fAngle ) ? fMaxAngle : fAngle;

	// Rotate the forward vector around the axis by fAngle (Rodrigues)
	TVector4 vAxis;
	vAxis.CrossProduct( vForward, vToTarget );
	vAxis.Normalise();

	TVector4 vAxisCrossForward;
	vAxisCrossForward.CrossProduct( vAxis, vForward );

	TVector4 vParallel;
	vParallel.Multiply( vAxis, TVector4::DotProduct3( vForward, vAxis ) );

	TFLOAT fCos = TMath::Cos( fAngle );

	TVector4 vParallelPart;
	vParallelPart.Multiply( vParallel, 1.0f - fCos );

	TVector4 vSinPart;
	vSinPart.Multiply( vAxisCrossForward, TMath::Sin( fAngle ) );

	TVector4 vCosPart;
	vCosPart.Multiply( vForward, fCos );

	TVector4 vDirection(
	    vParallelPart.x + ( vSinPart.x + vCosPart.x ),
	    vParallelPart.y + ( vSinPart.y + vCosPart.y ),
	    vParallelPart.z + ( vSinPart.z + vCosPart.z ),
	    vCosPart.w
	);

	pCamera->LookAtDirection( vDirection );
}

// $Barnyard: FUNCTION 00486e40
void AStatuesMiniGameState::UpdateMailman( TFLOAT a_fDeltaTime )
{
	m_fWatcherTimer += a_fDeltaTime;

	switch ( m_eWatcherState )
	{
		case WATCHERSTATE_WAITING: // searching
			if ( m_fWatcherTimer > 5.0f && !m_pWatcher->IsAnimPlaying( TPS8( mailman_takeout ), 0.0f ) )
			{
				m_pWatcher->PlayAnimation( TPS8( mailman_takeout ) );
				m_eWatcherState = WATCHERSTATE_TAKING_OUT;
			}
			break;
		case WATCHERSTATE_TAKING_OUT: // taking out the mail
			if ( !m_pWatcher->IsAnimPlaying( TPS8( mailman_takeout ), 0.0f ) )
			{
				m_eWatcherState = WATCHERSTATE_SORTING;
				m_fWatcherTimer = 0.0f;

				if ( !m_pWatcher->IsAnimPlaying( TPS8( mailman_sorting ), 0.0f ) )
					m_pWatcher->PlayAnimation( TPS8( mailman_sorting ) );
			}
			break;
		case WATCHERSTATE_SORTING: // sorting
			m_fSortingTimer += a_fDeltaTime;

			if ( m_ePlayerState == PLAYERSTATE_ON_ALL_FOURS || m_fSortingTimer <= 5.0f || m_pWatcher->IsAnimPlaying( TPS8( mailman_sorting ), 0.0f ) != TTRUE )
			{
				if ( m_fWatcherTimer > 5.0f )
				{
					m_fWatcherTimer = 0.0f;
					m_pWatcher->PlayAnimation( TPS8( mailman_insert ) );
					m_eWatcherState = WATCHERSTATE_PUTTING_IN;
				}
			}
			else
			{
				// Something is going on behind him...
				m_fSortingTimer = 0.0f;
				m_pWatcher->PlayAnimation( TPS8( mailman_suspect ) );
				m_fWatcherTimer = 0.0f;
				m_eWatcherState = WATCHERSTATE_SUSPECTING;
			}
			break;
		case WATCHERSTATE_PUTTING_IN: // putting the mail in
			if ( !m_pWatcher->IsAnimPlaying( TPS8( mailman_insert ), 0.0f ) )
			{
				m_pWatcher->PlayAnimation( TPS8( mailman_searching ) );
				m_eWatcherState = WATCHERSTATE_WAITING;
				m_fWatcherTimer = 0.0f;
			}
			break;
		case WATCHERSTATE_SUSPECTING: // suspecting
			if ( m_pWatcher->IsAnimationFinished( TPS8( mailman_suspect ) ) )
			{
				m_pWatcher->PlayAnimation( TPS8( mailman_looking ) );
				m_eWatcherState = WATCHERSTATE_TURNING;
				m_fWatcherTimer = 0.0f;

				T2_FOREACH( m_vecOtherCharacters, it )
				{
					( *it )->PlayAnimation( TPS8( hide ) );
				}
			}
			break;
		case WATCHERSTATE_TURNING: // turning around
			if ( !m_pWatcher->IsAnimPlaying( TPS8( mailman_looking ), 0.0f ) )
			{
				m_eWatcherState = WATCHERSTATE_LOOKING;

				if ( m_ePlayerState != PLAYERSTATE_ON_ALL_FOURS )
				{
					UpdateMailman_Caught();
					break;
				}

				m_fWatcherTimer = 0.0f;
				m_pWatcher->PlayAnimation( TPS8( mailman_nothing ) );
			}
			break;
		case WATCHERSTATE_LOOKING:
			if ( m_ePlayerState != PLAYERSTATE_ON_ALL_FOURS )
				UpdateMailman_Caught();
			break;
		case WATCHERSTATE_NOTHING_TO_SEE: // nothing to see
			if ( !m_pWatcher->IsAnimPlaying( TPS8( mailman_nothing ), 0.0f ) )
			{
				ASoundManager::GetSingleton()->StopCue( m_iWatcherCue );
				m_iWatcherCue = -1;
				m_pWatcher->PlayAnimation( TPS8( mailman_sorting ) );
				m_fWatcherTimer = 0.0f;
				m_fSortingTimer = 0.0f;
				m_eWatcherState = WATCHERSTATE_SORTING;
			}
			break;
	}
}

// NOTE: shared tail of cases 5 and 6 of UpdateMailman (LABEL_25), the mailman sees the player
void AStatuesMiniGameState::UpdateMailman_Caught()
{
	m_pWatcher->PlayAnimation( TPS8( mailman_faint ) );
	m_pStatue->PlayAnimation( TPS8( fail2 ) );
	m_pQuad->SetSkeletonUpdating( TFALSE, TTRUE );
	AShadowManager::GetSingleton()->SetShadowEnabled( m_pQuad->GetModelInstance(), TFALSE );

	ASoundManager::GetSingleton()->StopCue( m_iPoseCue );
	ASoundManager::GetSingleton()->StopCue( m_iWatcherCue );

	m_eWatcherState = WATCHERSTATE_CAUGHT;
	m_ePlayerState  = PLAYERSTATE_GAME_OVER;
	m_fWatcherTimer = 0.0f;
	m_iScore += CAUGHT_PENALTY;
	m_oHud.Hide();
}

// $Barnyard: FUNCTION 004872c0
void AStatuesMiniGameState::UpdateMrsBeady( TFLOAT a_fDeltaTime )
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( pSoundManager->GetCueSoundId( m_iMrsBeadyCue1 ) == soundbank::MRS_B_MRS_B_SEES1 )
		UpdateTalk( m_pMrsBeady, m_iMrsBeadyCue1 );

	UpdateTalk( m_pWatcher, m_iMrsBeadyCue2 );

	switch ( m_eWatcherState )
	{
		case WATCHERSTATE_WAITING: // Mrs Beady is waiting
			UpdateMrBeadySleep( a_fDeltaTime );

			if ( m_ePlayerState != PLAYERSTATE_ON_ALL_FOURS && !pSoundManager->IsCueUsed( m_iMrsBeadyCue1 ) )
			{
				m_eWatcherState = WATCHERSTATE_BEADY_SEES;
				m_fNextYellTime = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 3.0f, 10.0f );
			}
			break;
		case WATCHERSTATE_BEADY_SEES: // Mrs Beady has seen the player
			UpdateMrBeadySleep( a_fDeltaTime );

			if ( !m_pMrsBeady->GetToshiAnimInterface().GetAnim() || m_pMrsBeady->IsAnimPlaying( TPS8( waiting ), 0.0f ) )
			{
				static TFLOAT s_fNumChoices = 5.0f;
				static TFLOAT s_fAbuseCow2  = ( s_fNumChoices - 1.0f ) / s_fNumChoices;
				static TFLOAT s_fTellsMr    = 2.0f / s_fNumChoices;

				TFLOAT fRandom = ARandom::GetSingleton()->m_oRandom.GetFloat();

				if ( fRandom > s_fAbuseCow2 )
				{
					m_pMrsBeady->PlayAnimation( TPS8( abuse_cow1 ) );
				}
				else if ( fRandom > s_fTellsMr )
				{
					m_pMrsBeady->PlayAnimation( TPS8( abuse_cow2 ) );
				}
				else
				{
					m_pMrsBeady->PlayAnimation( TPS8( tells_mr ) );
					m_iMrsBeadyCue1 = pSoundManager->PlayCue( soundbank::MRS_B_MRS_B_SEES1 );
					PlayRandomTalk( m_pMrsBeady );
				}
			}

			m_fNextYellTime -= a_fDeltaTime;

			if ( m_fNextYellTime < 0.0f && !pSoundManager->IsCueUsed( m_iMrsBeadyCue1 ) )
			{
				// Wake Mr Beady up
				m_fNextYellTime = 0.0f;
				m_eWatcherState = WATCHERSTATE_MR_BEADY_WAKES;
				m_pMrsBeady->PlayAnimation( TPS8( yell ) );
				m_pWatcher->PlayAnimation( TPS8( suspect ) );
				m_iMrsBeadyCue1 = pSoundManager->PlayCue( soundbank::MRS_B_MRS_B_TELLS6 );
			}
			else if ( m_ePlayerState == PLAYERSTATE_ON_ALL_FOURS )
			{
				m_eWatcherState = WATCHERSTATE_WAITING;
				m_pMrsBeady->PlayAnimation( TPS8( waiting ) );
			}
			break;
		case WATCHERSTATE_MR_BEADY_WAKES: // Mr Beady wakes up
			if ( !m_pWatcher->IsAnimPlaying( TPS8( suspect ), 0.0f ) )
			{
				m_eWatcherState = WATCHERSTATE_LOOKING;

				if ( m_ePlayerState != PLAYERSTATE_ON_ALL_FOURS )
				{
					UpdateMrsBeady_Caught();
					break;
				}

				m_pWatcher->PlayAnimation( TPS8( nothing ) );
				m_pMrsBeady->PlayAnimation( TPS8( nothing ) );
			}
			break;
		case WATCHERSTATE_LOOKING:
			if ( m_ePlayerState != PLAYERSTATE_ON_ALL_FOURS )
				UpdateMrsBeady_Caught();
			break;
		case WATCHERSTATE_NOTHING_TO_SEE: // nothing to see, back to sleep
			if ( !m_pWatcher->IsAnimPlaying( TPS8( nothing ), 0.0f ) )
			{
				m_eWatcherState = WATCHERSTATE_WAITING;
				m_pWatcher->PlayAnimation( TPS8( idle ) );
				m_fSleepTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 3.0f, 6.0f );
			}
			break;
	}
}

// NOTE: shared tail of cases 4 and 6 of UpdateMrsBeady (LABEL_29), Mr Beady sees the player
void AStatuesMiniGameState::UpdateMrsBeady_Caught()
{
	m_eWatcherState = WATCHERSTATE_CAUGHT;
	m_pMrsBeady->PlayAnimation( TPS8( end_lose_spotted ) );
	m_pWatcher->PlayAnimation( TPS8( end_lose_spotted ) );
	m_pStatue->PlayAnimation( TPS8( end_lose_spotted ) );
	m_pQuad->SetSkeletonUpdating( TFALSE, TTRUE );
	AShadowManager::GetSingleton()->SetShadowEnabled( m_pQuad->GetModelInstance(), TFALSE );

	ASoundManager::GetSingleton()->StopCue( m_iPoseCue );
	ASoundManager::GetSingleton()->StopCue( m_iWatcherCue );

	m_ePlayerState  = PLAYERSTATE_GAME_OVER;
	m_fWatcherTimer = 0.0f;
	m_iScore += CAUGHT_PENALTY;
	m_oHud.Hide();
}

// $Barnyard: FUNCTION 004862a0
void AStatuesMiniGameState::UpdateMrBeadySleep( TFLOAT a_fDeltaTime )
{
	if ( m_fSleepTimer > 0.0f )
	{
		m_fSleepTimer -= a_fDeltaTime;

		if ( m_fSleepTimer < 0.0f )
		{
			m_fSleepTimer = 0.0f;
			m_pWatcher->PlayAnimation( TPS8( fall_asleep ) );
			m_bAsleep = TTRUE;
		}

		return;
	}

	if ( m_pWatcher->GetToshiAnimInterface().GetAnim() )
		return;

	static TFLOAT s_fNumChoices = 6.0f;
	static TFLOAT s_fSleep3     = ( s_fNumChoices - 1.0f ) / s_fNumChoices;
	static TFLOAT s_fSleep1     = 3.0f / s_fNumChoices;

	TFLOAT fRandom = ARandom::GetSingleton()->m_oRandom.GetFloat();

	if ( m_bAsleep || fRandom <= s_fSleep1 )
	{
		m_pWatcher->PlayAnimation( TPS8( sleep1 ) );
		m_bAsleep = TFALSE;
	}
	else
	{
		m_pWatcher->PlayAnimation( ( fRandom <= s_fSleep3 ) ? TPS8( sleep2 ) : TPS8( sleep3 ) );
		m_bAsleep = TTRUE;
	}
}

// $Barnyard: FUNCTION 00487750
void AStatuesMiniGameState::StartPose( AStatuesPose* a_pPose )
{
	m_pStatue->PlayAnimation( a_pPose->strAnim );
	m_ePlayerState = PLAYERSTATE_POSING;
	m_bComboReset  = TTRUE;

	TINT iRow      = a_pPose->aKeys[ 0 ];
	m_iCurrentPose = m_aRowPoseOrder[ iRow ][ m_aRowCursors[ iRow ] ];
	m_oHud.SetStarPanel( iRow + 3 );

	ASoundManager::GetSingleton()->StopCue( m_iPoseCue );
	m_iPoseCue = ASoundManager::GetSingleton()->PlayCue( a_pPose->eSound );

	AAnimationBreakpointCallback* pBreakpoint = m_pStatue->GetAnimationEmitter( a_pPose->strAnim );

	if ( pBreakpoint )
		m_oPoseListener.Connect( pBreakpoint->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnPoseAnimEvent ), 0 );

	m_fComboTimer    = 0.0f;
	m_bReshuffleRows = TTRUE;
}

// $Barnyard: FUNCTION 00487820
void AStatuesMiniGameState::StandUp()
{
	if ( m_ePlayerState != PLAYERSTATE_ON_ALL_FOURS )
		return;

	m_ePlayerState = PLAYERSTATE_STANDING;

	m_pStatue->SetSkeletonUpdating( TTRUE, TTRUE );
	AShadowManager::GetSingleton()->SetShadowEnabled( m_pStatue->GetModelInstance(), TTRUE );

	m_pQuad->SetSkeletonUpdating( TFALSE, TTRUE );
	AShadowManager::GetSingleton()->SetShadowEnabled( m_pQuad->GetModelInstance(), TFALSE );

	m_pStatue->PlayAnimation( TPS8( quad_idle ) );
	m_pQuad->PlayAnimation( TPS8( idle_lp ) );
}

// $Barnyard: FUNCTION 004899b0
void AStatuesMiniGameState::BecomeQuad()
{
	if ( m_ePlayerState == PLAYERSTATE_ON_ALL_FOURS )
		return;

	m_pStatue->SetSkeletonUpdating( TFALSE, TTRUE );
	AShadowManager::GetSingleton()->SetShadowEnabled( m_pStatue->GetModelInstance(), TFALSE );

	m_pQuad->SetSkeletonUpdating( TTRUE, TTRUE );
	AShadowManager::GetSingleton()->SetShadowEnabled( m_pQuad->GetModelInstance(), TTRUE );

	m_ePlayerState = PLAYERSTATE_ON_ALL_FOURS;
	m_iCurrentPose = NO_POSE;
	m_oPoseListener.Disconnect();

	PlayEffectAtBoneName( m_pQuad, TPS8( emt_foot_01 ), m_iDustPuffEffect );
	PlayEffectAtBoneName( m_pQuad, TPS8( emt_foot_02 ), m_iDustPuffEffect );
}

// $Barnyard: FUNCTION 004864c0
TBOOL AStatuesMiniGameState::OnPoseFinished()
{
	m_oHud.SetInputVisible( TTRUE );
	m_ePlayerState   = PLAYERSTATE_STANDING;
	m_fStarFadeTimer = 1.0f;
	m_iCurrentPose   = NO_POSE;
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_BADING );

	if ( m_oPoseListener.IsLinked() )
		m_oPoseListener.Disconnect();

	return TTRUE;
}

// $Barnyard: FUNCTION 004863f0
void AStatuesMiniGameState::AdvanceRowPose( TINT a_iRow )
{
	m_aRowCursors[ a_iRow ]++;

	if ( m_aRowPoseOrder[ a_iRow ][ m_aRowCursors[ a_iRow ] ] == -1 )
		m_aRowCursors[ a_iRow ] = 0;

	m_oHud.SetRowPose( a_iRow, &m_aPoses[ m_aRowPoseOrder[ a_iRow ][ m_aRowCursors[ a_iRow ] ] ] );
}

// $Barnyard: FUNCTION 004866b0
TBOOL AStatuesMiniGameState::OthersUnhide()
{
	T2_FOREACH( m_vecOtherCharacters, it )
	{
		( *it )->PlayAnimation( TPS8( unhide ) );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00486450
void __stdcall AStatuesMiniGameState::PlayRandomTalk( AAnimatableObject* a_pObject )
{
	// NOTE: TRandom::GetFloat is inlined here
	a_pObject->PlayAnimation( ( ARandom::GetSingleton()->m_oRandom.GetFloat() <= 0.5f ) ? TPS8( talk2 ) : TPS8( talk1 ) );
}

// $Barnyard: FUNCTION 00486710
void __stdcall AStatuesMiniGameState::UpdateTalk( AAnimatableObject* a_pObject, TINT a_iCueIndex )
{
	if ( a_pObject->IsAnimPlaying( TPS8( talk1 ), 0.0f ) || a_pObject->IsAnimPlaying( TPS8( talk2 ), 0.0f ) )
	{
		if ( !ASoundManager::GetSingleton()->IsCueUsed( a_iCueIndex ) )
		{
			a_pObject->FinishAnimation( TPS8( talk1 ) );
			a_pObject->FinishAnimation( TPS8( talk2 ) );
		}
	}
	else if ( ASoundManager::GetSingleton()->IsCueUsed( a_iCueIndex ) )
	{
		PlayRandomTalk( a_pObject );
	}
}

// $Barnyard: FUNCTION 00487940
void __stdcall AStatuesMiniGameState::PlayEffectAtBone( AAnimatableObject* a_pObject, TINT a_iBone, TINT a_iEffect )
{
	TMatrix44 matBone;
	a_pObject->GetModelInstance()->GetBoneTransform( a_iBone, matBone );

	TVector4 vPosition = matBone.GetTranslation();

	// NOTE: TODO: AParticleEffectManager::FindEffect returns -1 while particles aren't reconstructed
	if ( a_iEffect == -1 )
		return;

	AParticleEffectManager::GetSingleton()->PlayEffect( a_iEffect, vPosition, TVector4::VEC_NEGY );
}

// $Barnyard: FUNCTION 004897c0
void __stdcall AStatuesMiniGameState::PlayEffectAtBoneName( AAnimatableObject* a_pObject, const TPString8& a_rcBone, TINT a_iEffect )
{
	PlayEffectAtBone( a_pObject, a_pObject->GetModelInstance()->GetBoneIndex( a_rcBone ), a_iEffect );
}

// $Barnyard: FUNCTION 00486790
TBOOL __stdcall AStatuesMiniGameState::OnPoseAnimEvent( AStatuesMiniGameState* a_pThis, void* a_pOwner, void* a_pData )
{
	return a_pThis->OnPoseFinished();
}

// $Barnyard: FUNCTION 004867a0
TBOOL __stdcall AStatuesMiniGameState::OnWinnerStart( AStatuesMiniGameState* a_pThis, void* a_pOwner, void* a_pData )
{
	T2_FOREACH( a_pThis->m_vecOtherCharacters, it )
	{
		( *it )->PlayAnimation( TPS8( success ) );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00487a00
TBOOL __stdcall AStatuesMiniGameState::OnWatcherNothing( AStatuesMiniGameState* a_pThis, void* a_pOwner, void* a_pData )
{
	return a_pThis->OthersUnhide();
}

// $Barnyard: FUNCTION 00487a10
TBOOL __stdcall AStatuesMiniGameState::OnWatcherStopLook( AStatuesMiniGameState* a_pThis, void* a_pOwner, void* a_pData )
{
	if ( a_pThis->m_eWatcherState == WATCHERSTATE_LOOKING )
	{
		a_pThis->m_eWatcherState = WATCHERSTATE_NOTHING_TO_SEE;

		if ( a_pThis->m_bIsMrsBeady )
		{
			a_pThis->m_iMrsBeadyCue2 = ASoundManager::GetSingleton()->PlayCue( soundbank::MRS_B_MR_B_N01 );
			PlayRandomTalk( a_pThis->m_pWatcher );
		}
	}

	return TTRUE;
}
