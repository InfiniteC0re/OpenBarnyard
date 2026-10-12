#include "pch.h"
#include "AScriptedSequenceActivity.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "Cameras/ACameraManager.h"
#include "FX/AGameTimeFXManager.h"
#include "Helpers/AGateManager.h"
#include "Helpers/AGameLoader.h"
#include "GameInterface/ASimState.h"
#include "MiniGame/ABarnExploreMinigameState.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bcde0, CreateObject 005bce00, CreateObjectInPlace 005bce80
TDEFINE_CLASS( AScriptedSequenceActivity );

// 0x007791f0
const AScriptedSequenceActivity::SequenceInfo AScriptedSequenceActivity::ms_aSequences[ NUM_SEQUENCES ] = {
	{ L"SS01_start", "SS01_start", "SS01_start", "SS01_cameraB", 1, 45, 0, 1, 0, -1, 0, 0, 2, 1 },
	{ L"SS02_rac", "SS02_rac", "SS02_rac", "SS02_cameraB", 1, 46, 0, 2, 0, -1, 0, 0, 2, 1 },
	{ L"SS03_story3", "SS03_story3", "SS03", "SS03_cameraB", 1, 47, 0, 2, 0, -1, 0, 0, 2, 1 },
	{ L"SS04_party", "SS04_party", "SS04", "SS04_cameraB", 1, 48, 0, 4, 0, -1, 0, 0, 2, 1 },
	{ L"SS05_story", "SS05_story", "SS05", "SS05_cameraB", 1, 45, 0, 4, 2, -1, 0, 0, 2, 1 },
	{ L"SS06_story", "SS06_story", "SS06", "SS06_cameraB", 1, 50, 0, 2, 0, -1, 0, 0, 2, 1 },
	{ L"SS07a_story", "SS07a_stroy", "SS07a", "SS07_cameraB", 1, 51, 0, 4, 0, -1, 0, 0, 2, 1 },
	{ L"SS07b_story", "SS07b_stroy", "SS07b", "SS07_cameraB", 1, 52, 0, 4, 0, -1, 0, 0, 3, 0 },
	{ L"SS08_story", "SS08_story", "SS08", "SS08_cameraB", 1, 53, 0, 0, 0, -1, 0, 0, 2, 1 },
	{ L"SS09_story", "SS09_story", "SS09", "SS09_cameraB", 1, 54, 0, 3, 0, -1, 0, 0, 2, 1 },
	{ L"SS10_end", "SS10_end", "SS10", "SS10_cameraB", 1, 55, 0, 4, 0, -1, 0, 0, 2, 0 },
	{ L"SS11_prologue", "SS11_prolog", "SS11", "SS11_cameraB", 1, 56, 0, 2, 0, -1, 0, 0, 2, 0 },
	{ L"SSCT_Intro", "SSCT_Intro", "SSCT_Intro", "SSCT_CameraB", 1, 28, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSDART_Intro", "SSDART_Intro", "SSDART_Intro", "SSDART_CameraB", 1, 43, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSPOOL_Intro", "SSPOOL_Intro", "SSPOOL_Intro", "SSPOOL_CameraB", 1, 31, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSSS_Intro", "SSSS_Intro", "SSSS_Intro", "SSSS_CameraB", 1, 0, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSTAP_Intro", "SSTAP_Intro", "SSTAP_Intro", "SSTAP_CameraB", 1, 33, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSWAR_Intro", "SSWAR_Intro", "SSWAR_Intro", "SSWAR_CameraB", 1, 32, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSFTK_FollowTheKids", "SSFTK_FollowTheKids", "SSFTK_FollowTheKids", "SSFTK_CameraB", 1, 34, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSCC_ChasingChicks", "SSCC_ChasingChicks", "SSCC_ChasingChicks", "SSCC_CameraB", 1, 37, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSBR_Main", "SSBR_Main", "SSBR_Main", "SSBR_CameraB", 1, 0, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSCCDef_ChickCoopDefender", "SSCCDef_ChickCoopDefender", "SSCCDef_ChickCoopDefender", "SSCCDef_CameraB", 1, 0, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSEF_Main", "SSEF_Main", "SSEF_Main", "SSEF_CameraB", 1, 0, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSMJ_Main", "SSMJ_Main", "SSMJ_Main", "SSMJ_CameraB", 1, 1, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSVEG_Intro", "SSVEG_Intro", "SSVEG_Intro", "SSVEG_VicAndDef_CameraB", 1, 29, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSNBO_main", "SSNBO_main", "SSNBO_main", "SSNBO_CameraB", 1, 0, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSNBC_main", "SSNBC_main", "SSNBC_main", "SSNBC_CameraB", 1, 0, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSEND_Champ", "SSEND_Champ", "SSEND_Champ", "SSEND_CameraB", 1, 2, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSEND_Detail", "SSEND_Detail", "SSEND_Detail", "SSEND_CameraB", 1, 2, 1, -1, 0, -1, 0, 0, 2, 0 },
#ifdef BARNYARD_LATEST
	{ L"SSCUO_Main", "SSCUOc_Main", "SSCUOc_main", "SSCUOc_CameraB", 1, 0, 0, 3, 0, -1, 0, 0, 2, 1 },
	{ L"SSBFF_Main", "SSBFF_Main", "SSBFF_main", "SSBFF_CameraB", 1, 0, 1, -1, 0, -1, 0, 0, 2, 0 },
	{ L"SSCUO_MainMerged", "SS06_storyMerged", "SS06_StoryMerged", "SS06_CameraBMerged", 1, AGameLoader::Terrain_ss06merged, 0, 3, 0, -1, 0, 0, 2, 1 },
	{ L"SSUL_Main", "SSUL_Main", "SSUL_main", "SSUL_CameraB", 1, 0, 1, -1, 0, -1, 0, 0, 2, 0 },
#endif // BARNYARD_LATEST
};

