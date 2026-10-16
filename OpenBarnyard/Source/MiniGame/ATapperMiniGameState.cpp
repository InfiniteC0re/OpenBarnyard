#include "pch.h"
#include <Render/TCameraObject.h>
#include "ATapperMiniGameState.h"
#include "Helpers/APlayerProgress.h"
#include "AMiniGameManager.h"
#include "ARandom.h"
#include "Helpers/AWeightedRandom.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/AModel.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AGameTimeFXManager.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AMiniGameInstructionState.h"
#include "GameInterface/ASSeqState.h"
#include "GUI/AGUI2ModelRendererScreen.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Input/AInputHandler.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocaleManager.h"
#include "Locale/ALocale.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Render/ASkyDome.h"
#include "Render/ASunFlareEffect.h"
#include "Sound/AMultiplexStream.h"
#include "Sound/APositionalSoundSet.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/am_f.h"
#include "SoundBank/am_m.h"
#include "SoundBank/tappa.h"
#include "Tasks/ARootTask.h"
#include "Terrain/ATerrainInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ACharacterMovementComponent.h"
#include "Units/AUnitFacialComponent.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitProxy.h"
#include "Units/AUnitTransform.h"
#include "Units/ASSeqController.h"

#include <File/TTRB.h>
#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Render/TRenderInterface.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TString8.h>
#include <Toshi/TStringManager.h>
#include <Toshi/TSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( AMiniGameInstructionState );
TPSTRING8_DECLARE( ATapperMiniGameState );
TPSTRING8_DECLARE( angry1 );
TPSTRING8_DECLARE( angry2 );
TPSTRING8_DECLARE( angry_lp );
TPSTRING8_DECLARE( barnbuck );
TPSTRING8_DECLARE( carrysmall );
TPSTRING8_DECLARE( cb_llocator );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( close );
TPSTRING8_DECLARE( closed_idle );
TPSTRING8_DECLARE( dance );
TPSTRING8_DECLARE( dance2 );
TPSTRING8_DECLARE( dance3 );
TPSTRING8_DECLARE( default );
TPSTRING8_DECLARE( dodge_end );
TPSTRING8_DECLARE( dodge_mid );
TPSTRING8_DECLARE( dodge_start );
TPSTRING8_DECLARE( drop_glass );
TPSTRING8_DECLARE( fillup );
TPSTRING8_DECLARE( getofftable );
TPSTRING8_DECLARE( getontable );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( idle_lp2 );
TPSTRING8_DECLARE( idle_lp3 );
TPSTRING8_DECLARE( idle_lp_walk2 );
TPSTRING8_DECLARE( jump );
TPSTRING8_DECLARE( jump_left );
TPSTRING8_DECLARE( jump_right );
TPSTRING8_DECLARE( money );
TPSTRING8_DECLARE( open );
TPSTRING8_DECLARE( opened_idle );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( slideback );
TPSTRING8_DECLARE( sulk );
TPSTRING8_DECLARE( tap_curtain );
TPSTRING8_DECLARE( tap_drink );
TPSTRING8_DECLARE( tap_drink_empty );
TPSTRING8_DECLARE( tap_drinker );
TPSTRING8_DECLARE( tap_player_fe );
TPSTRING8_DECLARE( tap_player_m );
TPSTRING8_DECLARE( tap_root );
TPSTRING8_DECLARE( tap_wildmike );
TPSTRING8_DECLARE( throw );
TPSTRING8_DECLARE( throwback );
TPSTRING8_DECLARE( tp_table3_add );
TPSTRING8_DECLARE( tp_table3_sub );
TPSTRING8_DECLARE( tp_table4_add );
TPSTRING8_DECLARE( tp_table4_sub );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( walk_lp2 );
TPSTRING8_DECLARE( win );

// NOTE: GetClass 004de870, CreateObject 004e8bb0, CreateObjectInPlace 004e8c10 (the constructor is inlined in both)
TDEFINE_CLASS( ATapperMiniGameState );

// 00773bb8
TVector4 ATapperMiniGameState::s_vCameraPosition = TVector4( 29.0f, -14.0f, 33.0f, 1.0f );

// 00773bc8
TVector4 ATapperMiniGameState::s_vCameraDirection = TVector4( -0.768f, 0.63f, 0.0f, 1.0f );

// 00773bd8
TVector4 ATapperMiniGameState::s_vPlayerStart = TVector4( 26.0f, 0.63f, -83.0f, 1.0f );

// 00773b70
const TPString8* ATapperMiniGameState::s_apIdleAnims[ 4 ] = {
	&TPS8( idle_lp3 ),
	&TPS8( idle_lp2 ),
	&TPS8( angry1 ),
	&TPS8( angry2 ),
};

void ATapperMiniGameState::DisconnectAnimEventListener()
{
	if ( m_oAnimEventListener.pNext != &m_oAnimEventListener )
	{
		m_oAnimEventListener.pPrev->pNext = m_oAnimEventListener.pNext;
		m_oAnimEventListener.pNext->pPrev = m_oAnimEventListener.pPrev;
		m_oAnimEventListener.pNext        = &m_oAnimEventListener;
		m_oAnimEventListener.pPrev        = &m_oAnimEventListener;
	}

	m_oAnimEventListener.pCaller   = TNULL;
	m_oAnimEventListener.pCallback = TNULL;
}

// $Barnyard: FUNCTION 004df1f0
void ATapperMiniGameState::ShowWildMikeIcon()
{
	m_oWildMikeIcon.SetShown( TTRUE );
}

// $Barnyard: FUNCTION 004df210
void ATapperMiniGameState::HideWildMikeIcon()
{
	m_oWildMikeIcon.SetShown( TFALSE );
}

// $Barnyard: FUNCTION 004de950
void ATapperMiniGameState::SelectGlassPool()
{
	// NOTE: there is only one glass pool, this always ends up with 0 when it has free glasses
	m_iGlassPool = GlassPools::NUM_POOLS;

	do
	{
		m_iGlassPool--;

		if ( m_iGlassPool < 0 )
			break;

	} while ( m_aGlassPools[ m_iGlassPool ].iNumGlasses == 0 );
}

// $Barnyard: FUNCTION 004de980
TVector4& ATapperMiniGameState::GetPlayerGlassStart( TVector4& a_rOutPos, TVector4& a_rOutDir )
{
	a_rOutDir.Substract( GetPlayerBeerEnd( m_iBar ), GetPlayerBeerStart( m_iBar ) );
	a_rOutDir.Normalise();

	TVector4 vDir;
	vDir.Substract( GetPlayerBeerEnd( m_iBar ), GetPlayerBeerStart( m_iBar ) );
	vDir.Normalise();

	TVector4 vOffset;
	vOffset.Multiply( vDir, 0.3f );

	a_rOutPos.Substract( GetPlayerBeerStart( m_iBar ), vOffset );
	return a_rOutPos;
}

// $Barnyard: FUNCTION 004deaf0
void ATapperMiniGameState::GetDrinkerGlassPos( Drinker* a_pDrinker, TVector4& a_rOutPos, TVector4& a_rOutDir )
{
	a_rOutDir.Substract( GetEnemyBeerEnd( a_pDrinker->iBar ), GetEnemyBeerStart( a_pDrinker->iBar ) );
	a_rOutDir.Normalise();

	TVector4 vStart = GetEnemyBeerStart( a_pDrinker->iBar );
	TVector4 vEnd   = GetEnemyBeerEnd( a_pDrinker->iBar );
	TVector4 vPos   = *a_pDrinker->pUnit->GetSteer()->GetPosition();

	TFLOAT fT;
	TMath::DistancePointToLineSq( vPos, vStart, vEnd, fT );
	fT += 0.01f;

	TVector4 vLine;
	vLine.Substract( vEnd, vStart );

	TVector4 vOffset;
	vOffset.Multiply( vLine, fT );

	a_rOutPos.Add( vOffset, vStart );
}

// $Barnyard: FUNCTION 004dece0
void ATapperMiniGameState::AddScoreFloat( const TVector4& a_rcPosition, TINT a_iScore )
{
	m_iFloatText = ( m_iFloatText + 1 ) % MAX_FLOAT_TEXTS;

	if ( a_iScore >= 0 )
	{
		TStringManager::String16Format( m_awszFloatTexts[ m_iFloatText ], 16, L"%d", a_iScore );
		m_oFloatHud.AddTimedFloat( m_awszFloatTexts[ m_iFloatText ], 0x7fffff00, a_rcPosition, 3.0f, -0.6f, 4.0f );
	}
	else
	{
		TStringManager::String16Format( m_awszFloatTexts[ m_iFloatText ], 16, L"-%d", -a_iScore );
		m_oFloatHud.AddTimedFloat( m_awszFloatTexts[ m_iFloatText ], 0xccff0000, a_rcPosition, 3.0f, -0.3f, 3.0f );
	}
}

// $Barnyard: FUNCTION 004dedd0
TVector4& ATapperMiniGameState::GetPlayerTargetPos( TVector4& a_rOutPos, TINT a_iBar )
{
	if ( m_fRunDirection <= 0.0f )
		a_rOutPos = GetBarTap( a_iBar );
	else
		a_rOutPos = GetBarEnd( a_iBar );

	return a_rOutPos;
}

// $Barnyard: FUNCTION 004dee50
TFLOAT ATapperMiniGameState::GetDistanceSqToTarget( TINT a_iBar )
{
	TVector4 vTarget;
	GetPlayerTargetPos( vTarget, a_iBar );

	return TVector4::DistanceSq( vTarget, m_vPlayerPos );
}

// $Barnyard: FUNCTION 004dee80
void ATapperMiniGameState::HideTips()
{
	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		if ( m_aTips[ i ].eState != TIPSTATE_NONE )
		{
			m_aTips[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );
			m_aTips[ i ].eState = TIPSTATE_NONE;
		}
	}
}

// $Barnyard: FUNCTION 004deec0
void ATapperMiniGameState::AddLives( PlayerData& a_rPlayer, TINT a_iCount )
{
	a_rPlayer.iLives += a_iCount;

	if ( a_rPlayer.iLives < 0 )
		a_rPlayer.iLives = 0;
	else if ( a_rPlayer.iLives > m_aiParams[ INTPARAM_MAX_LIVES ] )
		a_rPlayer.iLives = m_aiParams[ INTPARAM_MAX_LIVES ];

	m_oLivesHud.AddLives( a_iCount );
}

// $Barnyard: FUNCTION 004def10
void ATapperMiniGameState::AddBonusLife( PlayerData& a_rPlayer )
{
	a_rPlayer.iLives += 1;

	if ( a_rPlayer.iLives < 0 )
		a_rPlayer.iLives = 0;
	else if ( a_rPlayer.iLives > m_aiParams[ INTPARAM_MAX_LIVES ] )
		a_rPlayer.iLives = m_aiParams[ INTPARAM_MAX_LIVES ];

	m_oLivesHud.AddLives( 1 );
}

// $Barnyard: FUNCTION 004def60
TBOOL ATapperMiniGameState::IsDrinkerBehindGlassLine( Drinker* a_pDrinker, TFLOAT a_fDistance )
{
	TVector4 vDir = GetPlayerBeerStart( a_pDrinker->iBar ) - GetPlayerBeerEnd( a_pDrinker->iBar );
	vDir.y        = 0.0f;
	vDir.Normalise();

	TVector4 vOffset;
	vOffset.Multiply( vDir, a_fDistance );

	TVector4 vLinePos = GetPlayerBeerEnd( a_pDrinker->iBar ) + vOffset;
	vDir.AsVector3().RotateY( TMath::PI * 0.5f );

	TVector4 vToDrinker = *a_pDrinker->pUnit->GetSteer()->GetPosition() - vLinePos;
	vToDrinker.y        = 0.0f;
	vToDrinker.Normalise();

	TVector4 vCross;
	vCross.CrossProduct( vToDrinker, vDir );

	return vCross.y >= 0.0f;
}

// $Barnyard: FUNCTION 004df120
void ATapperMiniGameState::OnStartHudFinished()
{
	APositionalSoundSet::GetSingleton()->SetPaused( TFALSE );

	m_oPortrait.Show();
	m_oLivesHud.SetVisible( TTRUE );
	m_oPickUpHudElement.SetForceHide( TFALSE );

	ALocale::FormatTags( m_wszLevelText, TARRAYSIZE( m_wszLevelText ), LOCALE_LEVEL, PPropertyValue( m_iLevel + 1 ) );
	m_oLevelText.SetText( m_wszLevelText, 0x55000000 );
	m_oFloatHud.Show();
}

// $Barnyard: FUNCTION 004df230
void ATapperMiniGameState::OnPreStart()
{
	m_pPlayer->SetVisible( TFALSE );
}

// $Barnyard: FUNCTION 004df250
void ATapperMiniGameState::UpdateAlways( TFLOAT a_fDeltaTime )
{
	m_oLevelText.Update( a_fDeltaTime );

	if ( m_oBonusLifePopup.IsVisible() && m_oBonusLifePopup.m_fTargetScale != 0.0f && m_fBonusLifeTextTimer < 0.0f )
	{
		m_oBonusLifePopup.SetShownInstantly( TTRUE );
		m_fBonusLifeTextTimer = 2.0f;
	}
}

// $Barnyard: FUNCTION 004df2c0
void ATapperMiniGameState::OnObjectCollected( AAnimatableObject* a_pObject )
{
	TINT iTip = 0;

	while ( m_aTips[ iTip ].pObject != a_pObject )
	{
		if ( ++iTip >= NUM_BARS )
			return;
	}

	if ( iTip != -1 && m_aTips[ iTip ].eState != TIPSTATE_NONE )
	{
		m_aTips[ iTip ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );
		m_aTips[ iTip ].eState = TIPSTATE_NONE;
	}
}

// $Barnyard: FUNCTION 004df330
void ATapperMiniGameState::Levels::Clear()
{
	for ( TINT i = 0; i < iNumLevels; i++ )
		aLevels[ i ] = Level();

	iNumLevels = 0;
}

// $Barnyard: FUNCTION 004df390
void ATapperMiniGameState::CommandQueue::Push( TBOOL a_bReset, TBOOL& a_rbHandled, AInputCommand a_eCommand, TINT a_iEventType )
{
	if ( a_bReset )
	{
		a_rbHandled = TFALSE;
	}
	else if ( a_rbHandled )
	{
		a_rbHandled = TTRUE;
	}
	else
	{
		TINT iPrev = iRead - 1;

		if ( iRead - 1 < 0 )
			iPrev = MAX_COMMANDS - 1;

		// NOTE: this can never be true, the queue is meant to be reset when it's full
		if ( iRead == iPrev )
		{
			iWrite = 0;
			iRead  = 0;
		}

		aCommands[ iWrite ].eCommand   = a_eCommand;
		aCommands[ iWrite ].iEventType = a_iEventType;

		if ( ++iWrite > MAX_COMMANDS - 1 )
			iWrite = 0;
	}
}

// $Barnyard: FUNCTION 004df410
void ATapperMiniGameState::CommandQueue::Process( ATapperCommandHandler* a_pHandler )
{
	if ( iRead == iWrite )
		return;

	AInputCommand eCommand   = aCommands[ iRead ].eCommand;
	TINT          iEventType = aCommands[ iRead ].iEventType;

	while ( iRead != iWrite )
	{
		TBOOL bStop = TTRUE;
		a_pHandler->HandleCommand( eCommand, iEventType, bStop );

		if ( bStop )
			break;

		if ( iRead != iWrite )
		{
			if ( iEventType != TInputInterface::EVENT_TYPE_GONE_UP && iEventType != TInputInterface::EVENT_TYPE_REPEAT )
				a_pHandler->HandleCommand( eCommand, TInputInterface::EVENT_TYPE_GONE_UP, bStop );

			if ( ++iRead > MAX_COMMANDS - 1 )
				iRead = 0;

			if ( bStop )
				break;
		}
	}
}

// $Barnyard: FUNCTION 004df580
void ATapperMiniGameState::CreateTips()
{
	AAnimatableObjectType* pType = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( barnbuck ) );

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		m_aTips[ i ].pObject = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pType, TNULL, 0 );
		m_aTips[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );
		m_aTips[ i ].pObject->SetAnimControllersDisabled( TTRUE );
		m_aTips[ i ].eState = TIPSTATE_NONE;
	}
}

// $Barnyard: FUNCTION 004df5e0
void ATapperMiniGameState::AddCurtainToModelRenderer()
{
	TMatrix44 matCamera = TMatrix44::IDENTITY;
	matCamera.SetTranslation( TVector4( 0.0f, 0.0f, -3.0f, 1.0f ) );

	AGUI2ModelRendererScreen::GetSingleton()->AddObject( m_pCurtain, TMatrix44::IDENTITY, matCamera, TMatrix44::IDENTITY, FOV * 0.017453292f );
	AGUI2ModelRendererScreen::GetSingleton()->SetEnabled( TFALSE );
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );
}

// $Barnyard: FUNCTION 004df690
void ATapperMiniGameState::DetachCarriedGlass()
{
	if ( m_pPlayer->GetAnimatableObject()->FindAttachedObject( TPS8( cb_rlocator ) ) == m_pCarriedGlass )
	{
		AAnimatableObjectManager::GetSingleton()->DetachObject( m_pCarriedGlass, m_pPlayer->GetAnimatableObject() );
		m_pCarriedGlass->SetSkeletonUpdating( TFALSE, TTRUE );
		m_pCarriedGlass->SetVisible( TFALSE );
	}
}

// $Barnyard: FUNCTION 004df700
void ATapperMiniGameState::CreateGlasses()
{
	TString8 strUnused( "test" );

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	TQuaternion        quatRotation;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "playerbeer_start%d", i + 1 ), m_avPlayerBeerStart[ i ], quatRotation );
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "playerbeer_end%d", i + 1 ), m_avPlayerBeerEnd[ i ], quatRotation );
	}

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "enemybeer_start%d", i + 1 ), m_avEnemyBeerStart[ i ], quatRotation );
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "enemybeer_end%d", i + 1 ), m_avEnemyBeerEnd[ i ], quatRotation );
	}

	for ( TINT i = 0; i < NUM_SPLASHES; i++ )
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "splash%02d", i + 1 ), m_avSplashes[ i ], quatRotation );

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pFullType       = pAnimObjManager->FindType( TPS8( tap_drink ) );
	AAnimatableObjectType*    pEmptyType      = pAnimObjManager->FindType( TPS8( tap_drink_empty ) );

	for ( TINT i = 0; i < MAX_GLASSES; i++ )
	{
		Glass& rGlass = m_aGlasses[ i ];

		rGlass.pFull     = pAnimObjManager->CreateAnimatableObject( pFullType, TNULL, 0 );
		rGlass.vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		rGlass.iCue      = -1;
		rGlass.pFull->SetSkeletonUpdating( TFALSE, TTRUE );
		rGlass.pFull->SetVisible( TFALSE );
		rGlass.pFull->SetAnimControllersDisabled( TTRUE );

		rGlass.pEmpty = pAnimObjManager->CreateAnimatableObject( pEmptyType, TNULL, 0 );
		rGlass.pEmpty->SetSkeletonUpdating( TFALSE, TTRUE );
		rGlass.pEmpty->SetVisible( TFALSE );
		rGlass.pEmpty->SetAnimControllersDisabled( TTRUE );

		m_aGlassPools[ 0 ].llFree.PushFront( &rGlass );
	}

	m_pCarriedGlass = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( tap_drink_empty ) ), TNULL, 0 );
	m_pCarriedGlass->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pCarriedGlass->SetVisible( TFALSE );
	m_pCarriedGlass->SetAnimControllersDisabled( TTRUE );
}
// $Barnyard: FUNCTION 004dfa60
void ATapperMiniGameState::CreateDrinkers()
{
	TString8 strUnused( "test" );

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	TQuaternion        quatRotation;

	for ( TINT i = 0; i < NUM_BARS; i++ )
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "enemy_start%d", i + 1 ), m_avEnemyStart[ i ], quatRotation );

	for ( TINT i = 0; i < NUM_BARS; i++ )
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "enemy_end%d", i + 1 ), m_avEnemyEnd[ i ], quatRotation );

	for ( TINT iLane = 0; iLane < NUM_BARS; iLane++ )
	{
		for ( TINT i = 0; i < DRINKERS_PER_BAR; i++ )
		{
			Drinker& rDrinker = m_aDrinkers[ iLane * DRINKERS_PER_BAR + i ];

			AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, iLane, -1, TTRUE );
			AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( TPS8( tap_drinker ), &oTransform ) );

			pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
			pUnit->SetVisible( TFALSE );
			pUnit->SetHashed( TFALSE );
			pUnit->SetNoQuery( TTRUE );
			pUnit->GetModelInstance()->GetSceneObject()->DisableDistanceCulling();

			// NOTE: every drinker faces the direction of the first bar
			TVector4 vDirection = m_avEnemyEnd[ 0 ] - m_avEnemyStart[ 0 ];
			vDirection.y        = 0.0f;
			vDirection.Normalise();
			pUnit->SetSteerDirection( vDirection );

			rDrinker.iLane  = iLane;
			rDrinker.pUnit  = pUnit;
			rDrinker.fDance = 0.0f;
			rDrinker.fTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( GetLevel().fDrinkerDelayMin, GetLevel().fDrinkerDelayMax );

			m_aBars[ iLane ].llWaiting.PushBack( &rDrinker );
		}
	}
}

// $Barnyard: FUNCTION 004dfd70
void ATapperMiniGameState::DestroyGlasses()
{
	m_aGlassPools[ 0 ].llFree.Clear();
	m_aGlassPools[ 0 ].llUsed.Clear();

	for ( TINT i = MAX_GLASSES - 1; i >= 0; i-- )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_aGlasses[ i ].pFull );
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_aGlasses[ i ].pEmpty );
	}

	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pCarriedGlass );
}

// $Barnyard: FUNCTION 004dfdd0
void ATapperMiniGameState::DestroyDrinkers()
{
	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		m_aBars[ i ].llActive.Clear();
		m_aBars[ i ].llWaiting.Clear();
	}

	for ( TINT i = MAX_DRINKERS - 1; i >= 0; i-- )
	{
		m_aDrinkers[ i ].oSkin.Reset();
		AUnitManager::GetSingleton()->DestroyUnit( m_aDrinkers[ i ].pUnit );
	}
}

// $Barnyard: FUNCTION 004dfe20
void ATapperMiniGameState::OnActivate()
{
	TMemory::TestMemIntegrity( TNULL );
	AInputHandler::GetSingleton()->SetRumblePaused( TFALSE );
	AMultiplexStream::GetSingleton()->Pause( TFALSE );

	AMiniGameState::OnActivate();

	if ( m_ePlayerState == PLAYERSTATE_FILLING )
		ASoundManager::GetSingleton()->SetCueVolume( m_iFillCue, 1.0f );

	m_oMessagePopup.ShowIfHasText();
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );
	m_oLevelText.Show();
}

// $Barnyard: FUNCTION 004dfe90
void ATapperMiniGameState::OnDeactivate()
{
	AInputHandler::GetSingleton()->SetRumblePaused( TTRUE );

	if ( m_ePlayerState == PLAYERSTATE_FILLING )
		ASoundManager::GetSingleton()->SetCueVolume( m_iFillCue, 0.0f );

	m_oMessagePopup.Hide();
	TMemory::TestMemIntegrity( TNULL );
	AMultiplexStream::GetSingleton()->Pause( TTRUE );

	AMiniGameState::OnDeactivate();

	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );
	m_oLevelText.Hide();
}

// $Barnyard: FUNCTION 004dff00
TBOOL ATapperMiniGameState::IsPlayerAtTarget()
{
	TVector4 vTarget;
	GetPlayerTargetPos( vTarget, m_iBar );

	return TVector4::DistanceXZ( vTarget, m_vPlayerPos ) <= 0.3f;
}

// $Barnyard: FUNCTION 004dff90
void ATapperMiniGameState::StopDrinker( Drinker* a_pDrinker )
{
	AAnimatedUnit* pUnit = a_pDrinker->pUnit;

	if ( a_pDrinker->eState == DRINKERSTATE_STOPPED )
		return;

	if ( IsDrinkerBehindGlassLine( a_pDrinker, 0.0f ) )
		pUnit->GetAnimatableObject()->PlayAnimation( GetRandomIdleAnim() );
	else
		pUnit->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp_walk2 ) );

	AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pUnit->GetSteer() );
	pSteer->Command_CancelCurrentCommand();
	pSteer->SetMode( AInteriorSteer::MODE_STILL );

	a_pDrinker->eState = DRINKERSTATE_STOPPED;
	a_pDrinker->fTimer = -ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( GetLevel().fStopTimeMin, GetLevel().fStopTimeMax );
}

// $Barnyard: FUNCTION 004dff50
const TPString8& ATapperMiniGameState::GetRandomIdleAnim()
{
	if ( ARandom::GetSingleton()->m_oRandom.GetInt( 100 ) >= 98 )
		return *s_apIdleAnims[ 1 + ARandom::GetSingleton()->m_oRandom.GetInt( 1 ) ];

	return TPS8( idle_lp3 );
}

// $Barnyard: FUNCTION 004e0060
TBOOL ATapperMiniGameState::IsMusicIntense()
{
	if ( m_fLevelTime <= m_fNextWildMikeTime )
		return TFALSE;

	return m_iWildMikeScore <= GetCurrentPlayer().iScore || !m_oWildMikeIcon.IsHiding();
}

// $Barnyard: FUNCTION 004e00b0
void ATapperMiniGameState::UpdateMusic( TFLOAT a_fDeltaTime )
{
	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( pStream->GetState() == AMultiplexStream::STATE_PLAYING )
	{
		TFLOAT fVolume = pStream->GetVolume( 2 );

		if ( IsMusicIntense() || ( m_eWildMikeState != WILDMIKESTATE_NONE && m_eWildMikeState != WILDMIKESTATE_LEAVING ) )
		{
			if ( fVolume >= 1.0f )
				return;

			fVolume += a_fDeltaTime;
		}
		else
		{
			if ( fVolume <= 0.0f )
				return;

			fVolume -= a_fDeltaTime;
		}

		if ( fVolume < 0.0f )
			fVolume = 0.0f;

		pStream->SetVolume( fVolume, 2, TTRUE );
		pStream->SetVolume( fVolume, 3, TTRUE );
		pStream->SetVolume( 1.0f - fVolume, 0, TTRUE );
		pStream->SetVolume( 1.0f - fVolume, 1, TTRUE );
	}
	else if ( pStream->Play( 1.0f ) == TTRUE )
	{
		pStream->SetVolume( 1.0f, 0, TTRUE );
		pStream->SetVolume( 1.0f, 1, TTRUE );
		pStream->SetVolume( 0.0f, 2, TTRUE );
		pStream->SetVolume( 0.0f, 3, TTRUE );
	}
}

// $Barnyard: FUNCTION 004e0200
void ATapperMiniGameState::ReturnHeldGlass()
{
	AAnimatableObject* pHeld = m_pPlayer->GetAnimatableObject()->FindAttachedObject( TPS8( cb_rlocator ) );

	if ( !pHeld )
		return;

	if ( pHeld == m_pCarriedGlass )
	{
		DetachCarriedGlass();
		return;
	}

	GlassPool& rPool  = m_aGlassPools[ m_iGlassPool ];
	Glass*     pGlass = rPool.llUsed.PopBack();

	AAnimatableObjectManager::GetSingleton()->DetachObject( pGlass->pFull, m_pPlayer->GetAnimatableObject() );
	pGlass->pFull->SetSkeletonUpdating( TFALSE, TTRUE );
	pGlass->pEmpty->SetSkeletonUpdating( TFALSE, TTRUE );
	pGlass->pFull->SetVisible( TFALSE );
	pGlass->pEmpty->SetVisible( TFALSE );

	rPool.llFree.PushBack( pGlass );
	rPool.iNumGlasses++;
}

// $Barnyard: FUNCTION 004e02e0
void ATapperMiniGameState::CancelGlass()
{
	if ( m_iGlassPool < 0 )
		return;

	AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();

	if ( !pPlayerObject->IsAnimPlaying( TPS8( throw ), 0.0f ) && !m_pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( throw ) ) )
		ReturnHeldGlass();
}

// $Barnyard: FUNCTION 004e0330
TBOOL ATapperMiniGameState::IsHoldingFullGlass()
{
	AAnimatableObject* pHeld = m_pPlayer->GetAnimatableObject()->FindAttachedObject( TPS8( cb_rlocator ) );
	return pHeld && pHeld != m_pCarriedGlass;
}

// $Barnyard: FUNCTION 004e0370
void ATapperMiniGameState::ThrowHeldGlass()
{
	ASoundManager::GetSingleton()->StopCue( m_iFillCue );
	m_iFillCue = -1;

	// NOTE: calls the empty function 0054df00 with the main controller
	StopRumble( m_iRumble, AInputHandler::GetSingleton()->GetMainController() );

	AAnimatableObject* pHeld = m_pPlayer->GetAnimatableObject()->FindAttachedObject( TPS8( cb_rlocator ) );

	if ( !pHeld || pHeld == m_pCarriedGlass )
	{
		m_ePlayerState = PLAYERSTATE_IDLE;
	}
	else
	{
		m_pPlayer->GetAnimatableObject()->KillAllAnimations();
		m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( throw ) );
		m_bTowardsEnd  = TTRUE;
		m_ePlayerState = PLAYERSTATE_THROWING;
	}
}

// $Barnyard: FUNCTION 004e0420
void ATapperMiniGameState::ResetGlasses()
{
	GlassPool& rPool = m_aGlassPools[ 0 ];

	rPool.iNumGlasses = MAX_POOL_GLASSES;
	rPool.llFree.Clear();
	rPool.llUsed.Clear();

	for ( TINT i = 0; i < MAX_GLASSES; i++ )
	{
		Glass& rGlass = m_aGlasses[ i ];

		rGlass.pFull->SetSkeletonUpdating( TFALSE, TTRUE );
		rGlass.pEmpty->SetSkeletonUpdating( TFALSE, TTRUE );
		rGlass.pFull->SetVisible( TFALSE );
		rGlass.pEmpty->SetVisible( TFALSE );
		rGlass.vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

		rPool.llFree.PushFront( &rGlass );
	}
}

// $Barnyard: FUNCTION 004e04a0
// $BarnyardWiiJP: FUNCTION 8022e228
TBOOL ATapperMiniGameState::LoadData()
{
	TTRB oTRB;

	if ( oTRB.Load( "Data\\MinigameData\\Tapper\\tapper.trb" ) != TTRB::ERROR_OK )
		return TFALSE;

	TUtil::MemCopy( m_afParams, oTRB.GetSymbolAddress( "Floats" ), sizeof( m_afParams ) );
	TUtil::MemCopy( m_aiParams, oTRB.GetSymbolAddress( "Ints" ), sizeof( m_aiParams ) );

	TINT         iNumLevels = *TSTATICCAST( TINT, oTRB.GetSymbolAddress( "LCount" ) );
	const Level* pLevels    = TSTATICCAST( Level, oTRB.GetSymbolAddress( "Levels" ) );

	m_oLevels.Clear();

	for ( TINT i = 0; i < iNumLevels; i++ )
		m_oLevels.aLevels[ m_oLevels.iNumLevels++ ] = pLevels[ i ];

#ifdef BARNYARD_LATEST
	// Latest version (Wii 8022e228): slower drinkers on Easy
	if ( APlayerProgress::GetSingleton()->IsEasyDifficulty() )
	{
		m_afParams[ FLOATPARAM_DRINKER_SPEED ] *= 0.85f;

		for ( TINT i = 0; i < 4; i++ )
			m_afParams[ FLOATPARAM_WALK_SPEED_BAR1 + i ] *= 0.85f;
	}

	// Latest version: the minimum glass speed (FLOATPARAM_FILL_MIN) is raised from 3.0 to 3.5.
	// NOTE: the Wii copies Floats[ 36 ] into Floats[ 33 ], where its tapper.trb inserts ( 3.0, 3.5 ) at index 35.
	// We load the PC data, so the value is set directly.
	m_afParams[ FLOATPARAM_FILL_MIN ] = 3.5f;
#endif

	return TTRUE;
}

// $Barnyard: FUNCTION 004e05d0
void ATapperMiniGameState::PlayJumpDust()
{
	TVector4 vPos  = m_vPlayerPos;
	TVector4 vStep = GetBarTap( m_iBar ) - vPos;
	vStep.Divide( 10.0f );

	for ( TINT i = 0; i < 10; i++ )
	{
		AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_DUSTPUFF ], vPos, TVector4::VEC_NEGY );
		vPos.x += vStep.x;
		vPos.y += vStep.y;
		vPos.z += vStep.z;
	}
}

// $Barnyard: FUNCTION 004e0700
void ATapperMiniGameState::AddScore( PlayerData& a_rPlayer, TINT a_iScore )
{
	a_rPlayer.iScore += a_iScore;
	m_iTotalScore += a_iScore;
	m_oPortrait.SetScore( a_rPlayer.iScore );

	if ( a_rPlayer.iScore > ( a_rPlayer.iBonusLives + 1 ) * m_aiParams[ INTPARAM_BONUS_LIFE_SCORE ] &&
	     m_eEndState != ENDSTATE_LOST && m_oStartHudElement.IsForceHidden() == TTRUE )
	{
		a_rPlayer.iBonusLives++;

		if ( a_rPlayer.iLives < m_aiParams[ INTPARAM_MAX_LIVES ] )
		{
			AddBonusLife( a_rPlayer );

			m_aBonusLifePopups[ m_iBonusLifePopup ].Show( ALocaleManager::Interface()->GetString( LOCALE_BONUS_LIFE ), m_oBonusLifeTextBox.GetScale(), &m_oBonusLifePopup, 0xffffffff, 0xffffc800 );
			m_iBonusLifePopup = ( m_iBonusLifePopup + 1 ) % NUM_BONUS_LIFE_POPUPS;
			m_oBonusLifePopup.SetShownInstantly( TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 004e07e0
void ATapperMiniGameState::OnStart()
{
	AMiniGameState::OnStart();
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );
}

// $Barnyard: FUNCTION 004e0800
void ATapperMiniGameState::OnInstructionsShown()
{
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );
	m_pCurtain->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pCurtain->SetVisible( TFALSE );
	AGUI2ModelRendererScreen::GetSingleton()->SetEnabled( TFALSE );
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );
}

// $Barnyard: FUNCTION 004e0850
TBOOL ATapperMiniGameState::CanDismissInstructions()
{
	// The instruction screen can't be dismissed, the curtain closes and OnCurtainClosed dismisses it
	m_pCurtain->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pCurtain->SetVisible( TTRUE );
	AGUI2ModelRendererScreen::GetSingleton()->SetEnabled( TTRUE );
	m_pCurtain->PlayAnimation( TPS8( close ) );
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );

	return TFALSE;
}

// $Barnyard: FUNCTION 004e08a0
void ATapperMiniGameState::OnCurtainClosed()
{
	AGameState* pInstructions = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( AMiniGameInstructionState ) );

	if ( pInstructions )
	{
		TSTATICCAST( AMiniGameInstructionState, pInstructions )->Dismiss();
		AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );
	}

	if ( m_eEndState == ENDSTATE_NONE && !m_pCurtain->IsAnimationPlanned( TPS8( open ) ) && !m_pCurtain->IsAnimPlaying( TPS8( open ), 0.0f ) )
		m_pCurtain->PlayAnimation( TPS8( open ) );

	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );
}

// $Barnyard: FUNCTION 004e0920
void ATapperMiniGameState::PlayLoseSequence()
{
	m_pCurtain->PlayAnimation( TPS8( open ) );
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );

	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sstap_lose", "sstap_lose", "SSTAP_CameraB", TVector4::VEC_ZERO, TTRUE, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004e09e0
void ATapperMiniGameState::PlayWinSequence()
{
	if ( !m_pCurtain->IsAnimationPlanned( TPS8( open ) ) && !m_pCurtain->IsAnimPlaying( TPS8( open ), 0.0f ) )
		m_pCurtain->PlayAnimation( TPS8( open ) );

	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );

	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sstap_win", "sstap_win", "SSTAP_CameraB", TVector4::VEC_ZERO, TTRUE, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004e0ad0
void ATapperMiniGameState::Pause()
{
	m_bIntroStarted = TTRUE;

	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sstap_intro", "sstap_intro", "SSTAP_CameraB", TVector4::VEC_ZERO, TTRUE, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TFALSE, TTRUE )
	);
}

// $Barnyard: FUNCTION 004e0b60
void ATapperMiniGameState::OnResultsShown()
{
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );
}

// $Barnyard: FUNCTION 004e0b80
void ATapperMiniGameState::CreateHUD()
{
	T2GUIElement* pRoot = &GetGUIElement();

	m_oPortrait.Create( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ), AGUI2Portrait::FLAGS_LEFT | AGUI2Portrait::FLAGS_NO_NAME, pRoot );
	pRoot->AddChildTail( m_oPortrait );
	m_oPortrait.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	// NOTE: inlined 00647300 (SetPortraitNameFromProfile)
	m_oPortrait.SetName( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString( 0 ) );
	m_oPortrait.Hide();

	m_oLivesHud.Create( pRoot, T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT, m_aiParams[ INTPARAM_MAX_LIVES ], "Tap_Glass", 1.0f, 10.0f, TFALSE );
	m_oLivesHud.SetVisible( TFALSE );

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		PlayerData& rPlayer     = m_aPlayers[ i ];
		rPlayer.iScore          = 0;
		rPlayer.iTipsCollected  = 0;
		rPlayer.iTips           = 0;
		rPlayer.iTimeBonus      = 0;
		rPlayer.iBonusLives     = 0;
		rPlayer.iMilkServed     = 0;
		rPlayer.iGlassesDropped = 0;
		rPlayer.iLevel          = 0;
		rPlayer.iLives          = m_aiParams[ INTPARAM_MAX_LIVES ];
	}

	m_oLivesHud.SetLives( 0, TFALSE );
	m_oLivesHud.AddLives( GetCurrentPlayer().iLives );
	AddScore( GetCurrentPlayer(), 0 );

	m_oMilkFill.Hide();
	m_oMilkFill.Create();
	pRoot->AddChildTail( m_oMilkFill );
	m_oMilkFill.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );

	T2GUIElement& rPickUpBacking = m_oPickUpHudElement.m_oBacking;
	rPickUpBacking.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	rPickUpBacking.SetTranslation( 0.0f, m_oPortrait.GetHeight() );
	rPickUpBacking.Hide();

	m_oWildMikeIcon.Create( T2GUITextureSectionManager::GetTextureSection( "Wild_Mike" ), 1.0f );
	m_oWildMikeIcon.SetShown( TFALSE );
	m_oWildMikeIcon.SetEffect( 1, TFALSE );
	m_oWildMikeIcon.SetEffect( 2, TTRUE );
	m_oWildMikeIcon.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oWildMikeIcon.SetTranslation( 0.0f, rPickUpBacking.GetHeight() );
	m_oPortrait.AddChildTail( m_oWildMikeIcon );

	T2GUIFont*    pFont   = T2GUIFontManager::FindFont( "rekord26" );
	const TWCHAR* wszText = ALocaleManager::Interface()->GetString( LOCALE_BONUS_LIFE );
	m_oBonusLifeTextBox.Create( pFont, pFont->GetTextWidth( wszText, 1.0f ) );
	m_oBonusLifeTextBox.SetColour( 0xffffc800 );
	m_oBonusLifeTextBox.SetText( wszText );
	m_oBonusLifeTextBox.SetScale( 0.8f );
	m_oBonusLifeTextBox.Show();

	m_oBonusLifePopup.Create( &m_oBonusLifeTextBox );
	m_oBonusLifePopup.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oBonusLifePopup.SetShownInstantly( TFALSE );
	m_oBonusLifePopup.m_fPulseSpeed = 10.0f;
	m_oBonusLifePopup.SetEffect( 1, TFALSE );
	m_oLivesHud.m_oBacking.AddChildTail( m_oBonusLifePopup );

	for ( TINT i = 0; i < NUM_BONUS_LIFE_POPUPS; i++ )
	{
		m_aBonusLifePopups[ i ].Create();
		pRoot->AddChildTail( m_aBonusLifePopups[ i ] );
	}
}
// $Barnyard: FUNCTION 004e0e30
void ATapperMiniGameState::SetupCamera()
{
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	s_vCameraDirection.Normalise();
	ATerrainInterface::GetSingleton()->GetLocatorPosAndAxis( "camera", s_vCameraPosition, s_vCameraDirection, 2 );

	ACamera* pCamera                                               = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) = s_vCameraPosition;
	pCamera->LookAtDirection( s_vCameraDirection );

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( GetFOV() );
}

// $Barnyard: FUNCTION 004e0ec0
ATapperMiniGameState::Drinker* ATapperMiniGameState::FindClosestDrinker( const TVector4& a_rcPosition, TFLOAT a_fMaxDistance, Drinker* a_pIgnore )
{
	Drinker* pClosest = TNULL;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		T2_FOREACH( m_aBars[ i ].llActive, it )
		{
			if ( a_pIgnore == it )
				continue;

			TVector4 vPos      = *it->pUnit->GetSteer()->GetPosition();
			TFLOAT   fDistance = TVector4::DistanceXZ( a_rcPosition, vPos );

			if ( fDistance < a_fMaxDistance )
			{
				a_fMaxDistance = fDistance;
				pClosest       = it;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 004e0f90
ATapperMiniGameState::Drinker* ATapperMiniGameState::FindClosestDrinkerOnLine( const TVector4& a_rcPosition, TFLOAT a_fMaxDistance, Drinker* a_pDrinker, TBOOL a_bIgnoreLeaving )
{
	TVector4 vStart = GetEnemyStart( a_pDrinker->iBar );
	TVector4 vEnd   = GetEnemyEnd( a_pDrinker->iBar );

	// Project the position onto the line of the bar
	TVector4 vPos = a_rcPosition;
	TFLOAT   fT;
	TMath::DistancePointToLineSq( vPos, vStart, vEnd, fT );

	TVector4 vLine = vEnd - vStart;
	TVector4 vOffset;
	vOffset.Multiply( vLine, fT );

	vPos.x = vOffset.x + vStart.x;
	vPos.y = vOffset.y + vStart.y;
	vPos.z = vOffset.z + vStart.z;
	vPos.w = vStart.w;

	Drinker* pClosest = TNULL;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		T2_FOREACH( m_aBars[ i ].llActive, it )
		{
			if ( a_pDrinker == it || a_pDrinker->iBar != it->iBar )
				continue;

			if ( a_bIgnoreLeaving && ( it->eState == DRINKERSTATE_10 || it->eState == DRINKERSTATE_11 ) )
				continue;

			TVector4 vOther = *it->pUnit->GetSteer()->GetPosition();

			TFLOAT fOtherT;
			TMath::DistancePointToLineSq( vOther, vStart, vEnd, fOtherT );

			TVector4 vOtherOffset;
			vOtherOffset.Multiply( vEnd - vStart, fOtherT );

			vOther.x = vOtherOffset.x + vStart.x;
			vOther.y = vOtherOffset.y + vStart.y;
			vOther.z = vOtherOffset.z + vStart.z;
			vOther.w = vStart.w;

			TFLOAT fDistance = TVector4::DistanceXZ( vPos, vOther );
			TBOOL  bCloser;

			if ( !a_bIgnoreLeaving || it->eState == DRINKERSTATE_DRINKING )
				bCloser = fDistance < a_fMaxDistance;
			else
				bCloser = fDistance + 0.3f < a_fMaxDistance;

			if ( bCloser )
			{
				a_fMaxDistance = fDistance;
				pClosest       = it;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 004e1350
T2SList<ATapperMiniGameState::Glass>::Iterator ATapperMiniGameState::FreeGlass( Glass* a_pGlass, TINT a_iPool )
{
	ASoundManager::GetSingleton()->StopCue( a_pGlass->iCue );
	a_pGlass->iCue      = -1;
	a_pGlass->vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

	a_pGlass->pFull->SetSkeletonUpdating( TFALSE, TTRUE );
	a_pGlass->pEmpty->SetSkeletonUpdating( TFALSE, TTRUE );
	a_pGlass->eState = GLASSSTATE_FREE;

	// NOTE: the results of these are not used
	if ( a_pGlass->pFull->GetParentObject() )
		a_pGlass->pFull->GetParentObject()->FindAttachedObject( TPS8( cb_llocator ) );

	if ( a_pGlass->pEmpty->GetParentObject() )
		a_pGlass->pEmpty->GetParentObject()->FindAttachedObject( TPS8( cb_llocator ) );

	return m_aGlassPools[ a_iPool ].llUsed.Transfer( a_pGlass, m_aGlassPools[ a_iPool ].llFree );
}

// $Barnyard: FUNCTION 004e1410
TINT ATapperMiniGameState::GetNumMovingGlasses()
{
	TINT iNumGlasses = 0;

	for ( auto it = m_aGlassPools[ 0 ].llUsed.Begin(); it != m_aGlassPools[ 0 ].llUsed.End(); )
	{
		Glass* pGlass = it;
		it++;

		if ( pGlass->vVelocity.x != 0.0f || pGlass->vVelocity.y != 0.0f || pGlass->vVelocity.z != 0.0f )
		{
			if ( ( pGlass->pFull->GetModelInstance().Get() && pGlass->pFull->GetModelInstance()->IsVisible() ) ||
			     ( pGlass->pEmpty->GetModelInstance().Get() && pGlass->pEmpty->GetModelInstance()->IsVisible() ) )
			{
				iNumGlasses++;
			}
		}
	}

	return iNumGlasses;
}

// $Barnyard: FUNCTION 004e14b0
void ATapperMiniGameState::RandomiseDanceDelays()
{
	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		for ( auto it = m_aBars[ i ].llActive.Begin(); it != m_aBars[ i ].llActive.End(); )
		{
			Drinker* pDrinker = it;
			it++;

			if ( pDrinker->pUnit->IsVisible() )
				pDrinker->fDance = ARandom::GetSingleton()->m_oRandom.GetFloat() - 1.0f;
		}
	}
}

// $Barnyard: FUNCTION 004e1560
TBOOL ATapperMiniGameState::IsAnyDrinkerInState( DRINKERSTATE a_eState )
{
	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		for ( auto it = m_aBars[ i ].llActive.Begin(); it != m_aBars[ i ].llActive.End(); )
		{
			Drinker* pDrinker = it;
			it++;

			if ( pDrinker->pUnit->IsVisible() && pDrinker->eState == a_eState )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004e15e0
AMiniGameState::S1* ATapperMiniGameState::GetResults()
{
	m_oResults.oPlayerList[ 0 ].iUnk2 = m_aPlayers[ 0 ].iScore;
	m_oResults.oPlayerList[ 0 ].bUnk1 = TTRUE;
	m_oResults.iResult                = -1;
	m_oResults.iMiniGameIndex         = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();

	TINT        iPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerIterator();
	PlayerInfo& rInfo   = m_oResults.oPlayerList[ iPlayer ];

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	PlayerData& rPlayer = m_aPlayers[ iPlayer ];
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_MILK_SERVED ), rPlayer.iMilkServed } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_GLASSES_DROPPED ), rPlayer.iGlassesDropped } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_TIME_BONUS ), rPlayer.iTimeBonus } );

	m_oResults.ComputeTotal( iPlayer );

	if ( rInfo.iTotal < 0 )
		rInfo.iTotal = 0;

	if ( rInfo.iUnk2 < 0 )
		rInfo.iUnk2 = 0;

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		TINT iRecord;

		switch ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() )
		{
			case 0: iRecord = HIGHSCORE_TABLE + 0; break;
			case 1: iRecord = HIGHSCORE_TABLE + 1; break;
			case 2: iRecord = HIGHSCORE_TABLE + 2; break;
			case 3: iRecord = HIGHSCORE_TABLE + 3; break;
			default: return &m_oResults;
		}

		// NOTE: the total of the first player is used
		TINT                     iTotal = m_oResults.oPlayerList[ 0 ].iTotal;
		AHighScoreManager::GRADE eGrade = AHighScoreManager::GetGrade( iRecord, TFLOAT( iTotal ) );
		m_oResults.bWon                 = eGrade != AHighScoreManager::GRADE_F;
		m_oResults.iResult              = ( eGrade != AHighScoreManager::GRADE_F ) - 1;
		AHighScoreManager::AddScore( iRecord, TFLOAT( iTotal ), eGrade != AHighScoreManager::GRADE_F, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 004e17d0
ATapperMiniGameState::Drinker* ATapperMiniGameState::FindDrinkerForGlass( const TVector4& a_rcGlassStart, const TVector4& a_rcGlassEnd, TFLOAT& a_rfOutDistance, TINT a_iBar )
{
	TVector2 vBarStart( GetEnemyStart( a_iBar ).x, GetEnemyStart( a_iBar ).z );
	TVector2 vBarEnd( GetEnemyEnd( a_iBar ).x, GetEnemyEnd( a_iBar ).z );
	TVector2 vBeerStart( GetPlayerBeerStart( a_iBar ).x, GetPlayerBeerStart( a_iBar ).z );
	TVector2 vBeerEnd( GetPlayerBeerEnd( a_iBar ).x, GetPlayerBeerEnd( a_iBar ).z );

	Drinker* pFound    = TNULL;
	TFLOAT   fDistance = TMath::DistanceSegmentToSegment2D( vBeerStart, vBeerEnd, vBarEnd, vBarStart ) + 0.1f;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		T2_FOREACH( m_aBars[ i ].llActive, it )
		{
			TVector4 vPos = *it->pUnit->GetSteer()->GetPosition();

			if ( a_iBar != it->iBar )
				continue;

			TVector2 vDrinkerPos( vPos.x, vPos.z );
			TVector2 vDrinkerPrevPos( it->vPrevPos.x, it->vPrevPos.z );
			TVector2 vGlassStart( a_rcGlassStart.x, a_rcGlassStart.z );
			TVector2 vGlassEnd( a_rcGlassEnd.x, a_rcGlassEnd.z );

			TFLOAT fDrinkerDistance = TMath::DistanceSegmentToSegment2D( vGlassEnd, vGlassStart, vDrinkerPos, vDrinkerPrevPos );

			if ( fDrinkerDistance < fDistance && it->fDance <= 0.0f )
			{
				AAnimatedUnit* pUnit = it->pUnit;

				if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( slideback ), 0.0f ) &&
				     !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( slideback ) ) &&
				     !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( dance ), 0.0f ) &&
				     !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( dance ) ) )
				{
					pFound    = it;
					fDistance = fDrinkerDistance;
				}
			}
		}
	}

	a_rfOutDistance = fDistance;
	return pFound;
}