// NOTE: inlined into CreateObject 005bce00 / CreateObjectInPlace 005bce80
AScriptedSequenceActivity::AScriptedSequenceActivity()
    : m_bPushState( TFALSE )
    , m_iSequence( -1 )
    , m_eReturn( RETURN_DEFAULT )
    , m_pState( TNULL )
    , m_iGate( -1 )
    , m_bGateState( TFALSE )
{
}

// $Barnyard: FUNCTION 005bcdf0
// $Barnyard: FUNCTION 005bcf00
AScriptedSequenceActivity::~AScriptedSequenceActivity()
{
}

// $Barnyard: FUNCTION 005bd190
TBOOL AScriptedSequenceActivity::Create( const PBProperties* a_pProperties )
{
	// NOTE: retail doesn't check if the property exists
	const TCHAR* szName = a_pProperties->GetProperty( "name" )->GetString();

	for ( TINT i = 0; i < NUM_SEQUENCES; i++ )
	{
		if ( ms_aSequences[ i ].szName && TStringManager::String8CompareNoCase( szName, ms_aSequences[ i ].szName ) == 0 )
			m_iSequence = i;
	}

	m_eReturn = RETURN_DEFAULT;

	const PBPropertyValue* pReturn = a_pProperties->GetProperty( "return" );

	if ( pReturn )
	{
		const TCHAR* szReturn = pReturn->GetString();

		if ( TStringManager::String8Compare( szReturn, "barn" ) == 0 )
			m_eReturn = RETURN_BARN;
		else if ( TStringManager::String8Compare( szReturn, "sim" ) == 0 )
			m_eReturn = RETURN_SIM;
	}

	m_bPushState = TFALSE;
	a_pProperties->GetOptionalPropertyValue( m_bPushState, TPS8D( "pushstate" ) );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bcf20
AInteractionActivity::RESULT AScriptedSequenceActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( !m_bPushState )
	{
		// Plays the story scripted sequence through AGameLoader, which returns to the barn or the sim afterwards
		AGameLoader::SetTransitionToScriptedSequence( m_iSequence );

		TBOOL bBarn = m_eReturn == RETURN_BARN;

		if ( m_eReturn == RETURN_DEFAULT )
			bBarn = AGameLoader::IsBarnExploreMiniGame();

		AGameLoader::SetPrevGameStateClass( bBarn ? &TGetClass( ABarnExploreMinigameState ) : &TGetClass( ASimState ) );
		return RESULT_DONE;
	}

	// Waits for the sequence to be over
	if ( m_pState )
	{
		if ( AGameStateController::GetSingleton()->GetCurrentState() == m_pState )
			return RESULT_RUNNING;

		// The gate gets its state back
		if ( AGateManager::IsSingletonCreated() && m_iGate != -1 )
		{
			if ( m_bGateState )
			{
				AGateManager::GetSingleton()->OpenGatesOfType( m_iGate, TFALSE );
				m_pState = TNULL;
				return RESULT_DONE;
			}

			AGateManager::GetSingleton()->CloseGatesOfType( m_iGate );
		}

		m_pState = TNULL;
		return RESULT_DONE;
	}

	const SequenceInfo& rcSequence = ms_aSequences[ m_iSequence ];
	TBOOL               bForcedFX  = TFALSE;

	if ( AGameTimeFXManager::IsSingletonCreated() && rcSequence.iTimeFX != -1 )
	{
		AGameTimeFXManager* pGameTimeFXManager = AGameTimeFXManager::GetSingleton();
		pGameTimeFXManager->SetForcedFX( pGameTimeFXManager->GetWeatherSettings( rcSequence.iTimeFXGroup, rcSequence.iTimeFX ) );
		bForcedFX = TTRUE;
	}

	m_iGate = -1;

	if ( AGateManager::IsSingletonCreated() && rcSequence.iGate != -1 )
	{
		AGateManager::Gate* pGate = AGateManager::GetSingleton()->FindGate( rcSequence.iGate, 0 );

		if ( rcSequence.bRestoreGate )
		{
			m_iGate      = rcSequence.iGate;
			m_bGateState = pGate->bOpen;
		}

		if ( rcSequence.bGateState )
			AGateManager::GetSingleton()->OpenGatesOfType( rcSequence.iGate, TFALSE );
		else
			AGateManager::GetSingleton()->CloseGatesOfType( rcSequence.iGate );
	}

	ASSeqState* pState = new ASSeqState( rcSequence.szName, rcSequence.szScript, rcSequence.szCamera, TVector4::VEC_ZERO, rcSequence.iSyncType, TTRUE, TTRUE, TFALSE, TFALSE, bForcedFX, rcSequence.bShowChapterTitle, TTRUE, TFALSE );
	m_pState           = pState;

	if ( !m_pState )
		return RESULT_FINISH_SEQUENCE;

	pState->SetCanSkip( rcSequence.bCanSkip );

	AGameStateController::GetSingleton()->PushState( m_pState );
	AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_3 );

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( ACamera::sm_fCurrentFOV );

	return RESULT_RUNNING;
}