// $Barnyard: FUNCTION 004e1a00
ATapperMiniGameState::Drinker* ATapperMiniGameState::FindDrinker( AUnit* a_pUnit )
{
	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		T2_FOREACH( m_aBars[ i ].llActive, it )
		{
			if ( it->pUnit == a_pUnit )
				return it;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004e1a60
ATapperMiniGameState::Glass* ATapperMiniGameState::FindGlass( AAnimatableObject* a_pObject )
{
	for ( TINT i = 0; i < GlassPools::NUM_POOLS; i++ )
	{
		T2_FOREACH( m_aGlassPools[ i ].llUsed, it )
		{
			if ( it->pFull == a_pObject || it->pEmpty == a_pObject )
				return it;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004e1ac0
ATapperMiniGameState::Glass* ATapperMiniGameState::FindGlass( AAnimatableObject* a_pObject, TINT& a_rOutPool )
{
	for ( TINT i = 0; i < GlassPools::NUM_POOLS; i++ )
	{
		T2_FOREACH( m_aGlassPools[ i ].llUsed, it )
		{
			if ( it->pFull == a_pObject || it->pEmpty == a_pObject )
			{
				a_rOutPool = i;
				return it;
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004e1b30
void ATapperMiniGameState::AddScoreAtPlayer( TINT a_iScore )
{
	TVector4 vPos = m_vPlayerPos;
	vPos.y -= 2.5f;

	TVector4 vOffset;
	vOffset.Multiply( m_vTapDirection, 0.3f );

	vPos.x -= vOffset.x;
	vPos.y -= vOffset.y;
	vPos.z -= vOffset.z;

	AddScoreFloat( vPos, a_iScore );
	AddScore( GetCurrentPlayer(), a_iScore );
}

// $Barnyard: FUNCTION 004e1c00
void ATapperMiniGameState::CollectTip( TINT a_iTip )
{
	TVector4 vUnused;
	vUnused.Multiply( m_vTapDirection, 0.3f );

	GetCurrentPlayer().iTipsCollected++;
	GetCurrentPlayer().iTips++;

	if ( m_eEndState != ENDSTATE_LOST &&
	     !m_pRoot->IsAnimationPlanned( TPS8( drop_glass ) ) && !m_pRoot->IsAnimPlaying( TPS8( drop_glass ), 0.0f ) &&
	     !m_pRoot->IsAnimationPlanned( TPS8( angry_lp ) ) && !m_pRoot->IsAnimPlaying( TPS8( angry_lp ), 0.0f ) &&
	     !m_pRoot->IsAnimationPlanned( TPS8( money ) ) && !m_pRoot->IsAnimPlaying( TPS8( money ), 0.0f ) )
	{
		m_pRoot->PlayAnimation( TPS8( money ) );
	}

	StartCollectEffect( m_aTips[ a_iTip ].pObject, 1, 15.0f );
	m_aTips[ a_iTip ].eState = TIPSTATE_COLLECTED;

	if ( ARandom::GetSingleton()->m_oRandom.GetInt( 4 ) == 0 )
	{
		if ( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetGender() != APlayerProfile::GENDER_MALE )
			ASoundManager::GetSingleton()->PlayCue( soundbank::AM_F_WAVE );
		else
			ASoundManager::GetSingleton()->PlayCue( soundbank::AM_M_WAVE );
	}
}

// $Barnyard: FUNCTION 004e1d70
void ATapperMiniGameState::StopGlassSounds()
{
	// NOTE: calls the empty function 0054df00 with the main controller
	StopRumble( m_iRumble, AInputHandler::GetSingleton()->GetMainController() );

	for ( auto it = m_aGlassPools[ 0 ].llUsed.Begin(); it != m_aGlassPools[ 0 ].llUsed.End(); )
	{
		Glass* pGlass = it;
		it++;

		ASoundManager::GetSingleton()->StopCue( pGlass->iCue );
		pGlass->iCue = -1;
	}

	ASoundManager::GetSingleton()->StopCue( m_iFillCue );
	m_iFillCue = -1;
}

// $Barnyard: FUNCTION 004e1df0
ATapperMiniGameState::Drinker* ATapperMiniGameState::FindDrinkerInFront( Drinker* a_pDrinker, TFLOAT a_fMaxDistance )
{
	TVector4 vPos = *a_pDrinker->pUnit->GetSteer()->GetPosition();

	TVector4 vLine = GetEnemyStart( a_pDrinker->iBar ) - GetEnemyEnd( a_pDrinker->iBar );
	TVector4 vFar  = vLine + GetEnemyStart( a_pDrinker->iBar );
	vFar.w         = GetEnemyStart( a_pDrinker->iBar ).w;

	TFLOAT fDistance = TVector4::DistanceXZ( vPos, vFar );

	Drinker* pFound = TNULL;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		T2_FOREACH( m_aBars[ i ].llActive, it )
		{
			if ( a_pDrinker == it || it->iBar != a_pDrinker->iBar || it->eState == DRINKERSTATE_11 || a_pDrinker->eState == DRINKERSTATE_10 )
				continue;

			TVector4 vOther = *it->pUnit->GetSteer()->GetPosition();
			TFLOAT   fDiff  = TVector4::DistanceXZ( vOther, vFar ) - fDistance;

			if ( fDiff >= 0.0f && fDiff < a_fMaxDistance )
			{
				a_fMaxDistance = fDiff;
				pFound         = it;
			}
		}
	}

	return pFound;
}

// $Barnyard: FUNCTION 004e2390
void ATapperMiniGameState::CreateRoot()
{
	m_pRoot = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( AAnimatableObjectManager::GetSingleton()->FindType( TPS8( tap_root ) ), TNULL, 0 );
	m_pRoot->SetAnimControllersDisabled( TTRUE );
	m_pRoot->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pRoot->SetVisible( TTRUE );

	TVector4    vPos;
	TQuaternion quatRot;
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "root_pos", vPos, quatRot );

	TVector4 vOffset = quatRot.AsVector4();
	vOffset.AsVector3().RotateY( TMath::PI * 0.5f );
	vOffset.Multiply( 1.3f );

	TVector4 vRootPos = vOffset + vPos;
	vRootPos.w        = vPos.w;

	// NOTE: the locator rotation is passed as the direction
	m_pRoot->GetModelInstance()->SetTransform( vRootPos, quatRot.AsVector4() );
	m_pRoot->PlayAnimation( TPS8( idle_lp ) );
}

// $Barnyard: FUNCTION 004e2520
void ATapperMiniGameState::CreateCurtain()
{
	m_pCurtain = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( AAnimatableObjectManager::GetSingleton()->FindType( TPS8( tap_curtain ) ), TNULL, 0 );
	m_pCurtain->SetAnimControllersDisabled( TTRUE );
	m_pCurtain->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pCurtain->SetVisible( TFALSE );
	m_pCurtain->PlayAnimation( TPS8( opened_idle ) );

	TVector4 vPos;
	vPos.Multiply( TVector4::VEC_POSZ, 8.0f );
	m_pCurtain->GetModelInstance()->SetTransform( vPos, TVector4::VEC_NEGZ );
}

// $Barnyard: FUNCTION 004e2630
void ATapperMiniGameState::CreateTables()
{
	const TPString8* apTypes[ 4 ] = {
		&TPS8( tp_table3_add ),
		&TPS8( tp_table4_add ),
		&TPS8( tp_table3_sub ),
		&TPS8( tp_table4_sub ),
	};

	TVector4    avPositions[ 4 ];
	TQuaternion aRotations[ 4 ];

	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "tp_table3_add", avPositions[ 0 ], aRotations[ 0 ] );
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "tp_table4_add", avPositions[ 1 ], aRotations[ 1 ] );
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "tp_table3_sub", avPositions[ 2 ], aRotations[ 2 ] );
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "tp_table4_sub", avPositions[ 3 ], aRotations[ 3 ] );

	for ( TINT i = 0; i < 4; i++ )
	{
		m_apTables[ i ] = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( AAnimatableObjectManager::GetSingleton()->FindType( *apTypes[ i ] ), TNULL, 0 );

		// NOTE: the locator rotation is passed as the direction
		m_apTables[ i ]->GetModelInstance()->SetTransform( avPositions[ i ], aRotations[ i ].AsVector4() );
		m_apTables[ i ]->GetModelInstance()->SetVisible( TTRUE );
		m_apTables[ i ]->SetAnimControllersDisabled( TTRUE );
	}

	// The "add" tables are only shown with 4 bars
	m_apTables[ TABLE_3_ADD ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
	m_apTables[ TABLE_4_ADD ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
	m_apTables[ TABLE_3_SUB ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
	m_apTables[ TABLE_4_SUB ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
}

// $Barnyard: FUNCTION 004e2940
void ATapperMiniGameState::CreateWildMike()
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	TQuaternion        quatRotation;

	for ( TINT i = 0; i < NUM_BARS; i++ )
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "wildmike_start%d", i + 1 ), m_avWildMikeStart[ i ], quatRotation );

	for ( TINT i = 0; i < NUM_BARS; i++ )
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "wildmike_end%d", i + 1 ), m_avWildMikeEnd[ i ], quatRotation );

	m_pWildMike = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( AAnimatableObjectManager::GetSingleton()->FindType( TPS8( tap_wildmike ) ), TNULL, 0 );
	m_pWildMike->SetAnimControllersDisabled( TTRUE );
	m_pWildMike->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pWildMike->SetVisible( TFALSE );

	// NOTE: the locator rotation (of wildmike_end4) is passed as the direction
	m_pWildMike->GetModelInstance()->SetTransform( m_avWildMikeStart[ 0 ], quatRotation.AsVector4() );
	m_pWildMike->PlayAnimation( TPS8( idle_lp ) );
	m_pWildMike->GetModelInstance()->SetUnlit();
}

// $Barnyard: FUNCTION 004e2b30
void ATapperMiniGameState::SetupInteriorNetwork()
{
	AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( TPS8D( "tapper" ) );
	m_pNetwork      = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	m_iNumDestGoals = 0;

	// Count the "dest" goals, one per bar
	for ( TINT i = m_pNetwork->m_pINetwork->m_iNumGoals - 1; i >= 0; i-- )
	{
		const TPString8* pName = m_pNetwork->m_pINetwork->m_pGoals[ i ].m_strName;

		if ( TStringManager::String8FindString( pName->GetString(), "dest" ) )
			m_iNumDestGoals++;
	}
}

// $Barnyard: FUNCTION 004e2c10
void ATapperMiniGameState::FacePlayer( TFLOAT a_fAngle )
{
	TVector4 vDirection = GetBarEnd( m_iBar ) - GetBarTap( m_iBar );
	vDirection.Normalise();
	vDirection.AsVector3().RotateY( a_fAngle );

	m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetDirection( vDirection );
}

// $Barnyard: FUNCTION 004e2d00
void ATapperMiniGameState::AttachCarriedGlass()
{
	if ( m_pPlayer->GetAnimatableObject()->FindAttachedObject( TPS8( cb_rlocator ) ) )
		return;

	AAnimatableObjectManager::GetSingleton()->AttachObject( m_pCarriedGlass, m_pPlayer->GetAnimatableObject(), TPString8(), TPS8( cb_rlocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
	m_pCarriedGlass->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pCarriedGlass->SetVisible( TTRUE );
}

// $Barnyard: FUNCTION 004e2de0
void ATapperMiniGameState::PlayPlayerIdle()
{
	m_pPlayer->GetAnimatableObject()->KillAllAnimations();
	m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
	AttachCarriedGlass();
}

// $Barnyard: FUNCTION 004e2e20
void ATapperMiniGameState::ResetPlayer()
{
	m_vPlayerVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vPlayerPos      = s_vPlayerStart;

	FacePlayer( TMath::PI * 0.5f );
	m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );

	m_pPlayer->GetAnimatableObject()->KillAllAnimations();
	m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
	AttachCarriedGlass();

	m_ePlayerState = PLAYERSTATE_IDLE;
	m_iBar         = 2;
}
// $Barnyard: FUNCTION 004e2f40
void ATapperMiniGameState::UpdatePlayer( TFLOAT a_fDeltaTime )
{
	if ( m_eEndState != ENDSTATE_NONE )
		return;

	m_vPlayerPrevPos = m_vPlayerPos;

	// Facing the customers
	TVector4 vFacing = GetBarEnd( m_iBar ) - GetBarTap( m_iBar );
	vFacing.Normalise();
	vFacing.AsVector3().RotateY( TMath::PI * 0.5f );

	if ( m_ePlayerState == PLAYERSTATE_FILLING )
	{
		AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();

		if ( !pPlayerObject->IsAnimPlaying( TPS8( fillup ), 0.0f ) && !m_pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( fillup ) ) )
		{
			TFLOAT fFillRange = m_afParams[ FLOATPARAM_FILL_MAX ] - m_afParams[ FLOATPARAM_FILL_MIN ];
			m_fFill           = ( g_oSystemManager.GetScheduler()->GetTotalTime() - m_fFillStartTime - 0.2f ) * m_afParams[ FLOATPARAM_FILL_SPEED ] * fFillRange;

			if ( m_fFill < 0.0f )
				m_fFill = 0.0f;

			TFLOAT fFillMax = fFillRange * 100.0f;
			TFLOAT fFill    = m_fFill * 100.0f;
			TFLOAT fValue   = fFill - 20.0f;
			TFLOAT fMax     = fFillMax - 20.0f;
			TMath::Clip( fValue, 0.0f, fMax );
			m_oMilkFill.SetFill( fValue, fMax );

			if ( fFill > fFillMax )
			{
				// Overfilled
				ThrowHeldGlass();
				m_bOverfilled = TTRUE;
			}
		}

		m_fDripTimer -= a_fDeltaTime;
		TVector4 vSplash = GetSplash( m_iBar );

		if ( m_fDripTimer < 0.0f )
		{
			m_fDripTimer = m_afParams[ FLOATPARAM_DRIP_INTERVAL ];
			AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_SPLASH ], vSplash, TVector4::VEC_NEGY );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_DRIP ], vSplash, TVector4::VEC_POSY );
		}

		return;
	}

	if ( m_ePlayerState == PLAYERSTATE_GLASS_THROWN )
	{
		m_ePlayerState  = PLAYERSTATE_IDLE;
		m_bWaitForThrow = TTRUE;
		m_oMilkFill.SetFill( 0.0f, 1.0f );
		m_oMilkFill.Hide();
	}
	else if ( m_ePlayerState == PLAYERSTATE_IDLE && m_bWaitForThrow )
	{
		if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( throw ) ) && !m_bRunning )
		{
			AttachCarriedGlass();
			m_bWaitForThrow = TFALSE;
		}
	}

	TVector4 vTarget;

	if ( m_bRunning == TTRUE )
	{
		// Running along the bar
		vFacing.AsVector3().RotateY( TMath::PI * -0.5f );

		if ( m_bTowardsEnd == TFALSE )
			vFacing.Negate3();

		m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetDirection( vFacing );

		if ( m_fRunDirection != 0.0f )
		{
			GetPlayerTargetPos( vTarget, m_iBar );

			TVector4 vDirection = vTarget - m_vPlayerPos;
			vDirection.Normalise();

			if ( TVector4::Distance( vTarget, m_vPlayerPos ) <= 0.4f )
			{
				if ( m_fRunDirection < 0.0f )
				{
					// Back at the tap
					m_vPlayerPos = vTarget;
					PlayPlayerIdle();
					m_bRunning = TFALSE;

					TVector4 vDir = GetBarEnd( m_iBar ) - GetBarTap( m_iBar );
					vDir.Normalise();
					vDir.AsVector3().RotateY( TMath::PI * 0.5f );
					m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetDirection( vDir );
				}
			}
			else
			{
				vDirection.Normalise();
				m_vPlayerVelocity = vDirection * m_afParams[ FLOATPARAM_RUN_SPEED ];

				TVector4 vStep = m_vPlayerVelocity * a_fDeltaTime;
				m_vPlayerPos.x += vStep.x;
				m_vPlayerPos.y += vStep.y;
				m_vPlayerPos.z += vStep.z;
			}

			m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );
		}
	}
	else
	{
		// Moving to the tap of the current bar
		GetPlayerTargetPos( vTarget, m_iBar );
		TVector4 vToTarget = vTarget - m_vPlayerPos;

		if ( vToTarget.x != 0.0f || vToTarget.y != 0.0f || vToTarget.z != 0.0f )
		{
			vToTarget.Normalise();
			m_vPlayerVelocity = vToTarget;

			TVector4 vStep = m_vPlayerVelocity * a_fDeltaTime * m_afParams[ FLOATPARAM_JUMP_SPEED ];
			m_vPlayerPos.x += vStep.x;
			m_vPlayerPos.y += vStep.y;
			m_vPlayerPos.z += vStep.z;

			vToTarget = vTarget - m_vPlayerPos;
			vToTarget.Normalise();

			TVector4 vBarEnd = GetBarEnd( m_iBar );
			TVector4 vBarDir = vTarget - vBarEnd;
			vBarDir.Normalise();

			TVector4 vCross;
			vCross.CrossProduct( vBarDir, vToTarget );

			// Snap to the target once the player has passed it
			TBOOL bPassed = ( m_iJumpDirection >= 0 ) ? ( vCross.y > 0.0f ) : ( vCross.y < 0.0f );

			if ( bPassed )
				m_vPlayerPos = vTarget;
		}
		else
		{
			m_vPlayerPos = vTarget;
		}

		m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );
	}

	m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetTransform( m_vPlayerPos, vFacing );
}

// $Barnyard: FUNCTION 004e3720
T2SList<ATapperMiniGameState::Drinker>::Iterator ATapperMiniGameState::RemoveDrinker( Drinker* a_pDrinker )
{
	TINT iBar     = a_pDrinker->iBar;
	m_fSpawnTimer = GetLevel().fSpawnDelayAfterRemove;

	AAnimatableObject* pGlassObject = a_pDrinker->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) );

	if ( pGlassObject )
	{
		AAnimatableObjectManager::GetSingleton()->DetachObject( pGlassObject, a_pDrinker->pUnit->GetAnimatableObject() );

		TINT   iPool;
		Glass* pGlass = FindGlass( pGlassObject, iPool );
		FreeGlass( pGlass, iPool );
	}

	a_pDrinker->pUnit->SetVisible( TFALSE );
	a_pDrinker->pUnit->GetSteer()->Command_CancelCurrentCommand();
	a_pDrinker->pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
	a_pDrinker->pUnit->SetHashed( TFALSE );
	a_pDrinker->pUnit->SetNoQuery( TTRUE );

	auto itBefore = m_aBars[ a_pDrinker->iLane ].llActive.Transfer( a_pDrinker, m_aBars[ a_pDrinker->iLane ].llWaiting );

	m_aiNumBarDrinkers[ iBar - 1 ]--;
	m_iNumActiveDrinkers--;
	m_iNumApproaching--;
	m_iNumDrinkersGone++;

	return itBefore;
}

// $Barnyard: FUNCTION 004e3840
ATapperMiniGameState::Glass* ATapperMiniGameState::FindClosestGlass( const TVector4& a_rcPosition, TFLOAT a_fMaxDistance )
{
	Glass* pClosest = TNULL;

	for ( auto it = m_aGlassPools[ 0 ].llUsed.Begin(); it != m_aGlassPools[ 0 ].llUsed.End(); it++ )
	{
		TVector4 vPos;
		it->pEmpty->GetModelInstance()->GetPosition( vPos );

		TFLOAT fDistance = TVector4::DistanceXZ( a_rcPosition, vPos );

		if ( fDistance < a_fMaxDistance )
		{
			pClosest       = it;
			a_fMaxDistance = fDistance;
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 004e3920
TBOOL ATapperMiniGameState::UpdateDancing( TFLOAT a_fDeltaTime )
{
	if ( !m_bAnyDancing )
		return TFALSE;

	TVector4 vWildMikePos;
	m_pWildMike->GetModelInstance()->GetPosition( vWildMikePos );

	if ( m_eWildMikeState == WILDMIKESTATE_NONE )
		m_bAnyDancing = TFALSE;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		for ( auto it = m_aBars[ i ].llActive.Begin(); it != m_aBars[ i ].llActive.End(); )
		{
			Drinker* pDrinker = it;
			it++;

			AAnimatedUnit* pUnit = pDrinker->pUnit;

			if ( !pUnit->IsVisible() ||
			     pDrinker->eState == DRINKERSTATE_9 || pDrinker->eState == DRINKERSTATE_WALKUPTO || pDrinker->eState == DRINKERSTATE_RUNAWAY ||
			     pDrinker->eState == DRINKERSTATE_10 || pDrinker->eState == DRINKERSTATE_11 || pDrinker->eState == DRINKERSTATE_DRINKING ||
			     pDrinker->eState == DRINKERSTATE_DRUNK )
			{
				pDrinker->bDancing = TFALSE;
				continue;
			}

			TFLOAT fRandom = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.99f, 1.0f );
			TFLOAT fStep   = fRandom * m_afParams[ FLOATPARAM_DANCE_SPEED ] * a_fDeltaTime;

			if ( m_eWildMikeState != WILDMIKESTATE_NONE )
				pDrinker->fDance = fStep + pDrinker->fDance;
			else
				pDrinker->fDance = pDrinker->fDance - fStep;

			if ( pDrinker->fDance <= 0.0f )
			{
				if ( m_eWildMikeState != WILDMIKESTATE_NONE )
					continue;

				// Stop dancing
				pDrinker->fDance = 0.0f;

				TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
				vDirection.y        = 0.0f;
				vDirection.Normalise();

				pUnit->SetSteerDirection( vDirection );
				pUnit->GetAnimatableObject()->GetModelInstance()->SetDirection( vDirection );
				pUnit->GetAnimatableObject()->GetToshiAnimInterface().KillAllAnimations();
				pUnit->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp3 ) );

				pDrinker->bDancing = TFALSE;

				switch ( pDrinker->eState )
				{
					case DRINKERSTATE_APPROACH:
						StopDrinker( pDrinker );
						break;
					case DRINKERSTATE_DRINKING:
					case DRINKERSTATE_DRUNK:
						pDrinker->eState = DRINKERSTATE_DRUNK;
						break;
					case DRINKERSTATE_WAITFORWALK:
						// NOTE: the result isn't used
						pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp ), 0.0f );
						break;
					case DRINKERSTATE_WALKUPTO:
						pDrinker->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( walk_lp2 ) );
						break;
				}
			}
			else
			{
				TMath::Clip( pDrinker->fDance, 0.0f, 1.0f );

				if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( dance ), 0.0f ) && !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( dance ) ) )
					pUnit->GetAnimatableObject()->PlayAnimation( TPS8( dance ) );

				// Turn towards Wild Mike
				TVector4 vDrinkerPos;
				vDrinkerPos.Set( *pDrinker->pUnit->GetSteer()->GetPosition() );

				TVector4 vToWildMike = vWildMikePos - vDrinkerPos;
				vToWildMike.y        = 0.0f;
				vToWildMike.Normalise();

				TVector4 vForward;
				vForward.Set( TVector4::VEC_POSZ );

				TQuaternion quatForward;
				quatForward.SetVectorDirection( vForward.AsVector3(), TVector3( 0.0f, 0.0f, 1.0f ) );

				TQuaternion quatToWildMike;
				quatToWildMike.SetVectorDirection( vToWildMike.AsVector3(), TVector3( 0.0f, 0.0f, 1.0f ) );

				TQuaternion quatRotation;
				TQuaternion::Lerp( quatRotation, quatForward, quatToWildMike, pDrinker->fDance );

				RotateVector( vForward, quatRotation, TVector4( 0.0f, 0.0f, 1.0f, 1.0f ) );
				vForward.w = 1.0f;
				vForward.y = 0.0f;
				vForward.Normalise();

				pUnit->SetSteerDirection( vForward );
				pUnit->GetAnimatableObject()->GetModelInstance()->SetDirection( vForward );

				m_bAnyDancing      = TTRUE;
				pDrinker->bDancing = TTRUE;
			}
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004e3de0
void ATapperMiniGameState::UpdateTips( TFLOAT a_fDeltaTime )
{
	TFLOAT fPickUpDistanceSq   = m_afParams[ FLOATPARAM_TIP_RADIUS ] * m_afParams[ FLOATPARAM_TIP_RADIUS ];
	TFLOAT fWildMikeDistanceSq = fPickUpDistanceSq * 0.25f;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		if ( m_aTips[ i ].eState != TIPSTATE_ACTIVE )
			continue;

		AAnimatableObject* pTip = m_aTips[ i ].pObject;

		// Spin the tip
		TMatrix44 matTransform;
		pTip->GetModelInstance()->GetLocalTransform( matTransform );
		matTransform.RotateZ( TIP_SPIN_SPEED * a_fDeltaTime );
		pTip->GetModelInstance()->SetTransform( matTransform );

		TFLOAT fDX = m_vPlayerPos.x - matTransform.m_f41;
		TFLOAT fDZ = m_vPlayerPos.z - matTransform.m_f43;

		if ( fDZ * fDZ + fDX * fDX < fPickUpDistanceSq )
			CollectTip( i );

		if ( m_eWildMikeState != WILDMIKESTATE_LEAVING && m_eWildMikeState != WILDMIKESTATE_NONE )
		{
			// Wild Mike takes the tip
			TVector4 vWildMikePos;
			m_pWildMike->GetModelInstance()->GetPosition( vWildMikePos );

			if ( ( vWildMikePos.z - matTransform.m_f43 ) * ( vWildMikePos.z - matTransform.m_f43 ) + ( vWildMikePos.x - matTransform.m_f41 ) * ( vWildMikePos.x - matTransform.m_f41 ) < fWildMikeDistanceSq &&
			     m_aTips[ i ].eState != TIPSTATE_NONE )
			{
				m_aTips[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );
				m_aTips[ i ].eState = TIPSTATE_NONE;
			}
		}
	}
}

// $Barnyard: FUNCTION 004e4000
void ATapperMiniGameState::WildMikeLeave()
{
	if ( m_eWildMikeState == WILDMIKESTATE_LEAVING || m_eWildMikeState == WILDMIKESTATE_NONE )
		return;

	m_eWildMikeState = WILDMIKESTATE_LEAVING;

	m_pWildMike->GetModelInstance()->GetUpVector( m_vWildMikeDirection );
	m_vWildMikeDirection.x = -m_vWildMikeDirection.x;
	m_vWildMikeDirection.y = -m_vWildMikeDirection.y;
	m_vWildMikeDirection.z = -m_vWildMikeDirection.z;
	m_vWildMikeDirection.w = 1.0f;

	m_vWildMikeOnTableDirection = TVector4::VEC_POSZ;

	if ( m_vWildMikeDirection.IsEqual( m_vWildMikeOnTableDirection ) )
		m_pWildMike->PlayAnimation( TPS8( getofftable ) );

	m_iWildMikeScore = m_aiParams[ INTPARAM_WILDMIKE_SCORE ] + GetCurrentPlayer().iScore;
}
// $Barnyard: FUNCTION 004e4120
void ATapperMiniGameState::UpdateWildMike( TFLOAT a_fDeltaTime )
{
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	switch ( m_eWildMikeState )
	{
		case WILDMIKESTATE_NONE:
		{
			if ( m_eEndState != ENDSTATE_NONE || !IsMusicIntense() )
				return;

			if ( m_oWildMikeIcon.IsHiding() )
			{
				// Warn the player first
				ShowWildMikeIcon();
				m_fNextWildMikeTime = m_fLevelTime + m_afParams[ FLOATPARAM_WILDMIKE_WARNING_TIME ];
				ASoundManager::GetSingleton()->PlayCue( soundbank::TAPPA_FREDDY );
			}
			else if ( !IsAnyDrinkerInState( DRINKERSTATE_11 ) && !IsAnyDrinkerInState( DRINKERSTATE_10 ) && !IsAnyDrinkerInState( DRINKERSTATE_THROWBACK ) && !IsAnyDrinkerInState( DRINKERSTATE_RUNAWAY ) )
			{
				// Wild Mike gets on a table
				HideWildMikeIcon();
				m_eWildMikeState = WILDMIKESTATE_GETTING_ON;
				m_iWildMikeScore = m_aiParams[ INTPARAM_WILDMIKE_SCORE ] + GetCurrentPlayer().iScore;
				m_pWildMike->PlayAnimation( TPS8( getontable ) );

				m_iWildMikeTable = rRandom.GetInt( GetLevel().iNumBars );
				m_pWildMike->GetModelInstance()->SetTranslation( m_avWildMikeStart[ m_iWildMikeTable ].AsVector3() );
				m_iWildMikeWalkDirection = 1;
				m_pWildMike->SetSkeletonUpdating( TTRUE, TTRUE );
				m_pWildMike->SetVisible( TTRUE );

				m_vWildMikeOnTableDirection.x = rRandom.GetFloatMinMax( -1.0f, 1.0f );
				m_vWildMikeOnTableDirection.z = rRandom.GetFloatMinMax( -1.0f, 1.0f );
				m_vWildMikeOnTableDirection.y = 0.0f;
				m_vWildMikeOnTableDirection.Normalise();

				m_vWildMikeDirection = TVector4::VEC_POSZ;
				m_pWildMike->GetModelInstance()->SetDirection( m_vWildMikeDirection );

				m_fWildMikeTurn       = 0.0f;
				m_fWildMikeWalkTimer  = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME ] - m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME_RANGE ], m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME ] + m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME_RANGE ] );
				m_fWildMikeSwayTimer  = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME ] - m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE ], m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME ] + m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE ] );
				m_fWildMikeSway       = 0.0f;
				m_fWildMikeJumpTimer  = m_afParams[ FLOATPARAM_WILDMIKE_JUMP_TIME ];
				m_fWildMikeSpinTimer  = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_SPIN_TIME_MIN ], m_afParams[ FLOATPARAM_WILDMIKE_SPIN_TIME_MAX ] );
				m_fWildMikeDanceTimer = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_DANCE_TIME_MIN ], m_afParams[ FLOATPARAM_WILDMIKE_DANCE_TIME_MAX ] );
				m_fNextWildMikeTime   = m_fLevelTime + m_afParams[ FLOATPARAM_WILDMIKE_INTERVAL ];
			}

			return;
		}
		case WILDMIKESTATE_GETTING_ON:
		{
			if ( m_pWildMike->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
			{
				m_eWildMikeState = WILDMIKESTATE_WALKING;
				m_fWildMikeTime  = m_afParams[ FLOATPARAM_WILDMIKE_DURATION ];
				RandomiseDanceDelays();
				m_bAnyDancing = TTRUE;
			}

			return;
		}
		case WILDMIKESTATE_WALKING:
		case WILDMIKESTATE_DANCING:
		{
			m_fWildMikeTime -= a_fDeltaTime;
			m_fWildMikeJumpTimer -= a_fDeltaTime;
			m_fWildMikeSpinTimer -= a_fDeltaTime;
			m_fWildMikeDanceTimer -= a_fDeltaTime;

			TBOOL bWalking = m_pWildMike->IsAnimPlaying( TPS8( walk_lp ), 0.0f );

			if ( ( m_fWildMikeTime < 0.0f || m_eEndState != ENDSTATE_NONE ) && bWalking )
			{
				WildMikeLeave();
				return;
			}

			TBOOL bJumpBack = TTRUE;

			if ( m_fWildMikeDanceTimer < 0.0f )
			{
				TFLOAT fDanceTimer;

				if ( m_eWildMikeState != WILDMIKESTATE_DANCING )
				{
					m_vWildMikeDirection = TVector4::VEC_POSZ;
					m_pWildMike->GetModelInstance()->SetDirection( m_vWildMikeDirection );

					m_eWildMikeState = WILDMIKESTATE_DANCING;
					m_pWildMike->PlayAnimation( TPS8( dance2 ) );

					fDanceTimer           = TFLOAT( rRandom.GetInt( 1 ) + 1 ) * 0.67f;
					m_fWildMikeDanceTimer = fDanceTimer;
				}
				else if ( !m_pWildMike->IsAnimPlaying( TPS8( dance2 ), 0.0f ) || m_pWildMike->GetAnimationProgress( TPS8( dance2 ) ) > 0.9f )
				{
					m_pWildMike->PlayAnimation( TPS8( walk_lp ) );
					m_eWildMikeState      = WILDMIKESTATE_WALKING;
					m_fWildMikeDanceTimer = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_DANCE_TIME_MIN ], m_afParams[ FLOATPARAM_WILDMIKE_DANCE_TIME_MAX ] );
				}

				m_fWildMikeSpinTimer = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_SPIN_TIME_MIN ], m_afParams[ FLOATPARAM_WILDMIKE_SPIN_TIME_MAX ] );
				bWalking             = TFALSE;
			}

			if ( m_fWildMikeSpinTimer < 0.0f && bWalking )
			{
				m_pWildMike->PlayAnimation( TPS8( dance3 ) );
				m_eWildMikeState = WILDMIKESTATE_SPINNING;

				TFLOAT fSpinTimer    = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_SPIN_TIME_MIN ], m_afParams[ FLOATPARAM_WILDMIKE_SPIN_TIME_MAX ] );
				m_fWildMikeSpinTimer = fSpinTimer;
				m_fWildMikeDanceTimer += 0.5f;
				m_fWildMikeJumpTimer = fSpinTimer * 0.2f + m_fWildMikeJumpTimer;
			}
			else if ( m_fWildMikeJumpTimer < 0.0f && bWalking )
			{
				// Jump to the next table
				TINT iNextTable;

				if ( m_iWildMikeTable != 0 )
				{
					if ( m_iWildMikeTable < GetLevel().iNumBars - 1 )
					{
						TINT iRandom = rRandom.GetInt( 1 );
						bJumpBack    = iRandom == 0;
						iNextTable   = m_iWildMikeTable + 2 * ( iRandom != 0 ) - 1;
					}
					else
					{
						iNextTable = m_iWildMikeTable - 1;
					}
				}
				else
				{
					iNextTable = 1;
					bJumpBack  = TFALSE;
				}

				TVector4 vPos;
				m_pWildMike->GetModelInstance()->GetPosition( vPos );

				m_vWildMikeJumpTarget = vPos + m_avWildMikeStart[ iNextTable ] - m_avWildMikeStart[ m_iWildMikeTable ];

				if ( bJumpBack )
				{
					m_vWildMikeOnTableDirection = TVector4::VEC_NEGZ;
					m_eWildMikeState            = WILDMIKESTATE_JUMPING_BACK;
				}
				else
				{
					m_vWildMikeOnTableDirection = TVector4::VEC_POSZ;
					m_eWildMikeState            = WILDMIKESTATE_JUMPING_FORWARD;
				}

				m_fWildMikeJumpTimer = rRandom.GetFloatMinMax( 0.2f, m_afParams[ FLOATPARAM_WILDMIKE_JUMP_TIME ] );
				m_vWildMikeJumpStart = vPos;
				m_bWildMikeJumping   = TFALSE;

				m_pWildMike->GetModelInstance()->GetUpVector( m_vWildMikeDirection );
				m_vWildMikeDirection.x = -m_vWildMikeDirection.x;
				m_vWildMikeDirection.y = -m_vWildMikeDirection.y;
				m_vWildMikeDirection.z = -m_vWildMikeDirection.z;

				m_iWildMikeTable = iNextTable;
			}
			else
			{
				// Walk along the table
				TVector4 vPos;
				m_pWildMike->GetModelInstance()->GetPosition( vPos );

				TVector4 vTarget;
				TVector4 vFrom;

				if ( m_iWildMikeWalkDirection == 1 )
				{
					vTarget = m_avWildMikeEnd[ m_iWildMikeTable ];
					vFrom   = m_avWildMikeStart[ m_iWildMikeTable ];
				}
				else
				{
					vTarget = m_avWildMikeStart[ m_iWildMikeTable ];
					vFrom   = m_avWildMikeEnd[ m_iWildMikeTable ];
				}

				TVector4 vToTarget;
				vToTarget.Substract( vTarget, vPos );

				if ( vToTarget.NormaliseMag() >= 0.4f )
				{
					vPos.Add( ( vToTarget * a_fDeltaTime * m_afParams[ FLOATPARAM_WILDMIKE_WALK_SPEED ] ).AsVector3() );

					TVector4 vSide;
					vSide.Set( vToTarget );
					vSide.AsVector3().RotateY( TMath::PI * 0.5f );

					TFLOAT fSwayDirection = ( m_fWildMikeSway <= 0.0f ) ? -1.0f : 1.0f;
					vPos.Add( ( vSide * ( fSwayDirection * m_afParams[ FLOATPARAM_WILDMIKE_SWAY_SPEED ] * a_fDeltaTime ) ).AsVector3() );

					TFLOAT fT;

					if ( TMath::DistancePointToLineSq( vPos, vFrom, vTarget, fT ) > m_fWildMikeSway * m_fWildMikeSway )
					{
						// Swayed too far, clamp to the sway distance and pick a new one
						TVector4 vSway = vSide * m_fWildMikeSway;
						vPos           = ( vTarget - vFrom ) * fT + vFrom + vSway;

						m_fWildMikeSwayTimer = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME ] - m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE ], m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME ] + m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE ] );
						m_fWildMikeSway      = rRandom.GetFloatMinMax( -m_afParams[ FLOATPARAM_WILDMIKE_SWAY ], m_afParams[ FLOATPARAM_WILDMIKE_SWAY ] );
					}

					m_pWildMike->GetModelInstance()->SetTranslation( vPos.AsVector3() );

					m_fWildMikeWalkTimer -= a_fDeltaTime;

					if ( m_fWildMikeWalkTimer < 0.0f )
					{
						m_fWildMikeWalkTimer     = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME ], m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME_RANGE ] + m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME ] );
						m_iWildMikeWalkDirection = 2 * ( m_iWildMikeWalkDirection != 1 ) - 1;
					}

					m_fWildMikeSwayTimer -= a_fDeltaTime;

					if ( m_fWildMikeSwayTimer < 0.0f )
					{
						m_fWildMikeSwayTimer = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME ] - m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE ], m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME ] + m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE ] );
						m_fWildMikeSway      = rRandom.GetFloatMinMax( -m_afParams[ FLOATPARAM_WILDMIKE_SWAY ], m_afParams[ FLOATPARAM_WILDMIKE_SWAY ] );
					}
				}
				else
				{
					// Reached the end of the table, turn around
					m_iWildMikeWalkDirection = 2 * ( m_iWildMikeWalkDirection != 1 ) - 1;
					m_fWildMikeWalkTimer     = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME ], m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME_RANGE ] + m_afParams[ FLOATPARAM_WILDMIKE_WALK_TIME ] );
					m_fWildMikeSwayTimer     = rRandom.GetFloatMinMax( m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME ] - m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE ], m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME ] + m_afParams[ FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE ] );
				}

				if ( m_eWildMikeState != WILDMIKESTATE_DANCING )
				{
					// Turn to the next random direction
					m_fWildMikeTurn = TMath::Min( a_fDeltaTime * m_afParams[ FLOATPARAM_WILDMIKE_TURN_SPEED ] + m_fWildMikeTurn, 1.0f );

					TVector4 vDirection;
					vDirection.Lerp4( m_vWildMikeDirection, m_vWildMikeOnTableDirection, m_fWildMikeTurn );
					vDirection.Normalise();

					if ( m_fWildMikeTurn == 1.0f )
					{
						m_vWildMikeOnTableDirection.x = rRandom.GetFloatMinMax( -1.0f, 1.0f );
						m_vWildMikeOnTableDirection.z = rRandom.GetFloatMinMax( -1.0f, 1.0f );
						m_vWildMikeOnTableDirection.y = 0.0f;
						m_vWildMikeOnTableDirection.Normalise();
						m_vWildMikeDirection = vDirection;
						m_fWildMikeTurn      = 0.0f;
					}

					m_pWildMike->GetModelInstance()->SetDirection( vDirection );
				}
			}

			return;
		}
		case WILDMIKESTATE_SPINNING:
		{
			if ( m_pWildMike->IsAnimationFinished( TPS8( dance3 ) ) )
				m_eWildMikeState = WILDMIKESTATE_WALKING;

			return;
		}
		case WILDMIKESTATE_JUMPING_FORWARD:
		case WILDMIKESTATE_JUMPING_BACK:
		{
			TVector4 vDirection;
			m_pWildMike->GetModelInstance()->GetUpVector( vDirection );
			vDirection.x = -vDirection.x;
			vDirection.w = 1.0f;
			vDirection.y = -vDirection.y;
			vDirection.z = -vDirection.z;

			if ( m_vWildMikeOnTableDirection.IsEqual( vDirection ) )
			{
				m_pWildMike->GetModelInstance()->SetDirection( m_vWildMikeOnTableDirection );

				if ( !m_bWildMikeJumping )
				{
					m_bWildMikeJumping = TTRUE;
					m_pWildMike->PlayAnimation( TPS8( jump ) );
				}

				if ( m_pWildMike->IsAnimPlaying( TPS8( jump ), 0.0f ) )
				{
					TVector4 vPos;
					vPos.Set( m_vWildMikeJumpStart );
					vPos.Lerp4( vPos, m_vWildMikeJumpTarget, m_pWildMike->GetAnimationProgress( TPS8( jump ) ) );
					m_pWildMike->GetModelInstance()->SetTranslation( vPos.AsVector3() );
				}
				else if ( m_pWildMike->IsAnimationFinished( TPS8( jump ) ) )
				{
					m_eWildMikeState = WILDMIKESTATE_WALKING;
					m_pWildMike->PlayAnimation( TPS8( walk_lp ) );
					m_fWildMikeSpinTimer += 0.5f;
					m_fWildMikeDanceTimer += 0.5f;
				}
			}
			else
			{
				// Turn to the next table first
				m_fWildMikeTurn = TMath::Min( a_fDeltaTime * m_afParams[ FLOATPARAM_WILDMIKE_JUMP_TURN_SPEED ] + m_fWildMikeTurn, 1.0f );

				TVector4 vNewDirection;
				vNewDirection.Lerp4( m_vWildMikeDirection, m_vWildMikeOnTableDirection, m_fWildMikeTurn );
				vNewDirection.Normalise();
				m_pWildMike->GetModelInstance()->SetDirection( vNewDirection );
			}

			return;
		}
		case WILDMIKESTATE_LEAVING:
		{
			TVector4 vDirection;
			m_pWildMike->GetModelInstance()->GetUpVector( vDirection );
			vDirection.x = -vDirection.x;
			vDirection.w = 1.0f;
			vDirection.y = -vDirection.y;
			vDirection.z = -vDirection.z;

			if ( m_vWildMikeOnTableDirection.IsEqual( vDirection ) )
			{
				if ( m_pWildMike->IsAnimationFinished( TPS8( getofftable ) ) ||
				     ( m_pWildMike->IsAnimPlaying( TPS8( getofftable ), 0.0f ) && m_pWildMike->GetAnimationProgress( TPS8( getofftable ) ) > 0.95f ) )
				{
					m_pWildMike->KillAllAnimations();
					m_pWildMike->SetSkeletonUpdating( TFALSE, TTRUE );
					m_pWildMike->SetVisible( TFALSE );
					m_eWildMikeState = WILDMIKESTATE_NONE;

					// NOTE: the results of these are not used
					for ( TINT i = 0; i < NUM_BARS; i++ )
					{
						for ( auto it = m_aBars[ i ].llActive.Begin(); it != m_aBars[ i ].llActive.End(); )
						{
							Drinker* pDrinker = it;
							it++;

							pDrinker->pUnit->IsVisible();
						}
					}
				}
			}
			else
			{
				m_fWildMikeTurn = TMath::Min( a_fDeltaTime * m_afParams[ FLOATPARAM_WILDMIKE_JUMP_TURN_SPEED ] + m_fWildMikeTurn, 1.0f );

				TVector4 vNewDirection;
				vNewDirection.Lerp4( m_vWildMikeDirection, m_vWildMikeOnTableDirection, m_fWildMikeTurn );
				vNewDirection.Normalise();
				m_pWildMike->GetModelInstance()->SetDirection( vNewDirection );

				if ( m_vWildMikeOnTableDirection.IsEqual( vNewDirection ) )
					m_pWildMike->PlayAnimation( TPS8( getofftable ) );
			}

			return;
		}
	}
}
// $Barnyard: FUNCTION 004e53d0
TBOOL ATapperMiniGameState::DrinkerCatchGlass( Drinker* a_pDrinker, Glass* a_pGlass )
{
	switch ( a_pDrinker->eState )
	{
		case DRINKERSTATE_11:
		case DRINKERSTATE_10:
		case DRINKERSTATE_DRINKING:
			return TFALSE;
	}

	AAnimatableObject* pDrinkerObject = a_pDrinker->pUnit->GetAnimatableObject();

	if ( pDrinkerObject->IsAnimPlaying( TPS8( throwback ), 0.0f ) || a_pDrinker->pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( throwback ) ) )
		return TFALSE;

	if ( !IsDrinkerBehindGlassLine( a_pDrinker, 0.0f ) )
		return TFALSE;

	if ( a_pDrinker->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) ) )
		return TFALSE;

	ASoundManager::GetSingleton()->StopCue( a_pGlass->iCue );
	a_pGlass->iCue = -1;

	a_pGlass->pFull->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );
	a_pGlass->pEmpty->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );

	AAnimatableObjectManager::GetSingleton()->AttachObject( a_pGlass->pFull, a_pDrinker->pUnit->GetAnimatableObject(), TPString8(), TPS8( cb_llocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
	a_pGlass->pFull->SetSkeletonUpdating( TTRUE, TTRUE );
	a_pGlass->pFull->SetVisible( TTRUE );
	a_pGlass->eState = GLASSSTATE_DRINKING;

	return TTRUE;
}

// $Barnyard: FUNCTION 004e55b0
void ATapperMiniGameState::TakeGlassFromPool()
{
	DetachCarriedGlass();

	AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();

	if ( pPlayerObject->IsAnimPlaying( TPS8( throw ), 0.0f ) || m_pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( throw ) ) || IsHoldingFullGlass() )
		return;

	if ( m_iGlassPool >= 0 && m_aGlassPools[ m_iGlassPool ].iNumGlasses <= 0 )
		SelectGlassPool();

	if ( m_iGlassPool < 0 || m_aGlassPools[ m_iGlassPool ].llFree.IsEmpty() )
		return;

	GlassPool& rPool = m_aGlassPools[ m_iGlassPool ];
	rPool.iNumGlasses--;

	Glass* pGlass = rPool.llFree.PopFront();
	rPool.llUsed.PushBack( pGlass );

	TQuaternion quatRotation = TQuaternion::IDENTITY;

	pGlass->pFull->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );
	pGlass->pEmpty->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );
	pGlass->pEmpty->SetSkeletonUpdating( TFALSE, TTRUE );

	AAnimatableObjectManager::GetSingleton()->AttachObject( pGlass->pFull, m_pPlayer->GetAnimatableObject(), TPString8(), TPS8( cb_rlocator ), TVector4::VEC_ZERO, quatRotation, 1.0f );
	pGlass->pFull->SetSkeletonUpdating( TTRUE, TTRUE );
	pGlass->pFull->SetVisible( TTRUE );
	pGlass->eState = GLASSSTATE_HELD;
}

// $Barnyard: FUNCTION 004e57c0
ATapperMiniGameState::Glass* ATapperMiniGameState::DetachDrinkerGlass( Drinker* a_pDrinker )
{
	AAnimatableObject* pGlassObject = a_pDrinker->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) );

	if ( !pGlassObject )
		return TNULL;

	AAnimatableObjectManager::GetSingleton()->DetachObject( pGlassObject, a_pDrinker->pUnit->GetAnimatableObject() );

	Glass* pGlass  = FindGlass( pGlassObject );
	pGlass->eState = GLASSSTATE_FULL;

	return pGlass;
}

// $Barnyard: FUNCTION 004e5830
void ATapperMiniGameState::DrinkerThrowBackGlass( Drinker* a_pDrinker )
{
	a_pDrinker->eState = DRINKERSTATE_THROWBACK;

	AAnimatableObject* pGlassObject = a_pDrinker->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) );

	if ( !pGlassObject )
		return;

	Glass* pGlass = FindGlass( pGlassObject );
	AAnimatableObjectManager::GetSingleton()->DetachObject( pGlass->pEmpty, a_pDrinker->pUnit->GetAnimatableObject() );

	TVector4 vPos;
	TVector4 vDirection;
	GetDrinkerGlassPos( a_pDrinker, vPos, vDirection );

	pGlass->vDirection = vDirection;
	pGlass->vVelocity  = ( vDirection * m_afParams[ FLOATPARAM_EMPTY_GLASS_SPEED ] ) * m_afParams[ FLOATPARAM_EMPTY_GLASS_SPEED_BAR1 + a_pDrinker->iLane ];
	pGlass->eState     = GLASSSTATE_EMPTY;

	pGlass->pFull->GetModelInstance()->SetTranslation( vPos.AsVector3() );
	pGlass->pEmpty->GetModelInstance()->SetTranslation( vPos.AsVector3() );
	pGlass->pEmpty->SetSkeletonUpdating( TTRUE, TTRUE );
	pGlass->pFull->SetSkeletonUpdating( TFALSE, TTRUE );

	pGlass->iCue = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::TAPPA_SLIDE, vPos );
}

// $Barnyard: FUNCTION 004e5a20
void ATapperMiniGameState::OnAnimEvent( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	switch ( a_pEvent->eAction )
	{
		case AAnimationBreakpointUnitAction::ACTION_HD_THROW:
		{
			TRef<AUnitProxy> pProxy = a_pEvent->pUnitProxy;
			AUnit*           pUnit  = pProxy ? pProxy->GetUnit() : TNULL;
			pProxy                  = TNULL;

			if ( pUnit == m_pPlayer )
			{
				// The player has thrown the glass
				if ( m_iGlassPool < 0 || m_aGlassPools[ m_iGlassPool ].llUsed.IsEmpty() )
					return;

				Glass* pGlass = m_aGlassPools[ m_iGlassPool ].llUsed.Back();
				AAnimatableObjectManager::GetSingleton()->DetachObject( pGlass->pFull, m_pPlayer->GetAnimatableObject() );

				TVector4 vPos;
				TVector4 vDirection;
				GetPlayerGlassStart( vPos, vDirection );

				TFLOAT fMinSpeed = m_afParams[ FLOATPARAM_FILL_MIN ];
				TFLOAT fMaxSpeed = m_afParams[ FLOATPARAM_FILL_MAX ];
				m_fFill          = ( fMaxSpeed - fMinSpeed ) * ( g_oSystemManager.GetScheduler()->GetTotalTime() - m_fFillStartTime - 0.2f ) * m_afParams[ FLOATPARAM_FILL_SPEED ] + fMinSpeed;
				TMath::Clip( m_fFill, fMinSpeed, fMaxSpeed );

				pGlass->vDirection = vDirection;
				pGlass->vVelocity  = vDirection * m_fFill;
				pGlass->iBar       = m_iBar;

				TINT iFill = TINT( ( m_fFill - m_afParams[ FLOATPARAM_FILL_MIN ] ) / ( m_afParams[ FLOATPARAM_FILL_MAX ] - m_afParams[ FLOATPARAM_FILL_MIN ] ) * 4.0f );

				if ( iFill >= 4 )
					iFill = 3;

				pGlass->iScore = m_aiParams[ INTPARAM_GLASS_SCORE_1 + iFill ];
				pGlass->eState = GLASSSTATE_FULL;
				pGlass->pFull->GetModelInstance()->SetTranslation( vPos.AsVector3() );
				pGlass->fDripTimer = 0.0f;

				m_ePlayerState = PLAYERSTATE_GLASS_THROWN;
				pGlass->iCue   = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::TAPPA_SLIDE, vPos );
			}
			else
			{
				Drinker* pDrinker = FindDrinker( pUnit );

				if ( pDrinker && pDrinker->eState != DRINKERSTATE_RUNAWAY )
					DrinkerThrowBackGlass( pDrinker );
			}

			break;
		}
		case AAnimationBreakpointUnitAction::ACTION_END:
			OnCurtainClosed();
			break;
		case AAnimationBreakpointUnitAction::ACTION_PUTDOWN:
		{
			TRef<AUnitProxy> pProxy = a_pEvent->pUnitProxy;
			AUnit*           pUnit  = pProxy ? pProxy->GetUnit() : TNULL;
			pProxy                  = TNULL;

			Drinker* pDrinker = FindDrinker( pUnit );

			if ( !pDrinker || pDrinker->eState == DRINKERSTATE_RUNAWAY )
				break;

			// The drinker has emptied the glass, swap the full glass for the empty one
			AAnimatableObject* pGlassObject = pDrinker->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) );

			if ( !pGlassObject )
				break;

			Glass*             pGlass    = FindGlass( pGlassObject );
			AAnimatableObject* pAttached = pDrinker->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) );
			AAnimatableObjectManager::GetSingleton()->DetachObject( pAttached, pDrinker->pUnit->GetAnimatableObject() );

			TVector4 vPos;
			TVector4 vDirection;
			GetDrinkerGlassPos( pDrinker, vPos, vDirection );

			pGlass->pFull->GetModelInstance()->SetTranslation( vPos.AsVector3() );
			pGlass->pEmpty->GetModelInstance()->SetTranslation( vPos.AsVector3() );
			pGlass->pEmpty->SetSkeletonUpdating( TTRUE, TTRUE );
			pGlass->pFull->SetSkeletonUpdating( TFALSE, TTRUE );

			AAnimatableObjectManager::GetSingleton()->AttachObject( pGlass->pEmpty, pDrinker->pUnit->GetAnimatableObject(), TPString8(), TPS8( cb_llocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
			break;
		}
	}
}

// $Barnyard: FUNCTION 004e5e80
void ATapperMiniGameState::SetDrinkerWalking( Drinker* a_pDrinker )
{
	if ( a_pDrinker->eMovement == MOVEMENT_WALK )
		return;

	a_pDrinker->eMovement = MOVEMENT_WALK;

	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, a_pDrinker->pUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) ) );

	TFLOAT fSpeed = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, m_afParams[ FLOATPARAM_WALK_SPEED_RANDOM ] ) +
	    m_afParams[ FLOATPARAM_WALK_SPEED_BAR1 + a_pDrinker->iLane ] * GetLevel().fWalkSpeed * m_fSpeedScale;

	a_pDrinker->fSpeed = fSpeed;
	pMovement->SetWalkingSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, fSpeed );

	AAnimatableObject*    pObject = a_pDrinker->pUnit->GetAnimatableObject();
	ANamedAnimationSetRef pSet    = pObject->GetObjectType()->FindAnimationSetOrDefault( TPS8( default ) );

	if ( a_pDrinker->pUnit->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet().Get() != pSet.Get() )
		a_pDrinker->pUnit->GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( pSet );
}

// $Barnyard: FUNCTION 004e6000
TVector4& ATapperMiniGameState::GetRandomGoalPosition( TVector4& a_rOutPos, TINT a_iBar, const TCHAR* a_szPrefix )
{
	TCHAR szName[ 100 ];
	TStringManager::String8Format( szName, sizeof( szName ), "%s%d", a_szPrefix, a_iBar );

	AInteriorGoal* pGoal = m_pNetwork->FindRandomGoal( TPString8( szName ) );

	TVector4 vPos;
	pGoal->GetPosition( vPos );
	a_rOutPos = vPos;

	return a_rOutPos;
}

// $Barnyard: FUNCTION 004e6100
void ATapperMiniGameState::StartPouring()
{
	AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();

	if ( pPlayerObject->IsAnimPlaying( TPS8( throw ), 0.0f ) || m_pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( throw ) ) )
		return;

	CancelGlass();

	while ( --m_iGlassPool >= 0 )
	{
		if ( m_aGlassPools[ m_iGlassPool ].iNumGlasses != 0 )
		{
			TakeGlassFromPool();
			return;
		}
	}

	SelectGlassPool();
	TakeGlassFromPool();
}

// $Barnyard: FUNCTION 004e61a0
void ATapperMiniGameState::AddServedScore( Drinker* a_pDrinker, TINT a_iScore )
{
	TVector4 vPos;
	a_pDrinker->pUnit->GetAnimatableObject()->GetModelInstance()->GetPosition( vPos );
	vPos.y -= 2.0f;

	TVector4 vOffset;
	vOffset.Multiply( m_vTapDirection, 0.6f );

	vPos.x -= vOffset.x;
	vPos.y -= vOffset.y;
	vPos.z -= vOffset.z;

	AddScoreFloat( vPos, a_iScore );

	GetCurrentPlayer().iMilkServed += a_iScore;
	AddScore( GetCurrentPlayer(), a_iScore );
}

// $Barnyard: FUNCTION 004e62d0
void ATapperMiniGameState::AddGlassScore( Glass* a_pGlass, TINT a_iScore )
{
	TVector4 vPos;
	a_pGlass->pFull->GetModelInstance()->GetPosition( vPos );
	vPos.y -= 0.3f;

	TVector4 vOffset;
	vOffset.Multiply( m_vTapDirection, 0.6f );

	vPos.x -= vOffset.x;
	vPos.y -= vOffset.y;
	vPos.z -= vOffset.z;

	if ( a_iScore < 0 )
		GetCurrentPlayer().iGlassesDropped += a_iScore;

	AddScoreFloat( vPos, a_iScore );
	AddScore( GetCurrentPlayer(), a_iScore );
}

// $Barnyard: FUNCTION 004e6400
TBOOL ATapperMiniGameState::SwitchBar( TINT a_iDirection )
{
	if ( m_fSwitchBarTimer > 0.0f )
		return TFALSE;

	if ( m_ePlayerState == PLAYERSTATE_FILLING || m_ePlayerState == PLAYERSTATE_2 || m_ePlayerState == PLAYERSTATE_THROWING )
		return TFALSE;

	m_fSwitchBarTimer = m_afParams[ FLOATPARAM_SWITCH_BAR_TIME ];

	if ( m_bRunning == TTRUE )
		m_vPlayerPos = GetBarTap( m_iBar );

	TINT iNumBars   = GetLevel().iNumBars;
	m_fRunDirection = 0.0f;
	m_bRunning      = TFALSE;

	if ( GetDistanceSqToTarget( m_iBar ) > 0.1f )
		return TTRUE;

	m_iBar += a_iDirection;

	// NOTE: the result isn't used
	TVector4 vUnused = m_avBarTaps[ 0 ] - m_avBarTaps[ 1 ];
	vUnused.Normalise();

	if ( m_iBar > iNumBars || m_iBar < 1 )
	{
		m_iBar = ( m_iBar > iNumBars ) ? iNumBars : 1;

		m_fSwitchBarTimer = 0.0f;

		if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( run_lp ), 0.0f ) || m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp2 ), 0.0f ) )
		{
			if ( !m_pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( idle_lp ) ) )
				PlayPlayerIdle();
		}

		return TTRUE;
	}

	m_pPlayer->GetAnimatableObject()->KillAllAnimations();

	if ( a_iDirection <= 0 )
	{
		m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( jump_left ) );
		PlayJumpDust();
	}
	else
	{
		PlayJumpDust();
		m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( jump_right ) );
	}

	m_iJumpDirection = a_iDirection;
	return TTRUE;
}

// $Barnyard: FUNCTION 004e6600
TBOOL ATapperMiniGameState::StartFilling()
{
	TVector4 vTarget;
	GetPlayerTargetPos( vTarget, m_iBar );

	TVector4 vToTarget = vTarget - m_vPlayerPos;

	if ( m_bRunning || vToTarget.x != 0.0f || vToTarget.y != 0.0f || vToTarget.z != 0.0f )
		return TFALSE;

	m_fDripTimer   = 0.0f;
	m_ePlayerState = PLAYERSTATE_FILLING;
	m_pPlayer->GetAnimatableObject()->KillAllAnimations();
	m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( fillup ) );
	m_bTowardsEnd = TFALSE;
	TakeGlassFromPool();

	m_vPlayerPos = GetBarTap( m_iBar );
	m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );
	FacePlayer( TMath::PI * 0.5f );

	m_fFillStartTime = g_oSystemManager.GetScheduler()->GetTotalTime();
	m_oMilkFill.SetFill( 0.0f, 1.0f );
	m_oMilkFill.Show();

	// Place the glass meter next to the tap
	TVector4 vPos = m_vPlayerPos;

	TVector4 vBarDirection = GetBarEnd( m_iBar ) - GetBarTap( m_iBar );
	vBarDirection.Normalise();

	vPos.x = vBarDirection.x * m_afParams[ FLOATPARAM_FILL_HUD_OFFSET ] + vPos.x;
	vPos.z = vBarDirection.z * m_afParams[ FLOATPARAM_FILL_HUD_OFFSET ] + vPos.z;
	vPos.y = vPos.y + m_afParams[ FLOATPARAM_FILL_HUD_HEIGHT ];
	ARenderer::GetSingleton()->WorldToGUI( vPos );

	m_oMilkFill.SetTransform( vPos.x, vPos.y + m_afParams[ FLOATPARAM_FILL_HUD_HEIGHT ] );

	m_bRunning      = TFALSE;
	m_fRunDirection = 0.0f;
	m_iFillCue      = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::TAPPA_POUR, m_vPlayerPos );

	StopRumble( m_iRumble, AInputHandler::GetSingleton()->GetMainController() );

	AInputHandler::RumbleParams oRumble = { { m_afParams[ FLOATPARAM_FILL_RUMBLE ], 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f }, 1 };
	m_iRumble                           = AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );

	return TTRUE;
}

// $Barnyard: FUNCTION 004e6960
ATapperMiniGameState::Tip* ATapperMiniGameState::SpawnTip( const TVector4& a_rcPosition )
{
	TINT i;

	for ( i = 0; i < NUM_BARS; i++ )
	{
		if ( m_aTips[ i ].eState == TIPSTATE_NONE && !m_aTips[ i ].pObject->GetModelInstance()->IsUpdatingSkeleton() )
			break;
	}

	if ( i >= NUM_BARS )
		return TNULL;

	Tip& rTip = m_aTips[ i ];
	rTip.pObject->GetModelInstance()->GetSceneObject()->DisableOpacity();
	rTip.pObject->GetModelInstance()->SetSkeletonUpdating( TTRUE );

	TMatrix44 matTransform                                 = TMatrix44::IDENTITY;
	matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rcPosition;
	matTransform.RotateX( TMath::PI * 0.5f );
	rTip.pObject->GetModelInstance()->SetTransform( matTransform );

	rTip.eState = TIPSTATE_ACTIVE;
	return &rTip;
}

// $Barnyard: FUNCTION 004e6b30
void ATapperMiniGameState::LoseGame( Drinker* a_pDrinker )
{
	m_oWildMikeIcon.SetShown( TFALSE );

	if ( m_eEndState == ENDSTATE_LOST )
		return;

	APositionalSoundSet::GetSingleton()->SetPaused( TTRUE );

	if ( a_pDrinker )
		a_pDrinker->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( sulk ) );

	m_oMilkFill.Hide();
	WildMikeLeave();

	m_eEndState = ENDSTATE_LOST;
	FacePlayer( 0.0f );

	m_pPlayer->GetAnimatableObject()->KillAllAnimations();
	m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( sulk ) );
	CancelGlass();
	m_pRoot->PlayAnimation( TPS8( sulk ) );

	m_fEndTimer     = m_afParams[ FLOATPARAM_END_TIME ];
	m_bRunning      = TFALSE;
	m_fRunDirection = 0.0f;

	AddLives( GetCurrentPlayer(), -1000 );
}

// $Barnyard: FUNCTION 004e6c30
TVector4& ATapperMiniGameState::GetClosestGlassPosition( TVector4& a_rOutPos )
{
	Glass* pGlass = FindClosestGlass( m_vPlayerPos, 10.0f );

	if ( pGlass )
		pGlass->pFull->GetModelInstance()->GetPosition( a_rOutPos );
	else
		a_rOutPos = TVector4::VEC_ZERO;

	return a_rOutPos;
}

// $Barnyard: FUNCTION 004e6d10
void ATapperMiniGameState::UpdatePlayerLook( TFLOAT a_fDeltaTime )
{
	m_fLookTimer += a_fDeltaTime;

	AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, m_pPlayer->FindComponent( &TGetClass( AUnitFacialComponent ) ) );

	if ( !pFacial )
		return;

	if ( m_fLookTimer <= 2.0f )
	{
		pFacial->StopLook();
	}
	else if ( !pFacial->IsLooking() )
	{
		Drinker* pDrinker = FindClosestDrinker( m_vPlayerPos, 3.0f, TNULL );

		if ( pDrinker && pDrinker->pUnit )
		{
			pFacial->LookAtUnit( pDrinker->pUnit, TVector4::VEC_NEGY * 2.0f, 3.0f );
		}
		else
		{
			TVector4 vGlassPos;
			GetClosestGlassPosition( vGlassPos );

			if ( !TVector4::VEC_ZERO.IsEqual( vGlassPos ) )
				pFacial->LookAtPosition( vGlassPos, 2.0f );
		}
	}
}

// $Barnyard: FUNCTION 004e6df0
void __stdcall ATapperMiniGameState::OnAnimEventCallback( ATapperMiniGameState* a_pCaller, AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	a_pCaller->OnAnimEvent( a_pEvent );
}
// $Barnyard: FUNCTION 004e6fd0
TBOOL ATapperMiniGameState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	// Can't pause while the curtain is moving
	if ( ( a_eInputCommand == AInputCommand_Quit || a_eInputCommand == AInputCommand_Start ) && !m_pCurtain->IsAnimPlaying( TPS8( opened_idle ), 0.0f ) )
	{
		if ( m_pCurtain->GetModelInstance().Get() && m_pCurtain->GetModelInstance()->IsUpdatingSkeleton() )
			return TTRUE;
	}

	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return TFALSE; // RETURNS_FALSE 006528c0

	m_oCommandQueue.Process( this );

	TFLOAT fLookTimer = m_fLookTimer;
	m_fLookTimer      = 0.0f;
	TBOOL bHandled    = TFALSE;

	if ( APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_MINIGAME )->GetPlayerSlotIndexByDevice( a_pInputEvent->GetSource() ) != m_iCurrentPlayer )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( m_oStartHudElement.IsForceHidden() == TFALSE )
	{
		m_oCommandQueue.Reset();
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}

	if ( m_eEndState != ENDSTATE_NONE )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	switch ( a_eInputCommand )
	{
		case AInputCommand_Move:
		{
			if ( m_ePlayerState == PLAYERSTATE_FILLING || m_ePlayerState == PLAYERSTATE_2 || m_ePlayerState == PLAYERSTATE_THROWING )
				break;

			TFLOAT fVertical = a_pInputEvent->GetMagnitudeFloat( 1 );

			if ( TMath::Abs( fVertical ) <= 0.2f )
			{
				m_bStickSwitchedBar = TFALSE;
			}
			else if ( !m_bRunning )
			{
				if ( m_ePlayerState == PLAYERSTATE_FILLING || m_ePlayerState == PLAYERSTATE_2 || m_ePlayerState == PLAYERSTATE_THROWING )
				{
					AInputCommand eCommand = ( fVertical <= 0.0f ) ? AInputCommand_ForwardWithUp : AInputCommand_BackwardWithUp;
					m_oCommandQueue.Push( m_bStickSwitchedBar, bHandled, eCommand, a_pInputEvent->GetEventType() );
					m_bStickSwitchedBar = TTRUE;
				}
				else
				{
					AInputCommand eCommand   = ( fVertical <= 0.0f ) ? AInputCommand_ForwardWithUp : AInputCommand_BackwardWithUp;
					TINT          iDirection = ( fVertical <= 0.0f ) ? 1 : -1;

					if ( SwitchBar( iDirection ) || m_bStickSwitchedBar )
						m_oCommandQueue.Push( TTRUE, bHandled, eCommand, a_pInputEvent->GetEventType() );
					else
						m_oCommandQueue.Push( TFALSE, bHandled, eCommand, a_pInputEvent->GetEventType() );

					m_bStickSwitchedBar = TTRUE;
				}
			}

			if ( !IsPlayerAtTarget() && m_bRunning != TTRUE )
			{
				m_fLookTimer = fLookTimer;
				return TTRUE;
			}

			TFLOAT fHorizontal = a_pInputEvent->GetMagnitudeFloat( 0 );

			if ( fHorizontal > 0.2f )
			{
				// Back to the tap
				if ( m_bRunning != TTRUE )
					return TTRUE;

				m_oCommandQueue.Reset();
				m_bRunning = TTRUE;
				DetachCarriedGlass();
				m_bTowardsEnd   = TFALSE;
				m_fRunDirection = -0.1f;
			}
			else if ( fHorizontal < -0.2f )
			{
				// Along the bar
				m_oCommandQueue.Reset();
				m_bRunning = TTRUE;
				DetachCarriedGlass();
				m_bTowardsEnd   = TTRUE;
				m_fRunDirection = 0.1f;
			}
			else
			{
				if ( m_bRunKeyHeld || m_bRunning != TTRUE )
					return TTRUE;

				m_fRunDirection = 0.0f;
				m_fLookTimer    = fLookTimer;
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp2 ) );
				return TTRUE;
			}

			m_bRunKeyHeld = TFALSE;

			if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( run_lp ), 0.0f ) )
				return TTRUE;

			m_pPlayer->GetAnimatableObject()->KillAllAnimations();
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
			return TTRUE;
		}
		case AInputCommand_Up:
		case AInputCommand_Down:
		case AInputCommand_LeftWithUp:
		case AInputCommand_RightWithUp:
		case AInputCommand_ForwardWithUp:
		case AInputCommand_BackwardWithUp:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			break;
		case AInputCommand_Shoot:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				if ( m_iShootDoodad != 0 )
					return TTRUE;

				m_iShootDoodad = a_pInputEvent->GetDoodad();
			}
			else if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				if ( a_pInputEvent->GetDoodad() != m_iShootDoodad )
					return TTRUE;

				m_iShootDoodad = 0;
			}

			break;
	}

	if ( !HandleCommand( a_eInputCommand, a_pInputEvent->GetEventType(), bHandled ) )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	return TTRUE;
}

// $Barnyard: FUNCTION 004e7440
TBOOL ATapperMiniGameState::HandleCommand( AInputCommand a_eCommand, TINT a_iEventType, TBOOL& a_rbHandled )
{
	if ( m_oStartHudElement.IsForceHidden() == TFALSE )
	{
		m_oCommandQueue.Reset();
		return TFALSE;
	}

	if ( m_eEndState != ENDSTATE_NONE )
	{
		// Shoot skips the end of the level
		if ( a_eCommand == AInputCommand_Shoot && m_afParams[ FLOATPARAM_END_TIME ] * 0.8f > m_fEndTimer )
			m_fEndTimer = 0.0f;

		m_oCommandQueue.Reset();
		return TFALSE;
	}

	TBOOL bDown = a_iEventType == TInputInterface::EVENT_TYPE_GONE_DOWN || a_iEventType == TInputInterface::EVENT_TYPE_REPEAT;

	switch ( a_eCommand )
	{
		case AInputCommand_LeftWithUp:
		{
			if ( m_ePlayerState == PLAYERSTATE_FILLING || m_ePlayerState == PLAYERSTATE_2 || m_ePlayerState == PLAYERSTATE_THROWING )
				return TTRUE;

			m_oCommandQueue.Reset();
			m_bTowardsEnd = TTRUE;

			if ( !IsPlayerAtTarget() && m_bRunning != TTRUE )
				return TTRUE;

			m_bRunning = TTRUE;
			DetachCarriedGlass();

			if ( !bDown )
			{
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp2 ) );
				m_fRunDirection = 0.0f;
				m_bRunKeyHeld   = TFALSE;
				return TTRUE;
			}

			m_fRunDirection = 0.1f;
			break;
		}
		case AInputCommand_RightWithUp:
		{
			if ( m_ePlayerState == PLAYERSTATE_FILLING || m_ePlayerState == PLAYERSTATE_2 || m_ePlayerState == PLAYERSTATE_THROWING )
				return TTRUE;

			m_oCommandQueue.Reset();
			TBOOL bRunning = m_bRunning == TTRUE;
			m_bTowardsEnd  = TFALSE;

			if ( !bRunning )
				return TTRUE;

			m_bRunning = TTRUE;
			DetachCarriedGlass();

			if ( !bDown )
			{
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp2 ) );
				m_bRunKeyHeld   = TFALSE;
				m_fRunDirection = 0.0f;
				return TTRUE;
			}

			m_fRunDirection = -0.1f;
			break;
		}
		case AInputCommand_ForwardWithUp:
		{
			if ( m_bRunning )
				return TTRUE;

			m_bTowardsEnd = TTRUE;

			if ( !bDown )
			{
				m_bSwitchKeyReleased = TTRUE;
				return TTRUE;
			}

			m_bSwitchKeyReleased = TFALSE;
			m_oCommandQueue.Push( SwitchBar( 1 ) || a_iEventType != TInputInterface::EVENT_TYPE_GONE_DOWN, a_rbHandled, a_eCommand, a_iEventType );
			return TTRUE;
		}
		case AInputCommand_BackwardWithUp:
		{
			if ( m_bRunning )
				return TTRUE;

			m_bTowardsEnd = TTRUE;

			if ( !bDown )
			{
				m_bSwitchKeyReleased = TTRUE;
				return TTRUE;
			}

			m_oCommandQueue.Push( SwitchBar( -1 ) || a_iEventType != TInputInterface::EVENT_TYPE_GONE_DOWN, a_rbHandled, a_eCommand, a_iEventType );
			m_bSwitchKeyReleased = TFALSE;
			return TTRUE;
		}
		case AInputCommand( 45 ):
			return TTRUE;
		case AInputCommand_Shoot:
		{
			if ( bDown )
			{
				if ( m_ePlayerState != PLAYERSTATE_IDLE )
					m_oCommandQueue.Push( TFALSE, a_rbHandled, a_eCommand, TInputInterface::EVENT_TYPE_REPEAT );
				else
					m_oCommandQueue.Push( StartFilling(), a_rbHandled, a_eCommand, TInputInterface::EVENT_TYPE_REPEAT );
			}
			else if ( m_ePlayerState == PLAYERSTATE_FILLING )
			{
				ThrowHeldGlass();
				a_rbHandled = TFALSE;
			}
			else if ( m_bOverfilled )
			{
				m_bOverfilled = TFALSE;
			}
			else
			{
				m_oCommandQueue.Push( TFALSE, a_rbHandled, a_eCommand, a_iEventType );
			}

			return TTRUE;
		}
		case AInputCommand_Switch:
		{
			if ( a_iEventType != TInputInterface::EVENT_TYPE_GONE_DOWN )
				return TTRUE;

			StartPouring();
			m_oCommandQueue.Reset();
			return TFALSE;
		}
		default:
			return TFALSE;
	}

	// Start running
	m_bRunKeyHeld = TTRUE;

	if ( !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( run_lp ), 0.0f ) )
	{
		m_pPlayer->GetAnimatableObject()->KillAllAnimations();
		m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 004e7850
void ATapperMiniGameState::OnRemoval()
{
	APositionalSoundSet::GetSingleton()->SetPaused( TFALSE );

	DisconnectAnimEventListener();

	AUnitManager::GetSingleton()->DestroyUnit( m_pPlayer );
	DestroyGlasses();
	DestroyDrinkers();
	m_oPortrait.Destroy();
	m_oMilkFill.Destroy();

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = NUM_BARS - 1; i >= 0; i-- )
		pAnimObjManager->DestroyAnimatableObject( m_aTips[ i ].pObject );

	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pRoot );
	m_pRoot = TNULL;
	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pCurtain );
	m_pCurtain = TNULL;
	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pWildMike );
	m_pWildMike = TNULL;

	for ( TINT i = 0; i < NUM_TABLES; i++ )
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_apTables[ i ] );

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();
	m_oFloatHud.Destroy();
	AGameLoader::DestroyLevel();
	AUnitManager::GetSingleton()->RemoveAllUnits();
	AUnitManager::GetSingleton()->DeleteDyingUnits();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( 1.0472f );

	AInputHandler::GetSingleton()->StopAllRumbles();
	// NOTE: retail calls the empty function 00628f40 with 0 here
	AInputHandler::GetSingleton()->StopRumble( AInputHandler::GetSingleton()->GetMainController() );

	m_oMessagePopup.Hide();
	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( AMultiplexStream::IsPaused() )
		pStream->Pause( TFALSE );

	AMultiplexStream::GetSingleton()->Close( TTRUE );

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_ROTATE );
	ASunFlareEffect::GetSingleton()->SetEnabled( TTRUE );

	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 004e7a60
TINT ATapperMiniGameState::PickRandomBar()
{
	// NOTE: retail uses a weighted random picker (004ad3d0) with every weight being 1
	struct Entry
	{
		TINT iBar;
		TINT iWeight;
	};

	TRandom* pRandom      = &ARandom::GetSingleton()->m_oRandom;
	TINT     iTotalWeight = 0;
	TINT     iNumEntries  = 0;
	Entry    aEntries[ NUM_BARS ];
	TINT     iNumValid = 0;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		aEntries[ i ].iBar    = 0;
		aEntries[ i ].iWeight = 0;
	}

	if ( m_iNumDestGoals <= 0 )
		return -1;

	for ( TINT i = 0; i < m_iNumDestGoals; i++ )
	{
		if ( i < GetLevel().iNumBars && GetLevel().iMaxBarDrinkers > m_aiNumBarDrinkers[ i ] )
		{
			iTotalWeight++;
			aEntries[ iNumEntries ].iBar      = i + 1;
			aEntries[ iNumEntries++ ].iWeight = 1;
			iNumValid++;
		}
	}

	if ( iNumValid <= 0 )
		return -1;

	// NOTE: retail dereferences TNULL if nothing gets picked (can't happen with these weights)
	if ( iNumEntries == 0 || iTotalWeight == 0 )
		return 0;

	TINT iRandom = pRandom->GetInt( iTotalWeight );
	TINT iSum    = 0;

	for ( TINT i = 0; i < iNumEntries; i++ )
	{
		iSum += aEntries[ i ].iWeight;

		if ( iSum > iRandom )
			return aEntries[ i ].iBar;
	}

	return 0;
}

// $Barnyard: FUNCTION 004e7b40
void ATapperMiniGameState::SendDrinkerToBar( Drinker* a_pDrinker )
{
	ASteer* pSteer     = a_pDrinker->pUnit->GetSteer();
	a_pDrinker->eState = DRINKERSTATE_APPROACH;
	a_pDrinker->fTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( GetLevel().fApproachTimeMin, GetLevel().fApproachTimeMax );
	pSteer->Command_CancelCurrentCommand();

	// NOTE: the result isn't used
	TVector4 vGoal;
	GetRandomGoalPosition( vGoal, a_pDrinker->iBar, "dest" );

	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, a_pDrinker->pUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) ) );

	TFLOAT fSpeed = m_afParams[ FLOATPARAM_WALK_SPEED_BAR1 + a_pDrinker->iLane ] * GetLevel().fWalkSpeed * m_fSpeedScale +
	    ARandom::GetSingleton()->m_oRandom.GetFloat() * m_afParams[ FLOATPARAM_WALK_SPEED_RANDOM ];

	a_pDrinker->fSpeed = fSpeed;
	pMovement->SetWalkingSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, fSpeed );
}

// $Barnyard: FUNCTION 004e7c90
void ATapperMiniGameState::DetachDrinkerGlasses()
{
	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		T2_FOREACH( m_aBars[ i ].llActive, it )
		{
			AAnimatableObject* pGlassObject = it->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) );

			if ( pGlassObject )
			{
				AAnimatableObjectManager::GetSingleton()->DetachObject( pGlassObject, it->pUnit->GetAnimatableObject() );
				FindGlass( pGlassObject )->eState = GLASSSTATE_FULL;
			}
		}

		T2_FOREACH( m_aBars[ i ].llWaiting, it )
		{
			AAnimatableObject* pGlassObject = it->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) );

			if ( pGlassObject )
			{
				AAnimatableObjectManager::GetSingleton()->DetachObject( pGlassObject, it->pUnit->GetAnimatableObject() );
				FindGlass( pGlassObject )->eState = GLASSSTATE_FULL;
			}
		}
	}
}

// $Barnyard: FUNCTION 004e7de0
void ATapperMiniGameState::SendDrinkerToStart( Drinker* a_pDrinker )
{
	if ( a_pDrinker->eState == DRINKERSTATE_RUNAWAY )
		return;

	ASteer* pSteer = a_pDrinker->pUnit->GetSteer();
	pSteer->SetInteracting( 0 );
	pSteer->Command_CancelCurrentCommand();

	// NOTE: the result isn't used
	TVector4 vGoal;
	GetRandomGoalPosition( vGoal, a_pDrinker->iBar, "start" );

	if ( a_pDrinker->eMovement != MOVEMENT_RUN )
	{
		a_pDrinker->eMovement = MOVEMENT_RUN;
		a_pDrinker->pUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) );
	}

	a_pDrinker->eState = DRINKERSTATE_RUNAWAY;
}

// $Barnyard: FUNCTION 004e7e60
void ATapperMiniGameState::ResetLevel()
{
	m_bOverfilled = TFALSE;
	APositionalSoundSet::GetSingleton()->SetPaused( TTRUE );

	// NOTE: this condition can never be true (opened_idle has to be playing and not playing)
	if ( !m_pCurtain->IsAnimPlaying( TPS8( open ), 0.0f ) && !m_pCurtain->IsAnimPlaying( TPS8( open ), 0.0f ) &&
	     m_pCurtain->IsAnimPlaying( TPS8( opened_idle ), 0.0f ) && !m_pCurtain->IsAnimPlaying( TPS8( opened_idle ), 0.0f ) )
	{
		m_pCurtain->PlayAnimation( TPS8( open ) );
	}

	m_eWildMikeState  = WILDMIKESTATE_NONE;
	m_bAnyDancing     = TFALSE;
	m_fSpeedScale     = 1.0f;
	m_bRunning        = TFALSE;
	m_fRunDirection   = 0.0f;
	m_fSwitchBarTimer = 0.0f;
	m_bWaitForThrow   = TFALSE;
	m_iShootDoodad    = 0;
	m_fLevelTime      = 0.0f;
	RandomiseDanceDelays();
	m_iWildMikeScore = m_aiParams[ INTPARAM_WILDMIKE_SCORE ];
	m_iNextSkin      = 0;

	HideTips();
	DetachDrinkerGlasses();
	ReturnHeldGlass();
	ResetGlasses();

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		for ( auto it = m_aBars[ i ].llActive.Begin(); it != m_aBars[ i ].llActive.End(); )
			it = RemoveDrinker( it )->Next();
	}

	ResetPlayer();

	m_iNumActiveDrinkers = 0;
	m_iNumApproaching    = 0;
	m_iNumDrinkersGone   = 0;
	m_iNumSpawned        = 0;

	for ( TINT i = 0; i < NUM_BARS; i++ )
		m_aiNumBarDrinkers[ i ] = 0;

	m_oWildMikeIcon.SetShownInstantly( TFALSE );
	m_fNextWildMikeTime = m_afParams[ FLOATPARAM_WILDMIKE_INTERVAL ] + m_fLevelTime;
}

// $Barnyard: FUNCTION 004e8010
ATapperMiniGameState::Tip* ATapperMiniGameState::SpawnTipAtDrinker( Drinker* a_pDrinker )
{
	TVector4 vPos;
	TVector4 vDirection;
	GetDrinkerGlassPos( a_pDrinker, vPos, vDirection );

	vPos.y -= 0.3f;
	vPos.x += 0.3f;

	return SpawnTip( vPos );
}

// $Barnyard: FUNCTION 004e8060
void ATapperMiniGameState::WinLevel()
{
	m_oMilkFill.Hide();
	m_oWildMikeIcon.SetShown( TFALSE );

	if ( m_eEndState != ENDSTATE_NONE )
		return;

	ASoundManager::GetSingleton()->PlayCue( soundbank::TAPPA_ROOT_HAPPY );

	m_iNumApproaching = 0;
	m_iNumSpawned     = GetLevel().iNumDrinkers;
	m_eEndState       = ENDSTATE_WON;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		for ( auto it = m_aBars[ i ].llActive.Begin(); it != m_aBars[ i ].llActive.End(); )
			it = RemoveDrinker( it )->Next();
	}

	StopGlassSounds();
	DetachDrinkerGlasses();
	ReturnHeldGlass();
	ResetGlasses();
	FacePlayer( TMath::PI * -0.5f );

	m_pPlayer->GetAnimatableObject()->KillAllAnimations();
	m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( win ) );
	m_pRoot->PlayAnimation( TPS8( win ) );

	m_fEndTimer     = m_afParams[ FLOATPARAM_END_TIME ];
	m_bRunning      = TFALSE;
	m_fRunDirection = 0.0f;
	WildMikeLeave();
}

// $Barnyard: FUNCTION 004e81c0
TINT ATapperMiniGameState::PickSpawnLane()
{
	TINT iMin = 100000;

	if ( m_aiLaneSpawns[ 3 ] < 100000 )
		iMin = m_aiLaneSpawns[ 3 ];

	if ( iMin > m_aiLaneSpawns[ 2 ] )
		iMin = m_aiLaneSpawns[ 2 ];

	if ( iMin > m_aiLaneSpawns[ 1 ] )
		iMin = m_aiLaneSpawns[ 1 ];

	if ( iMin > m_aiLaneSpawns[ 0 ] )
		iMin = m_aiLaneSpawns[ 0 ];

	AWeightedRandom<TINT, NUM_BARS> oPicker( &ARandom::GetSingleton()->m_oRandom );
	TINT                            iNumValid = 0;

	// The lanes with fewer spawns are more likely
	for ( TINT iLane = NUM_BARS - 1; iLane >= 0; iLane-- )
	{
		TFLOAT fWeight = GetLevel().afLaneWeights[ iLane ] / TFLOAT( m_aiLaneSpawns[ iLane ] - iMin + 1 );

		if ( fWeight > 0.0f )
		{
			oPicker.Add( iLane, fWeight );
			iNumValid++;
		}
	}

	if ( iNumValid <= 0 )
		return 0;

	AWeightedRandom<TINT, NUM_BARS>::Entry* pPicked = oPicker.Pick();

	if ( pPicked )
		return pPicked->oValue;

	// NOTE: retail dereferences TNULL here
	return 0;
}

// $Barnyard: FUNCTION 004e83c0
ATapperMiniGameState::ATapperMiniGameState()
{
	m_oAnimEventListener.pNext     = &m_oAnimEventListener;
	m_oAnimEventListener.pPrev     = &m_oAnimEventListener;
	m_oAnimEventListener.pCaller   = TNULL;
	m_oAnimEventListener.pCallback = TNULL;
	m_oLevels.iNumLevels           = 0;
	m_fLookTimer                   = 0.0f;
	m_iUnk4F9C                     = 0;
	m_iGlassPool                   = 0;
	m_bSwitchKeyReleased           = TFALSE;
	m_oCommandQueue.Reset();
	m_bUnk5A4C = TFALSE;

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( ATapperMiniGameState ) );

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		m_aiUnk3628[ i ] = i + 2;
		m_aiUnk3638[ i ] = i + 2;
	}

	m_aiEffects[ EFFECT_SPLASH ]        = AParticleEffectManager::GetSingleton()->FindEffect( "tappersplash" );
	m_aiEffects[ EFFECT_DRIP ]          = AParticleEffectManager::GetSingleton()->FindEffect( "tapperdrip" );
	m_aiEffects[ EFFECT_SPLASH_DRINK ]  = AParticleEffectManager::GetSingleton()->FindEffect( "tappersplashdrink" );
	m_aiEffects[ EFFECT_SPLASH_FOLLOW ] = AParticleEffectManager::GetSingleton()->FindEffect( "tappersplashfollow" );
	m_aiEffects[ EFFECT_SPLASH_BREAK ]  = AParticleEffectManager::GetSingleton()->FindEffect( "tappersplashbreak" );
	m_aiEffects[ EFFECT_DUSTPUFF ]      = AParticleEffectManager::GetSingleton()->FindEffect( "tapperdustpuff" );

	TStringManager::String16Format( m_wszLevelText, 31, L"" );
}

// $Barnyard: FUNCTION 004e8680
// $Barnyard: FUNCTION 004e6e10
ATapperMiniGameState::~ATapperMiniGameState()
{
	DisconnectAnimEventListener();
}

// $Barnyard: FUNCTION 004e86a0
void ATapperMiniGameState::CreatePlayer()
{
	TString8 strUnused( "test" );
	m_fRunDirection = 0.0f;

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	TQuaternion        quatRotation;

	for ( TINT i = 0; i < NUM_BARS; i++ )
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "player_tap%d", i + 1 ), m_avBarTaps[ i ], quatRotation );

	for ( TINT i = 0; i < NUM_BARS; i++ )
		pTerrain->GetLocatorPosAndRot( TString8::VarArgs( "player_end%d", i + 1 ), m_avBarEnds[ i ], quatRotation );

	// Move the ends a bit further from the taps
	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		TVector4 vDirection = m_avBarTaps[ i ] - m_avBarEnds[ i ];
		vDirection.w        = m_avBarTaps[ i ].w;
		vDirection.Normalise();

		TVector4 vOffset;
		vOffset.Multiply( vDirection, 0.25f );

		m_avBarEnds[ i ].x -= vOffset.x;
		m_avBarEnds[ i ].y -= vOffset.y;
		m_avBarEnds[ i ].z -= vOffset.z;
	}

	s_vPlayerStart = m_avBarTaps[ 1 ];

	const TPString8* pType;

	if ( APlayerProfileManager::IsSingletonCreated() && APlayerProfileManager::GetSingleton()->GetMainGender() != APlayerProfile::GENDER_MALE )
		pType = &TPS8( tap_player_fe );
	else
		pType = &TPS8( tap_player_m );

	m_pPlayer = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( *pType, TVector4::VEC_ZERO, TVector4::VEC_POSX ) );
	m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
	m_pPlayer->SetVisible( TTRUE );

	if ( APlayerProfileManager::IsSingletonCreated() )
		m_pPlayer->GetAnimatableObject()->ApplyMaterial( APlayerProfileManager::GetSingleton()->GetMainCostume(), TTRUE );

	m_pPlayer->ChangeSteer( &TGetClass( AStationarySteer ) );

	AAnimatableObject*    pPlayerObject = m_pPlayer->GetAnimatableObject();
	ANamedAnimationSetRef pCarrySmall   = m_pPlayer->GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( carrysmall ) );

	if ( pPlayerObject->GetToshiAnimInterface().GetAnimationSet().Get() != pCarrySmall.Get() )
		pPlayerObject->GetToshiAnimInterface().SetAnimationSet( pCarrySmall );

	ResetPlayer();
	m_pPlayer->GetAnimatableObject()->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pPlayer->GetAnimatableObject()->SetVisible( TTRUE );

	// Listen to the animation events (inserted at the tail of the global list)
	AAnimationBreakpointUnitAction::Listener& rHead = AAnimationBreakpointUnitAction::GetListeners();
	m_oAnimEventListener.pCaller                    = this;
	m_oAnimEventListener.pCallback                  = TREINTERPRETCAST( AAnimationBreakpointUnitAction::Listener::t_Callback, OnAnimEventCallback );
	m_oAnimEventListener.pNext                      = &rHead;
	m_oAnimEventListener.pPrev                      = rHead.pPrev;
	rHead.pPrev->pNext                              = &m_oAnimEventListener;
	rHead.pPrev                                     = &m_oAnimEventListener;

	m_vTapDirection   = m_avBarTaps[ 1 ] - m_avBarTaps[ 0 ];
	m_vTapDirection.w = m_avBarTaps[ 1 ].w;
	m_vTapDirection.Normalise();
}

// $Barnyard: FUNCTION 004e8a90
TVector4& ATapperMiniGameState::GetDrinkerDestination( TVector4& a_rOutPos, Drinker* a_pDrinker )
{
	TVector4 vPos;
	GetRandomGoalPosition( vPos, a_pDrinker->iBar, "dest" );
	a_rOutPos = vPos;

	return a_rOutPos;
}

// $Barnyard: FUNCTION 004e8ad0
TBOOL ATapperMiniGameState::ServeDrinker( Drinker* a_pDrinker, Glass* a_pGlass )
{
	if ( !DrinkerCatchGlass( a_pDrinker, a_pGlass ) )
		return TFALSE;

	a_pDrinker->pUnit->GetAnimatableObject()->KillAllAnimations();
	a_pDrinker->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( slideback ) );
	a_pDrinker->eState = DRINKERSTATE_DRINKING;
	a_pDrinker->iNumServed++;

	a_pDrinker->fSlideTime = a_pGlass->vVelocity.Magnitude() / m_afParams[ FLOATPARAM_SLIDEBACK_SPEED_BAR1 + a_pDrinker->iLane ];

	TFLOAT fAnimSpeed = ( a_pDrinker->fSlideTime - m_afParams[ FLOATPARAM_FILL_MIN ] ) / ( m_afParams[ FLOATPARAM_FILL_MAX ] - m_afParams[ FLOATPARAM_FILL_MIN ] ) * m_afParams[ FLOATPARAM_SLIDEBACK_ANIM_SPEED ] + 1.0f;
	a_pDrinker->pUnit->GetAnimatableObject()->SetAnimationSpeed( TPS8( slideback ), fAnimSpeed );

	AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, a_pDrinker->pUnit->GetSteer() );
	AddServedScore( a_pDrinker, a_pGlass->iScore );
	SpawnTipAtDrinker( a_pDrinker );
	pSteer->SetMode( AInteriorSteer::MODE_STILL );
	a_pGlass->fDripTimer = 0.0f;

	return TTRUE;
}
// $Barnyard: FUNCTION 004e8f30
void ATapperMiniGameState::SpawnDrinker( TINT a_iLane, TBOOL a_bWhistle )
{
	if ( m_iNumSpawned >= GetLevel().iMaxSpawns || m_eEndState != ENDSTATE_NONE )
		return;

	Bar& rBar = m_aBars[ a_iLane ];

	if ( rBar.llWaiting.IsEmpty() )
		return;

	TINT iBar = PickRandomBar();

	if ( iBar <= -1 )
		return;

	Drinker* pDrinker = rBar.llWaiting.Begin();
	DetachDrinkerGlass( pDrinker );
	pDrinker->pUnit->SetVisible( TTRUE );

	pDrinker->iBar       = iBar;
	pDrinker->eMovement  = MOVEMENT_RUN;
	pDrinker->iUnk1C     = 0;
	pDrinker->iNumServed = 0;
	pDrinker->fSlideTime = m_afParams[ FLOATPARAM_FILL_MIN ];
	pDrinker->bDancing   = TFALSE;

	pDrinker->oSkin.SetObject( pDrinker->pUnit->GetAnimatableObject(), m_iNextSkin++ );

	if ( m_iNextSkin >= pDrinker->pUnit->GetAnimatableObject()->GetObjectType()->GetNumSkins() )
		m_iNextSkin = 0;

	pDrinker->pUnit->ChangeSteer( &TGetClass( AInteriorSteer ) );

	// Snap the drinker to the first free start goal of the bar ("start1", "start1p1", "start1p2"...)
	TString8 strGoal;
	strGoal.Format( "%s%d", "start", iBar );
	TPString8 strGoalName( strGoal.GetString() );

	AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pDrinker->pUnit->GetSteer() );
	pSteer->SetInteracting( 0 );
	pSteer->SetNetwork( m_pNetwork, 0 );
	pDrinker->pUnit->GetSteer()->GetPosition();

	for ( TINT i = 0;; )
	{
		if ( pSteer->Command_SnapToGoal( strGoalName ) && !FindDrinkerInFront( pDrinker, 0.5f ) )
			break;

		i++;
		strGoal.Format( "%s%dp%d", "start", iBar, i );
		strGoalName = TPString8( strGoal.GetString() );
		pDrinker->pUnit->GetSteer()->GetPosition();
	}

	pSteer->Command_CancelCurrentCommand();

	// NOTE: the results of these are not used
	TVector4 vDestination;
	GetDrinkerDestination( vDestination, pDrinker );
	m_pNetwork->FindClosestArea( vDestination );
	TCHAR szUnused[ 100 ];
	TStringManager::String8Format( szUnused, sizeof( szUnused ), "dest%d", pDrinker->iBar );

	pDrinker->pUnit->SetHashed( TTRUE );
	pDrinker->pUnit->SetNoQuery( TFALSE );
	pDrinker->eState = DRINKERSTATE_WALKUPTO;
	pSteer->SetFlags( pSteer->GetFlags() | AInteriorSteer::FLAGS_KINEMATICS_NO_AVOID );
	pSteer->SetFlags( pSteer->GetFlags() | AInteriorSteer::FLAGS_KINEMATICS_NO_PUSH );

	SetDrinkerWalking( pDrinker );
	rBar.llWaiting.Transfer( pDrinker, rBar.llActive );

	m_fSpawnTimer = GetLevel().fSpawnDelay;
	m_aiNumBarDrinkers[ iBar - 1 ]++;

	TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
	vDirection.Normalise();

	pDrinker->pUnit->GetAnimatableObject()->GetModelInstance()->SetDirection( vDirection );
	pDrinker->pUnit->SetSteerDirection( vDirection );

	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, pDrinker->pUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) ) );

	TFLOAT fSpeed = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, m_afParams[ FLOATPARAM_WALK_SPEED_RANDOM ] ) +
	    m_afParams[ FLOATPARAM_WALK_SPEED_BAR1 + pDrinker->iLane ] * GetLevel().fWalkSpeed * m_fSpeedScale;

	pDrinker->fSpeed = fSpeed;
	pMovement->SetWalkingSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, fSpeed );

	m_iNumActiveDrinkers++;
	m_iNumSpawned++;
	m_aiLaneSpawns[ a_iLane ]++;

	pSteer->SetMode( AInteriorSteer::MODE_STILL );
	pDrinker->pUnit->GetAnimatableObject()->KillAllAnimations();
	pDrinker->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( walk_lp2 ) );

	if ( a_bWhistle )
		ASoundManager::GetSingleton()->PlayCue( soundbank::TAPPA_WHISTLE );
}

// $Barnyard: FUNCTION 004e9470
TBOOL ATapperMiniGameState::UpdateDrinkers( TFLOAT a_fDeltaTime )
{
	if ( UpdateDancing( a_fDeltaTime ) )
		return TFALSE;

	// Spawn the drinkers
	if ( !m_bAnyDancing && m_eWildMikeState == WILDMIKESTATE_NONE )
	{
		m_fSpawnTimer = m_fSpawnTimer - a_fDeltaTime * m_fSpeedScale;

		if ( m_fSpawnTimer < 0.0f )
		{
			TINT iMaxActive = GetLevel().iMaxActiveDrinkers;
			TINT iLane      = PickSpawnLane();

			if ( m_iNumActiveDrinkers < iMaxActive )
				SpawnDrinker( iLane, TTRUE );
		}
	}

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		T2SList<Drinker>& rList = m_aBars[ i ].llActive;

		for ( auto it = rList.Begin(); it != rList.End(); )
		{
			Drinker* pDrinker = it;
			it++;

			AAnimatedUnit* pUnit = pDrinker->pUnit;

			if ( !pUnit->IsVisible() )
				continue;

			if ( pDrinker->fDance > 0.0f )
				continue;

			TVector4 vPos      = *pDrinker->pUnit->GetSteer()->GetPosition();
			pDrinker->vPrevPos = vPos;

			ASteer* pSteer = pUnit->GetSteer();

			switch ( pDrinker->eState )
			{
				case DRINKERSTATE_WALKUPTO:
					if ( IsDrinkerBehindGlassLine( pDrinker, GetLevel().fApproachDistance ) )
					{
						pUnit->GetAnimatableObject()->PlayAnimation( TPS8( walk_lp ) );
						pDrinker->eState = DRINKERSTATE_WAITFORWALK;
					}
					// fall through
				case DRINKERSTATE_9:
					if ( pDrinker->eState != DRINKERSTATE_WALKUPTO && IsDrinkerBehindGlassLine( pDrinker, 0.0f ) )
					{
						pUnit->GetAnimatableObject()->PlayAnimation( TPS8( walk_lp ) );
						pDrinker->eState = DRINKERSTATE_WAITFORWALK;
						m_iNumApproaching++;
					}
					// fall through
				case DRINKERSTATE_APPROACH:
				{
					if ( pDrinker->bDancing )
						break;

					if ( pDrinker->iNumServed > GetLevel().iMaxServed )
					{
						StopDrinker( pDrinker );
						break;
					}

					if ( pDrinker->eState != DRINKERSTATE_9 && pDrinker->eState != DRINKERSTATE_WALKUPTO )
					{
						if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) && !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( walk_lp ) ) )
						{
							StopDrinker( pDrinker );
							break;
						}
					}

					// Walk along the bar
					TVector4 vDestination;
					GetDrinkerDestination( vDestination, pDrinker );
					TFLOAT fDistanceToDestination = TVector4::Distance( vDestination, vPos );

					TFLOAT fT;
					TMath::DistancePointToLineSq( vPos, GetEnemyStart( pDrinker->iBar ), GetEnemyEnd( pDrinker->iBar ), fT );

					TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
					TFLOAT   fAlong     = vDirection.NormaliseMag() * fT;
					pSteer->SetDirection( vDirection );

					TFLOAT fSpeed = m_afParams[ FLOATPARAM_DRINKER_SPEED ];

					if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
						fSpeed *= pDrinker->fSpeed;

					TVector4 vNewPos;
					vNewPos.Add( GetEnemyStart( pDrinker->iBar ), vDirection * ( fSpeed * a_fDeltaTime + fAlong ) );

					pDrinker->pUnit->GetSteer()->GetKinematics();
					pSteer->SetPosition( vNewPos );

					TFLOAT fAnimSpeed = TVector4::DistanceXZ( vNewPos, vPos ) / a_fDeltaTime / m_afParams[ FLOATPARAM_WALK_ANIM_SPEED ];

					if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
						pDrinker->pUnit->GetAnimatableObject()->SetAnimationSpeed( TPS8( walk_lp ), fAnimSpeed );
					else if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp2 ), 0.0f ) )
						pDrinker->pUnit->GetAnimatableObject()->SetAnimationSpeed( TPS8( walk_lp2 ), fAnimSpeed );

					if ( fDistanceToDestination < 0.7f )
					{
						// The drinker reached the end of the bar
						pDrinker->eState = DRINKERSTATE_WAITING;
						LoseGame( pDrinker );
						return TTRUE;
					}

					if ( FindDrinkerInFront( pDrinker, m_afParams[ FLOATPARAM_DRINKER_SPACING ] ) )
						StopDrinker( pDrinker );

					break;
				}
				case DRINKERSTATE_DRINKING:
				{
					if ( pDrinker->bDancing )
					{
						TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
						vDirection.y        = 0.0f;
						vDirection.Normalise();

						pUnit->SetSteerDirection( vDirection );
						pUnit->GetAnimatableObject()->GetModelInstance()->SetDirection( vDirection );
					}

					// Slide back along the bar
					TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
					vDirection.Normalise();

					TVector4 vSlidePos = *pSteer->GetPosition();
					TVector4 vSlide    = vDirection * m_afParams[ FLOATPARAM_SLIDE_SPEED ] * a_fDeltaTime * pDrinker->fSlideTime;
					vSlidePos.x -= vSlide.x;
					vSlidePos.y -= vSlide.y;
					vSlidePos.z -= vSlide.z;
					pSteer->SetPosition( vSlidePos );

					if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( slideback ), 0.0f ) && !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( slideback ) ) )
					{
						Drinker* pBehind    = FindClosestDrinkerOnLine( vSlidePos, 1.1f, pDrinker, TFALSE );
						TFLOAT   fBarLength = TVector4::Distance( GetEnemyEnd( pDrinker->iBar ), GetEnemyStart( pDrinker->iBar ) );

						TVector4 vCurrentPos;
						vCurrentPos.Set( *pSteer->GetPosition() );
						TFLOAT fDistance = TVector4::DistanceXZ( vCurrentPos, GetEnemyEnd( pDrinker->iBar ) );

						if ( fBarLength - m_afParams[ FLOATPARAM_BAR_END_DISTANCE ] > fDistance )
						{
							if ( pBehind || pDrinker->iNumServed > GetLevel().iMaxServed )
								pUnit->GetAnimatableObject()->PlayAnimation( TPS8( slideback ) );
							else
								pDrinker->eState = DRINKERSTATE_DRUNK;
						}
						else
						{
							pDrinker->eState = DRINKERSTATE_DRUNK;
						}
					}

					// Push the drinker behind out of the way
					Drinker* pOther = FindClosestDrinkerOnLine( vSlidePos, 1.0f, pDrinker, TFALSE );

					if ( pOther && pOther->eState != DRINKERSTATE_11 && pOther->eState != DRINKERSTATE_10 )
					{
						pOther->ePrevState = pOther->eState;

						if ( pOther->eState == DRINKERSTATE_DRINKING || pOther->eState == DRINKERSTATE_DRUNK )
						{
							pOther->pUnit->GetAnimatableObject()->KillAllAnimations();
							pOther->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp_walk2 ) );
						}

						pOther->eState = DRINKERSTATE_11;
						pOther->pUnit->GetAnimatableObject()->KillAllAnimations();
						pOther->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( dodge_start ) );

						if ( pOther->fDance > 0.0f )
						{
							pOther->fDance = 0.0f;
							pOther->pUnit->GetAnimatableObject()->GetModelInstance()->SetDirection( m_vWildMikeDirection );
						}

						pOther->vDodgeStart = *pOther->pUnit->GetSteer()->GetPosition();
					}

					// Milk drips from the glass
					AAnimatableObject* pGlassObject = pDrinker->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) );

					if ( pGlassObject )
					{
						Glass* pGlass = FindGlass( pGlassObject );
						pGlass->fDripTimer -= a_fDeltaTime;

						if ( pGlass->fDripTimer < 0.0f )
						{
							TVector4 vGlassPos;

							if ( pGlass->pFull->GetModelInstance().Get() && pGlass->pFull->GetModelInstance()->IsUpdatingSkeleton() )
								pGlass->pFull->GetModelInstance()->GetPosition( vGlassPos );
							else
								pGlass->pEmpty->GetModelInstance()->GetPosition( vGlassPos );

							pGlass->fDripTimer = m_afParams[ FLOATPARAM_GLASS_DRIP_INTERVAL ];

							TVector4 vDripPos = vGlassPos;
							vDripPos.y -= 0.25f;
							AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_SPLASH_DRINK ], vDripPos, TVector4::VEC_NEGY );
						}
					}

					break;
				}
				case DRINKERSTATE_DRUNK:
				{
					if ( pDrinker->bDancing )
					{
						TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
						vDirection.y        = 0.0f;
						vDirection.Normalise();

						pUnit->SetSteerDirection( vDirection );
						pUnit->GetAnimatableObject()->GetModelInstance()->SetDirection( vDirection );
					}

					TFLOAT   fBarLength = TVector4::Distance( GetEnemyEnd( pDrinker->iBar ), GetEnemyStart( pDrinker->iBar ) );
					TVector4 vCurrentPos( *pSteer->GetPosition() );
					TFLOAT   fDistance = TVector4::DistanceXZ( vCurrentPos, GetEnemyEnd( pDrinker->iBar ) );

					if ( !( fBarLength - m_afParams[ FLOATPARAM_BAR_END_DISTANCE ] > fDistance ) )
					{
						// Slid out of the bar
						SendDrinkerToStart( pDrinker );
						break;
					}

					if ( !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( throwback ) ) && !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( throwback ), 0.0f ) )
					{
						pUnit->GetAnimatableObject()->PlayAnimation( TPS8( throwback ) );
						break;
					}

					if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( throwback ), 0.0f ) )
						break;

					if ( pUnit->GetAnimatableObject()->GetAnimationProgress( TPS8( throwback ) ) <= 0.5f )
						break;

					// Catch an empty glass that the player is touching
					for ( auto itGlass = m_aGlassPools[ 0 ].llUsed.Begin(); itGlass != m_aGlassPools[ 0 ].llUsed.End(); )
					{
						Glass* pGlass = itGlass;
						itGlass++;

						TVector4 vGlassPos;

						if ( pGlass->pFull->GetModelInstance().Get() && pGlass->pFull->GetModelInstance()->IsUpdatingSkeleton() )
						{
							if ( pGlass->pFull->GetParentObject() != pUnit->GetAnimatableObject() )
								continue;

							pGlass->pFull->GetModelInstance()->GetPosition( vGlassPos );
						}
						else
						{
							if ( pGlass->pEmpty->GetParentObject() != pUnit->GetAnimatableObject() )
								continue;

							pGlass->pEmpty->GetModelInstance()->GetPosition( vGlassPos );
						}

						TFLOAT fDistance = GetGlassToPlayerDistance( pGlass->vPrevPos, vGlassPos, m_vPlayerPrevPos, m_vPlayerPos );

						if ( !pGlass->pEmpty->GetModelInstance().Get() || !pGlass->pEmpty->GetModelInstance()->IsUpdatingSkeleton() )
							continue;

						if ( pGlass->iBar != m_iBar )
							continue;

						if ( m_afParams[ FLOATPARAM_CATCH_DISTANCE ] * 1.25f <= fDistance )
							continue;

						DrinkerThrowBackGlass( pDrinker );
						GetCurrentPlayer().iMilkServed += m_aiParams[ INTPARAM_CATCH_SCORE ];
						AddScoreAtPlayer( m_aiParams[ INTPARAM_CATCH_SCORE ] );
						itGlass = FreeGlass( pGlass, 0 );
						break;
					}

					break;
				}
				case DRINKERSTATE_THROWBACK:
				{
					if ( pDrinker->bDancing )
					{
						TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
						vDirection.y        = 0.0f;
						vDirection.Normalise();

						pUnit->SetSteerDirection( vDirection );
						pUnit->GetAnimatableObject()->GetModelInstance()->SetDirection( vDirection );
					}

					if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( throwback ), 0.0f ) && !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( throwback ) ) )
						SendDrinkerToBar( pDrinker );

					break;
				}
				case DRINKERSTATE_RUNAWAY:
				{
					TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
					vDirection.Normalise();

					TVector4 vRunPos = *pSteer->GetPosition();
					TVector4 vStep   = vDirection * m_afParams[ FLOATPARAM_SLIDE_SPEED ] * a_fDeltaTime * pDrinker->fSlideTime;
					vRunPos.x -= vStep.x;
					vRunPos.y -= vStep.y;
					vRunPos.z -= vStep.z;
					pSteer->SetPosition( vRunPos );

					TFLOAT fBarLength = TVector4::Distance( GetEnemyEnd( pDrinker->iBar ), GetEnemyStart( pDrinker->iBar ) );

					if ( TVector4::DistanceXZ( vRunPos, GetEnemyEnd( pDrinker->iBar ) ) >= fBarLength )
						RemoveDrinker( pDrinker );

					break;
				}
				case DRINKERSTATE_STOPPED:
				{
					if ( pDrinker->bDancing )
						break;

					pDrinker->fTimer += a_fDeltaTime;

					if ( IsDrinkerBehindGlassLine( pDrinker, 0.0f ) )
					{
						if ( pDrinker->fTimer <= 0.0f || pDrinker->iNumServed > GetLevel().iMaxServed )
						{
							if ( !pUnit->GetAnimatableObject()->IsAnimationPlanned( *s_apIdleAnims[ 0 ] ) &&
							     !pUnit->GetAnimatableObject()->IsAnimationPlanned( *s_apIdleAnims[ 1 ] ) &&
							     !pUnit->GetAnimatableObject()->IsAnimationPlanned( *s_apIdleAnims[ 2 ] ) &&
							     !pUnit->GetAnimatableObject()->IsAnimationPlanned( *s_apIdleAnims[ 3 ] ) )
							{
								const TPString8* pAnim;

								if ( IsDrinkerBehindGlassLine( pDrinker, 0.0f ) )
								{
									TVector4 vDestination;
									GetDrinkerDestination( vDestination, pDrinker );
									TFLOAT fDistance = TVector4::Distance( vDestination, vPos );

									if ( pDrinker->iNumServed > GetLevel().iMaxServed )
									{
										pAnim = &GetRandomIdleAnim();
									}
									else if ( m_afParams[ FLOATPARAM_ANGRY2_DISTANCE ] <= fDistance )
									{
										if ( m_afParams[ FLOATPARAM_ANGRY1_DISTANCE ] <= fDistance )
											pAnim = &GetRandomIdleAnim();
										else
											pAnim = s_apIdleAnims[ 2 ];
									}
									else
									{
										pAnim = s_apIdleAnims[ 3 ];
									}
								}
								else
								{
									pAnim = &TPS8( idle_lp_walk2 );
								}

								pUnit->GetAnimatableObject()->PlayAnimation( *pAnim );
							}
						}
						else if ( !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( walk_lp ) ) && !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
						{
							pUnit->GetAnimatableObject()->PlayAnimation( TPS8( walk_lp ) );
							pDrinker->eState = DRINKERSTATE_WAITFORWALK;
						}
					}
					else if ( !FindDrinkerInFront( pDrinker, m_afParams[ FLOATPARAM_DRINKER_SPACING ] + m_afParams[ FLOATPARAM_DRINKER_SPACING ] ) )
					{
						if ( !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( walk_lp2 ) ) && !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp2 ), 0.0f ) )
						{
							pUnit->GetAnimatableObject()->PlayAnimation( TPS8( walk_lp2 ) );
							pDrinker->eState = ( m_fLevelTime >= 1.0f ) ? DRINKERSTATE_9 : DRINKERSTATE_WALKUPTO;
						}
					}

					// Speed of the idle animations
					for ( TINT j = 0; j < 2; j++ )
					{
						if ( pUnit->GetAnimatableObject()->IsAnimPlaying( *s_apIdleAnims[ j ], 0.0f ) )
						{
							TFLOAT fRandom = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, m_afParams[ FLOATPARAM_WALK_SPEED_RANDOM ] );
							TFLOAT fSpeed  = ( m_afParams[ FLOATPARAM_WALK_SPEED_BAR1 + pDrinker->iLane ] * GetLevel().fWalkSpeed * m_fSpeedScale + fRandom ) * m_afParams[ FLOATPARAM_IDLE_ANIM_SPEED ];
							pUnit->GetAnimatableObject()->SetAnimationSpeed( *s_apIdleAnims[ j ], fSpeed );
						}
					}

					for ( TINT j = 2; j < 4; j++ )
					{
						if ( pUnit->GetAnimatableObject()->IsAnimPlaying( *s_apIdleAnims[ j ], 0.0f ) )
						{
							TFLOAT fRandom = ARandom::GetSingleton()->m_oRandom.GetFloat() * m_afParams[ FLOATPARAM_WALK_SPEED_RANDOM ];
							TFLOAT fSpeed  = ( m_afParams[ FLOATPARAM_WALK_SPEED_BAR1 + pDrinker->iLane ] * GetLevel().fWalkSpeed * m_fSpeedScale + fRandom ) * m_afParams[ FLOATPARAM_IDLE_ANIM_SPEED ];
							pUnit->GetAnimatableObject()->SetAnimationSpeed( *s_apIdleAnims[ j ], fSpeed );
						}
					}

					break;
				}
				case DRINKERSTATE_WAITFORWALK:
				{
					if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
						SendDrinkerToBar( pDrinker );
					else if ( !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( walk_lp ) ) )
						pUnit->GetAnimatableObject()->PlayAnimation( TPS8( walk_lp ) );

					break;
				}
				case DRINKERSTATE_10:
				case DRINKERSTATE_11:
				{
					// Dodging a drinker that slides back
					if ( pDrinker->bDancing )
					{
						TVector4 vDirection = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
						vDirection.y        = 0.0f;
						vDirection.Normalise();

						pUnit->SetSteerDirection( vDirection );
						pUnit->GetAnimatableObject()->GetModelInstance()->SetDirection( vDirection );
					}

					TVector4 vDodgePos = *pSteer->GetPosition();
					Drinker* pOther    = FindClosestDrinkerOnLine( vDodgePos, ( pDrinker->eState == DRINKERSTATE_11 ) ? 1.0f : 0.6f, pDrinker, TTRUE );

					if ( pOther )
					{
						TVector4 vSide = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
						vSide.Normalise();
						vSide.AsVector3().RotateY( TMath::PI * 0.5f );

						TVector4 vStep = vSide * m_afParams[ FLOATPARAM_DODGE_SPEED ] * a_fDeltaTime;
						vDodgePos.x += vStep.x;
						vDodgePos.y += vStep.y;
						vDodgePos.z += vStep.z;

						if ( TVector4::Distance( vDodgePos, pDrinker->vDodgeStart ) > m_afParams[ FLOATPARAM_DODGE_DISTANCE ] )
							pDrinker->eState = DRINKERSTATE_10;

						if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( dodge_start ), 0.0f ) && !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( dodge_start ) ) &&
						     !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( dodge_mid ), 0.0f ) && !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( dodge_mid ) ) )
						{
							pUnit->GetAnimatableObject()->PlayAnimation( TPS8( dodge_start ) );
						}
					}
					else if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( dodge_mid ), 0.0f ) )
					{
						TVector4 vSide = GetEnemyEnd( pDrinker->iBar ) - GetEnemyStart( pDrinker->iBar );
						vSide.Normalise();
						vSide.AsVector3().RotateY( TMath::PI * 0.5f );

						TVector4 vStep = vSide * m_afParams[ FLOATPARAM_DODGE_SPEED ] * a_fDeltaTime;
						vDodgePos.x -= vStep.x;
						vDodgePos.y -= vStep.y;
						vDodgePos.z -= vStep.z;

						TVector4 vTarget = pDrinker->vDodgeStart - vSide * m_afParams[ FLOATPARAM_DODGE_DISTANCE ];

						if ( TVector4::Distance( vDodgePos, vTarget ) < m_afParams[ FLOATPARAM_DODGE_DISTANCE ] )
						{
							pUnit->GetAnimatableObject()->PlayAnimation( TPS8( dodge_end ) );

							DRINKERSTATE eState = pDrinker->ePrevState;

							if ( eState != DRINKERSTATE_DRINKING && eState != DRINKERSTATE_DRUNK && eState != DRINKERSTATE_THROWBACK )
								eState = pDrinker->pUnit->GetAnimatableObject()->FindAttachedObject( TPS8( cb_llocator ) ) ? DRINKERSTATE_DRUNK : DRINKERSTATE_APPROACH;

							pDrinker->eState = eState;
						}
					}

					break;
				}
			}

			// NOTE: the result of this isn't used
			if ( pDrinker != rList.End() )
			{
				TVector4 vUnused;
				pDrinker->pUnit->GetModelInstance()->GetPosition( vUnused );
			}
		}
	}

	return TFALSE;
}
// $Barnyard: FUNCTION 004e2090
void ATapperMiniGameState::Glass::UpdateMotion( TFLOAT a_fDeltaTime )
{
	if ( vVelocity.x == 0.0f && vVelocity.y == 0.0f && vVelocity.z == 0.0f )
		return;

	ATapperMiniGameState* pState = ATapperMiniGameState::GetSingleton();

	TVector4 vPos;
	pFull->GetModelInstance()->GetPosition( vPos );

	// Falling glasses get pulled down
	if ( eState == GLASSSTATE_FALLING )
		vVelocity.y = a_fDeltaTime * pState->m_afParams[ FLOATPARAM_GRAVITY ] + vVelocity.y;

	// Slow down to the minimum speed
	TFLOAT fMagnitudeSq = vVelocity.MagnitudeSq();
	TFLOAT fMinSpeed    = pState->m_afParams[ FLOATPARAM_FILL_MIN ];

	if ( fMagnitudeSq > fMinSpeed * fMinSpeed )
	{
		TFLOAT fSpeed = TMath::Sqrt( fMagnitudeSq );
		vVelocity.Divide( fSpeed );

		fSpeed = fSpeed - a_fDeltaTime * pState->m_afParams[ FLOATPARAM_GLASS_FRICTION ];
		vVelocity.Multiply( fSpeed );

		if ( fSpeed < fMinSpeed )
		{
			vVelocity.Multiply( 1.0f / TMath::Sqrt( fSpeed * fSpeed ) );
			vVelocity.Multiply( fMinSpeed );
		}
	}

	TVector4 vStep;
	vStep.Multiply( vVelocity, a_fDeltaTime );

	TVector4 vNewPos;
	vNewPos.x = vStep.x + vPos.x;
	vNewPos.y = vStep.y + vPos.y;
	vNewPos.z = vStep.z + vPos.z;
	vNewPos.w = vPos.w;

	pFull->GetModelInstance()->SetTranslation( vNewPos.AsVector3() );
	pEmpty->GetModelInstance()->SetTranslation( vNewPos.AsVector3() );
	pFull->GetModelInstance()->SetDirection( TVector4::VEC_POSX );
	pEmpty->GetModelInstance()->SetDirection( TVector4::VEC_POSX );
}

// $Barnyard: FUNCTION 004de880
TFLOAT ATapperMiniGameState::GetGlassToPlayerDistance( const TVector4& a_rcGlassPrevPos, const TVector4& a_rcGlassPos, const TVector4& a_rcPlayerPrevPos, const TVector4& a_rcPlayerPos )
{
	TVector2 vGlassPrev( a_rcGlassPrevPos.x, a_rcGlassPrevPos.z );
	TVector2 vGlass( a_rcGlassPos.x, a_rcGlassPos.z );

	// Extend the glass segment backwards by 0.25
	TVector2 vDirection( vGlassPrev.x - vGlass.x, vGlassPrev.y - vGlass.y );

	if ( vDirection.y * vDirection.y + vDirection.x * vDirection.x > 0.0f )
	{
		vDirection.Normalize();
		vGlassPrev.x = vDirection.x * 0.25f + vGlassPrev.x;
		vGlassPrev.y = vDirection.y * 0.25f + vGlassPrev.y;
	}

	TVector2 vPlayerPrev( a_rcPlayerPrevPos.x, a_rcPlayerPrevPos.z );
	TVector2 vPlayer( a_rcPlayerPos.x, a_rcPlayerPos.z );

	return TMath::DistanceSegmentToSegment2D( vGlassPrev, vGlass, vPlayerPrev, vPlayer ) - 0.05f;
}

// Returns the position of the visible model of the glass
static void GetGlassPosition( AAnimatableObject* a_pFull, AAnimatableObject* a_pEmpty, TVector4& a_rOutPos )
{
	if ( a_pFull->GetModelInstance().Get() && a_pFull->GetModelInstance()->IsUpdatingSkeleton() )
		a_pFull->GetModelInstance()->GetPosition( a_rOutPos );
	else
		a_pEmpty->GetModelInstance()->GetPosition( a_rOutPos );
}

// $Barnyard: FUNCTION 004eac20
TBOOL ATapperMiniGameState::UpdateGlasses( TFLOAT a_fDeltaTime )
{
	TBOOL bLost = TFALSE;

	for ( TINT iPool = 0; iPool < GlassPools::NUM_POOLS; iPool++ )
	{
		T2SList<Glass>& rUsed = m_aGlassPools[ iPool ].llUsed;

		for ( auto it = rUsed.Begin(); it != rUsed.End(); )
		{
			Glass* pGlass = it;
			it++;

			if ( pGlass->vVelocity.x == 0.0f && pGlass->vVelocity.y == 0.0f && pGlass->vVelocity.z == 0.0f )
				continue;

			if ( ( !pGlass->pFull->GetModelInstance().Get() || !pGlass->pFull->GetModelInstance()->IsUpdatingSkeleton() ) &&
			     ( !pGlass->pEmpty->GetModelInstance().Get() || !pGlass->pEmpty->GetModelInstance()->IsUpdatingSkeleton() ) )
			{
				continue;
			}

			pGlass->pFull->GetModelInstance()->GetPosition( pGlass->vPrevPos );

			if ( pGlass->eState == GLASSSTATE_FULL )
			{
				// Milk splashes behind the glass
				pGlass->fDripTimer -= a_fDeltaTime;

				if ( pGlass->fDripTimer < 0.0f )
				{
					TVector4 vPos;
					GetGlassPosition( pGlass->pFull, pGlass->pEmpty, vPos );

					pGlass->fDripTimer = m_afParams[ FLOATPARAM_GLASS_SPLASH_INTERVAL ];

					TVector4 vSplashPos( vPos.x, vPos.y - 0.25f, vPos.z, vPos.w );

					TVector4 vDirection = TVector4::VEC_NEGY;
					TVector4 vHalfVelocity;
					vHalfVelocity.Multiply( pGlass->vVelocity, 0.5f );
					vDirection.x -= vHalfVelocity.x;
					vDirection.y -= vHalfVelocity.y;
					vDirection.z -= vHalfVelocity.z;
					vDirection.Normalise();

					AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_SPLASH_FOLLOW ], vSplashPos, vDirection );
				}
			}

			switch ( pGlass->eState )
			{
				case GLASSSTATE_FULL:
				{
					TVector4 vPrevPos;
					GetGlassPosition( pGlass->pFull, pGlass->pEmpty, vPrevPos );

					pGlass->UpdateMotion( a_fDeltaTime );

					TVector4 vPos;
					GetGlassPosition( pGlass->pFull, pGlass->pEmpty, vPos );

					ASoundManager::GetSingleton()->SetCuePosition( pGlass->iCue, vPos, 0.0f );

					TFLOAT fDistance    = TVector4::DistanceXZ( vPos, GetPlayerBeerStart( pGlass->iBar ) );
					TFLOAT fMaxDistance = TVector4::Distance( GetEnemyBeerEnd( pGlass->iBar ), GetEnemyBeerStart( pGlass->iBar ) ) + 0.3f;

					if ( m_bRunning == TTRUE && pGlass->iBar == m_iBar && GetGlassToPlayerDistance( pGlass->vPrevPos, vPos, m_vPlayerPrevPos, m_vPlayerPos ) < m_afParams[ FLOATPARAM_CATCH_DISTANCE ] )
					{
						// The player has caught the glass he just threw
						it = FreeGlass( pGlass, iPool );
						it++;
					}
					else if ( fDistance <= fMaxDistance )
					{
						Drinker* pDrinker = FindDrinkerForGlass( vPrevPos, vPos, fMaxDistance, pGlass->iBar );

						if ( pDrinker )
							ServeDrinker( pDrinker, pGlass );
					}
					else
					{
						// Nobody took the glass, it falls off the bar
						AddGlassScore( pGlass, -m_aiParams[ INTPARAM_MISSED_GLASS_PENALTY ] );
						ASoundManager::GetSingleton()->StopCue( pGlass->iCue );
						pGlass->iCue      = -1;
						pGlass->vVelocity = TVector4( 0.0f, 0.01f, 0.0f, 1.0f );
						pGlass->eState    = GLASSSTATE_FALLING;
					}

					break;
				}
				case GLASSSTATE_EMPTY:
				{
					pGlass->UpdateMotion( a_fDeltaTime );

					TVector4 vPos;
					GetGlassPosition( pGlass->pFull, pGlass->pEmpty, vPos );

					ASoundManager::GetSingleton()->SetCuePosition( pGlass->iCue, vPos, 0.0f );

					TFLOAT fDistance = GetGlassToPlayerDistance( pGlass->vPrevPos, vPos, m_vPlayerPrevPos, m_vPlayerPos );

					if ( pGlass->iBar == m_iBar && fDistance < m_afParams[ FLOATPARAM_CATCH_DISTANCE ] )
					{
						// Caught an empty glass
						GetCurrentPlayer().iMilkServed += m_aiParams[ INTPARAM_CATCH_SCORE ];
						AddScoreAtPlayer( m_aiParams[ INTPARAM_CATCH_SCORE ] );
						it = FreeGlass( pGlass, iPool );
						it++;
					}
					else
					{
						TFLOAT fFromStart = TVector4::DistanceXZ( vPos, GetEnemyBeerStart( pGlass->iBar ) );

						if ( TVector4::Distance( GetEnemyBeerEnd( pGlass->iBar ), GetEnemyBeerStart( pGlass->iBar ) ) >= fFromStart )
							break;

						// Missed the empty glass, it falls off the bar
						if ( m_aiParams[ INTPARAM_MISSED_EMPTY_PENALTY ] >= 0 )
							AddGlassScore( pGlass, -m_aiParams[ INTPARAM_MISSED_EMPTY_PENALTY ] );

						ASoundManager::GetSingleton()->StopCue( pGlass->iCue );
						pGlass->iCue      = -1;
						pGlass->vVelocity = TVector4( 0.0f, 0.01f, 0.0f, 1.0f );
						pGlass->eState    = GLASSSTATE_FALLING;
					}

					break;
				}
				case GLASSSTATE_FALLING:
				{
					pGlass->UpdateMotion( a_fDeltaTime );

					if ( pGlass->vVelocity.y < m_afParams[ FLOATPARAM_GLASS_BREAK_SPEED ] )
						break;

					// The glass smashes on the floor
					TVector4 vPos;
					GetGlassPosition( pGlass->pFull, pGlass->pEmpty, vPos );

					TVector4 vEffectPos = vPos;
					AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_SPLASH_BREAK ], vEffectPos, TVector4::VEC_NEGY );
					vEffectPos.y -= 0.3f;
					AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_SPLASH_BREAK ], vEffectPos, TVector4::VEC_NEGY );
					ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::TAPPA_SMASH, vPos );

					AInputHandler::RumbleParams oRumble = { { m_afParams[ FLOATPARAM_SMASH_RUMBLE ], 0.0f, 0.0f, m_afParams[ FLOATPARAM_SMASH_RUMBLE_TIME ], 0.0f, 1.0f, 1.0f }, 1 };
					AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );

					if ( m_eEndState != ENDSTATE_LOST && m_eEndState != ENDSTATE_WON )
					{
						m_pRoot->PlayAnimation( TPS8( drop_glass ) );
						AddLives( GetCurrentPlayer(), -1 );

						if ( GetCurrentPlayer().iLives <= 0 )
						{
							LoseGame( TNULL );
							bLost = TTRUE;
						}
					}

					it = FreeGlass( pGlass, iPool );
					it++;

					if ( m_aiParams[ INTPARAM_MISSED_GLASS_PENALTY ] < 0 )
					{
						LoseGame( TNULL );
						bLost = TTRUE;
					}

					break;
				}
			}
		}
	}

	return bLost;
}

// $Barnyard: FUNCTION 004eb6e0
void ATapperMiniGameState::UpdateRoot( TFLOAT a_fDeltaTime )
{
	// The root gets angry when a drinker gets close to the end of a bar
	TBOOL bAngry = TFALSE;

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		for ( auto it = m_aBars[ i ].llActive.Begin(); it != m_aBars[ i ].llActive.End(); )
		{
			Drinker* pDrinker = it;
			it++;

			if ( pDrinker->pUnit->IsVisible() )
			{
				TVector4 vPos = *pDrinker->pUnit->GetSteer()->GetPosition();

				TVector4 vDestination;
				GetRandomGoalPosition( vDestination, pDrinker->iBar, "dest" );

				if ( TVector4::Distance( vDestination, vPos ) < m_afParams[ FLOATPARAM_ROOT_ANGRY_DISTANCE ] )
					bAngry = TTRUE;
			}
		}
	}

	if ( m_eEndState == ENDSTATE_LOST || m_eEndState == ENDSTATE_WON )
		return;

	if ( bAngry )
	{
		m_pRoot->PlayAnimation( TPS8( angry_lp ) );
	}
	else if ( !m_pRoot->IsAnimationPlanned( TPS8( drop_glass ) ) && !m_pRoot->IsAnimPlaying( TPS8( drop_glass ), 0.0f ) )
	{
		if ( m_pRoot->IsAnimationPlanned( TPS8( angry_lp ) ) || m_pRoot->IsAnimPlaying( TPS8( angry_lp ), 0.0f ) )
			m_pRoot->PlayAnimation( TPS8( idle_lp ) );
	}
}

// $Barnyard: FUNCTION 004eb880
void ATapperMiniGameState::StartLevel()
{
	ResetLevel();

	m_iTotalScore = 0;
	m_fSpawnTimer = GetLevel().fSpawnDelay;

	for ( TINT i = NUM_BARS - 1; i >= 0; i-- )
		m_aiLaneSpawns[ i ] = 0;

	for ( TINT i = NUM_BARS - 1; i >= 0; i-- )
		m_aiNumBarDrinkers[ i ] = 0;

	// The drinkers that are already at the bars
	for ( TINT i = GetLevel().iNumInitialDrinkers; i > 0; i-- )
		SpawnDrinker( PickSpawnLane(), TFALSE );

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		T2_FOREACH( m_aBars[ i ].llActive, it )
		{
			it->pUnit->GetAnimatableObject()->KillAllAnimations();
			StopDrinker( it );
		}
	}

	m_fTimeLimit = 10000.0f;
	ResetPlayer();
	HideTips();
	APositionalSoundSet::GetSingleton()->SetPaused( TFALSE );

	// Show the tables of the bars that are used
	TINT iNumBars = GetLevel().iNumBars;

	if ( iNumBars == 4 )
	{
		m_apTables[ TABLE_3_ADD ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
		m_apTables[ TABLE_4_ADD ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
		m_apTables[ TABLE_3_SUB ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
		m_apTables[ TABLE_4_SUB ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
	}
	else if ( iNumBars == 3 )
	{
		m_apTables[ TABLE_3_ADD ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
		m_apTables[ TABLE_4_ADD ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
		m_apTables[ TABLE_3_SUB ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
		m_apTables[ TABLE_4_SUB ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
	}
	else if ( iNumBars < 3 )
	{
		m_apTables[ TABLE_3_ADD ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
		m_apTables[ TABLE_4_ADD ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
		m_apTables[ TABLE_3_SUB ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
		m_apTables[ TABLE_4_SUB ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
	}

	m_oFloatHud.Hide();
	m_oMilkFill.Hide();
	m_oPortrait.Hide();
	m_oLivesHud.SetVisible( TFALSE );
	m_oWildMikeIcon.SetShownInstantly( TFALSE );
	m_oBonusLifePopup.SetShownInstantly( TFALSE );
	m_oBonusLifeTextBox.Hide();
	m_oMessagePopup.SetText( TNULL, 0x55000000, TNULL );
	m_oPickUpHudElement.SetForceHide( TTRUE );
}

// $Barnyard: FUNCTION 004ebd90
void ATapperMiniGameState::OnPause()
{
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );

	if ( m_bIntroStarted )
	{
		m_fNextWildMikeTime = m_fLevelTime + m_afParams[ FLOATPARAM_WILDMIKE_INTERVAL ];
		AMultiplexStream::GetSingleton()->Open( "tappa" );
		SetupCamera();
		StartLevel();
		m_bIntroStarted = TFALSE;
	}

	m_pPlayer->SetVisible( TTRUE );
}

// $Barnyard: FUNCTION 004ebe00
void ATapperMiniGameState::EndPlayerTurn( TBOOL a_bLevelsDone )
{
	m_oMessagePopup.SetText( TNULL, 0x55000000, TNULL );
	StopGlassSounds();
	DetachDrinkerGlasses();
	ReturnHeldGlass();
	ResetGlasses();

	m_iNumTurns++;
	m_iTotalScore             = 0;
	GetCurrentPlayer().iLevel = m_iLevel;

	// Find the next player that can still play
	m_iCurrentPlayer++;

	if ( m_iCurrentPlayer >= m_iNumPlayers )
		m_iCurrentPlayer = 0;

	TINT iNumChecked = 0;

	while ( !( GetCurrentPlayer().iLives != 0 && GetCurrentPlayer().iLevel < m_aiParams[ INTPARAM_NUM_LEVELS_EASY + m_iDifficulty ] ) && iNumChecked < m_iNumPlayers )
	{
		iNumChecked++;
		m_iCurrentPlayer++;

		if ( m_iCurrentPlayer >= m_iNumPlayers )
			m_iCurrentPlayer = 0;
	}

	m_iNumApproaching  = 0;
	m_iNumDrinkersGone = 0;
	m_iNumSpawned      = 0;

	if ( iNumChecked < m_iNumPlayers )
	{
		m_fTimeLimit += 100000.0f;
		m_iLevel = GetCurrentPlayer().iLevel;
		m_oLivesHud.SetLives( GetCurrentPlayer().iLives, TFALSE );
		ResetLevel();
		m_oPortrait.Show();
		m_oLivesHud.SetVisible( TTRUE );

		if ( m_iNumPlayers <= 1 )
			m_oStartHudElement.Setup( m_iNumPlayers, m_iCurrentPlayer );
		else
			OnSetupPlayers();

		StartLevel();
	}
	else
	{
		// Every player has finished
		m_iLevel = 0;
		HideTips();
		DetachDrinkerGlasses();
		m_oLivesHud.SetVisible( TTRUE );

		for ( TINT i = 0; i < NUM_BARS; i++ )
		{
			for ( auto it = m_aBars[ i ].llActive.Begin(); it != m_aBars[ i ].llActive.End(); )
			{
				Drinker* pDrinker = it;
				it++;

				pDrinker->pUnit->GetAnimatableObject()->KillAllAnimations();
				pDrinker->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp3 ) );
				pDrinker->eState = DRINKERSTATE_WAITING;
			}
		}

		APositionalSoundSet::GetSingleton()->SetPaused( TTRUE );
		ResetGlasses();

		AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );
		AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );
	}
}

// $Barnyard: FUNCTION 004ec0d0
void ATapperMiniGameState::NextLevel()
{
	m_oMilkFill.Hide();
	ResetPlayer();

	// Time bonus
	TFLOAT fTime      = m_fLevelTime;
	TINT   iLevel     = m_iLevel;
	TINT   iTimeBonus = 0;

	if ( fTime < m_oLevels.aLevels[ iLevel ].fTimeBonusTime )
	{
		fTime      = ( m_oLevels.aLevels[ iLevel ].fTimeBonusTime - m_fLevelTime ) * m_oLevels.aLevels[ iLevel ].fTimeBonusScale;
		iTimeBonus = TINT( fTime );
	}

	m_iLevel = iLevel + 1;

	if ( iLevel + 1 < m_aiParams[ INTPARAM_NUM_LEVELS_EASY + m_iDifficulty ] )
	{
		if ( m_iNumPlayers <= 1 )
			m_oStartHudElement.Setup( m_iNumPlayers, m_iCurrentPlayer );
		else
			OnSetupPlayers();

		StartLevel();
	}
	else
	{
		EndPlayerTurn( TTRUE );
	}

	GetCurrentPlayer().iTimeBonus += iTimeBonus;
	AddScore( GetCurrentPlayer(), iTimeBonus );

	m_iTotalScore = 0;
	m_fLevelTime  = 0.0f;
	m_oMessagePopup.SetText( TNULL, 0x55000000, TNULL );
}

// $Barnyard: FUNCTION 004ec1f0
void ATapperMiniGameState::UpdateGame( TFLOAT a_fDeltaTime )
{
	UpdateTips( a_fDeltaTime );

	if ( ARootTask::GetSingleton()->IsPaused() )
		return;

	m_oLivesHud.Tick( a_fDeltaTime );

	if ( m_oBonusLifePopup.IsVisible() && m_oBonusLifePopup.m_fTargetScale != 0.0f && m_fBonusLifeTextTimer < 0.0f )
	{
		m_oBonusLifePopup.SetShownInstantly( TTRUE );
		m_fBonusLifeTextTimer = 2.0f;
	}

	m_fBonusLifeTextTimer -= a_fDeltaTime;

	if ( m_fBonusLifeTextTimer < 0.0f )
		m_oBonusLifePopup.SetShown( TFALSE );

	UpdateMusic( a_fDeltaTime );
	m_oMessagePopup.Update( a_fDeltaTime );
	m_oCommandQueue.Process( this );

	// The drinkers get faster and faster
	if ( m_eWildMikeState == WILDMIKESTATE_NONE )
		m_fSpeedScale = a_fDeltaTime * m_afParams[ FLOATPARAM_SPEED_INCREASE ] + m_fSpeedScale;

	m_fSwitchBarTimer -= a_fDeltaTime;
	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( GetFOV() );

	if ( !m_oStartHudElement.IsForceHidden() )
		return;

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devautoplay" runs fill and throw glasses and switch the bars
	if ( T2String8::FindString( GetCommandLineA(), "-devautoplay" ) && m_eEndState == ENDSTATE_NONE )
	{
		static TFLOAT s_fTime  = 0.0f;
		static TINT   s_iCycle = 0;
		static TINT   s_iStep  = 0;
		TBOOL         bHandled = TFALSE;

		s_fTime += a_fDeltaTime;

		if ( s_iStep == 0 && s_fTime > 0.5f )
		{
			// Hold Shoot to fill a glass
			HandleCommand( AInputCommand_Shoot, TInputInterface::EVENT_TYPE_GONE_DOWN, bHandled );
			s_iStep = 1;
		}
		else if ( s_iStep == 1 && s_fTime > 1.0f )
		{
			// Release it to throw the glass
			HandleCommand( AInputCommand_Shoot, TInputInterface::EVENT_TYPE_GONE_UP, bHandled );
			TINFO( "[devautoplay] threw a glass at bar %d, score %d, lives %d\n", m_iBar, GetCurrentPlayer().iScore, GetCurrentPlayer().iLives );
			s_iStep = 2;
		}
		else if ( s_iStep == 2 && s_fTime > 1.6f )
		{
			// Every 8th glass goes to the other bar
			if ( ( ++s_iCycle & 7 ) == 0 )
			{
				AInputCommand eCommand = ( ( s_iCycle / 8 ) & 1 ) ? AInputCommand_ForwardWithUp : AInputCommand_BackwardWithUp;
				HandleCommand( eCommand, TInputInterface::EVENT_TYPE_GONE_DOWN, bHandled );
				HandleCommand( eCommand, TInputInterface::EVENT_TYPE_GONE_UP, bHandled );
			}

			s_fTime = 0.0f;
			s_iStep = 0;
		}
	}
#endif

	if ( m_eEndState != ENDSTATE_NONE )
	{
		m_fEndTimer -= a_fDeltaTime;

		if ( m_fEndTimer < 0.0f )
		{
			m_pCurtain->PlayAnimation( TPS8( open ) );

			if ( m_eEndState == ENDSTATE_WON )
			{
				m_eEndState = ENDSTATE_NONE;
				NextLevel();
				return;
			}

			if ( m_eEndState == ENDSTATE_LOST )
			{
				m_eEndState = ENDSTATE_NONE;
				EndPlayerTurn( TFALSE );
				return;
			}
		}
		else if ( m_fEndTimer < m_afParams[ FLOATPARAM_CURTAIN_TIME ] && !m_pCurtain->IsAnimationPlanned( TPS8( close ) ) && !m_pCurtain->IsAnimPlaying( TPS8( close ), 0.0f ) && !m_pCurtain->IsAnimationPlanned( TPS8( closed_idle ) ) && !m_pCurtain->IsAnimPlaying( TPS8( closed_idle ), 0.0f ) )
		{
			m_pCurtain->PlayAnimation( TPS8( close ) );
		}
	}

	if ( m_iNumDrinkersGone < GetLevel().iNumDrinkers || m_iNumActiveDrinkers > 0 || GetNumMovingGlasses() != 0 || m_eEndState == ENDSTATE_LOST )
	{
		m_fLevelTime += a_fDeltaTime;

		if ( UpdateGlasses( a_fDeltaTime ) )
			return;

		UpdatePlayer( a_fDeltaTime );
		UpdatePlayerLook( a_fDeltaTime );
		UpdateRoot( a_fDeltaTime );
		UpdateWildMike( a_fDeltaTime );
		UpdateDrinkers( a_fDeltaTime );
	}
	else
	{
		m_iNumApproaching = 0;
		WinLevel();
	}
}

// $Barnyard: FUNCTION 004e8c50
void ATapperMiniGameState::OnInsertion()
{
	TMemory::TestMemIntegrity( TNULL );

	m_iDifficulty         = AMiniGameManager::GetSingleton()->GetMiniGameVariant();
	m_eWildMikeState      = WILDMIKESTATE_NONE;
	m_bAnyDancing         = TFALSE;
	m_bShowPlayerFloats   = TFALSE;
	m_iFillCue            = -1;
	m_iNumDrinkersGone    = 0;
	m_iNumApproaching     = 0;
	m_fSpeedScale         = 1.0f;
	m_fSwitchBarTimer     = 0.0f;
	m_eEndState           = ENDSTATE_NONE;
	m_fEndTimer           = 0.0f;
	m_iJumpDirection      = 0;
	m_iNumSpawned         = 0;
	m_iShootDoodad        = 0;
	m_bWaitForThrow       = TFALSE;
	m_iNumActiveDrinkers  = 0;
	m_iLevel              = 0;
	m_iFloatText          = 0;
	m_ePlayerState        = PLAYERSTATE_IDLE;
	m_bRunKeyHeld         = TFALSE;
	m_bRunning            = TFALSE;
	m_bTowardsEnd         = TTRUE;
	m_fLevelTime          = 0.0f;
	m_iNextSkin           = 0;
	m_bIntroStarted       = TFALSE;
	m_fBonusLifeTextTimer = 0.0f;
	m_bOverfilled         = TFALSE;

	LoadData();

	m_fFill          = m_afParams[ FLOATPARAM_FILL_MIN ];
	m_iWildMikeScore = m_aiParams[ INTPARAM_WILDMIKE_SCORE ];
	m_iNumPlayers    = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerCount();

	for ( TINT i = 0; i < NUM_BARS; i++ )
		m_aiNumBarDrinkers[ i ] = 0;

	SetupInteriorNetwork();

	m_iNumPlayers2 = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerCount();
	m_iNumTurns    = 0;

	for ( TINT i = 0; i < m_iNumPlayers2; i++ )
	{
		m_aPlayers[ i ].eTeam  = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerTeam( i );
		m_aPlayers[ i ].iUnk14 = 0;
	}

	m_iCurrentPlayer = 0;

	AGameLoader::Terrain_Set( AGameLoader::Terrain_Tappers, TFALSE, TTRUE, 0, 0, 0, 0 );
	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME_DEFAULT );
	m_oFloatHud.Create( &GetGUIElement(), "rekord26" );

	m_fSpawnTimer                  = 0.0f;
	m_iBar                         = 2;
	m_aGlassPools[ 0 ].iNumGlasses = MAX_POOL_GLASSES;

	CreateGlasses();
	CreatePlayer();
	CreateDrinkers();
	CreateHUD();
	CreateTips();
	CreateRoot();
	CreateWildMike();
	CreateTables();
	m_iUnk4F6C = 0;
	CreateCurtain();
	m_iGlassPool = 0;

	AMiniGameState::OnInsertion();

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->SetFOV( GetFOV() );

	// NOTE: calls the empty function 0054df00 with the main controller and the camera position
	StopRumble( 0, AInputHandler::GetSingleton()->GetMainController() );

	m_oTimeLeftHudElement.SetForceHide( TTRUE );

	AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
	pGameTimeFX->SetForcedFX( pGameTimeFX->FindFX( "Tapper" ) );

	AddCurtainToModelRenderer();
	ASSeqController::GetSingleton()->LoadScript( "sstap_intro.tso", TVector4::VEC_ZERO );
	m_iBonusLifePopup = 0;

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() & ~ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_FROZEN );
	ASunFlareEffect::GetSingleton()->SetEnabled( TFALSE );

	m_oLevelText.Create( T2GUIFontManager::FindFont( "Rekord26" ), &GetGUIElement(), T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oPickUpHudElement.SetNumber( 0 );
}

// $Barnyard: FUNCTION 004df1e0
TFLOAT ATapperMiniGameState::GetFOV()
{
	return FOV * 0.017453292f;
}
