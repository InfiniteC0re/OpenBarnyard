#include "pch.h"
#include <Render/TCameraObject.h>
#include "AVeggiePatchDefenderGame.h"
#include "Helpers/APlayerProgress.h"
#include "AMiniGameManager.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAlienAnimController.h"
#include "Animation/AModelRepos.h"
#include "Assets/AModelLoader.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AMilkManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/ASSeqState.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "HUD/AHudManager.h"
#include "Input/AInputMapManager.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Sound/AMusicManager.h"
#include "Sound/APositionalSoundSet.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/ASSeqController.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitNameManager.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitTransform.h"
#include "Units/ASimplePlayerControlComponent.h"
#include "Units/AVeggiePatchThief.h"
#include "Units/AVeggiePatchVeggie.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "Physics/ACollisionCommon.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Input/TInputDeviceController.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004c66d0, CreateObject 004caae0, CreateObjectInPlace 004cab40 (the constructor is inlined in both)
TDEFINE_CLASS( AVeggiePatchDefenderGame );

TPSTRING8_DECLARE( AVeggiePatchDefenderGame );
TPSTRING8_DECLARE( veggie_outside );
TPSTRING8_DECLARE( veggie );
TPSTRING8_DECLARE( veggie_raccoon );
TPSTRING8_DECLARE( player_veggie );
TPSTRING8_DECLARE( simplayer );
TPSTRING8_DECLARE( sunglasses1 );
TPSTRING8_DECLARE( goal_spawn );
TPSTRING8_DECLARE( vpd_carrot );
TPSTRING8_DECLARE( vpd_melon );
TPSTRING8_DECLARE( vpd_cabbage );
TPSTRING8_DECLARE( vpd_pumpkin );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( idle_lp2 );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( sprint_lp );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( defeat );
TPSTRING8_DECLARE( pickup_raccoon );
TPSTRING8_DECLARE( picked_up );
TPSTRING8_DECLARE( throw_raccoon );
TPSTRING8_DECLARE( thrown );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( winner_start );
TPSTRING8_DECLARE( winner_loop );
TPSTRING8_DECLARE( loser_start );
TPSTRING8_DECLARE( loser_loop );
TPSTRING8_DECLARE( fence_open );
TPSTRING8_DECLARE( fence_close );
TPSTRING8_DECLARE( passfence );
TPSTRING8_DECLARE( rac_locator );
TPSTRING8_DECLARE( avatar_locator );
TPSTRING8_DECLARE( cb_slats );
TPSTRING8_DECLARE( cb_rail );
TPSTRING8_DECLARE( fncveg03 );
TPSTRING8_DECLARE( fncveg03_02 );
TPSTRING8_DECLARE( fncveg03_03 );

// Locale strings
static constexpr TINT LOCALE_PICKUP_RACCOON = 196; // "Pick up {0}"
static constexpr TINT LOCALE_CLOSE_GATE     = 1018;
static constexpr TINT LOCALE_MILK_HITS      = 684;
static constexpr TINT LOCALE_THROWN         = 685;
static constexpr TINT LOCALE_VEGGIES_LEFT   = 686;
static constexpr TINT LOCALE_LIVES_LEFT     = 687;

// Where the snipers sit ("sniper01" - "sniper04")
static TVector4    s_aSniperPositions[ AVeggiePatchDefenderGame::NUM_SNIPERS ]; // 0x00783050
static TQuaternion s_aSniperRotations[ AVeggiePatchDefenderGame::NUM_SNIPERS ]; // 0x00783090

// NOTE: engine TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper.cpp
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// Pitch of the camera before the first game (0x00773b3c)
static TBOOL s_bFirstCameraSetup = TTRUE;

// The patch of the current game (0x00783100)
static AVeggiePatchDefenderGame::Patch* s_pPatch = TNULL;

//-----------------------------------------------------------------------------
// Fence
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004c5070
AVeggiePatchDefenderGame::Fence::Fence()
    : pData( TNULL )
    , pPatch( TNULL )
    , eState( STATE_CLOSED )
    , pModelInstance( TNULL )
    , pGate( TNULL )
    , pSlats( TNULL )
    , pRail( TNULL )
    , iUnk1c( 0 )
    , vOutside( TVector4::VEC_ZERO )
    , vInside( TVector4::VEC_ZERO )
    , vDirection( TVector4::VEC_ZERO )
    , vCentre( TVector4::VEC_ZERO )
    , iHealth( -1 )
    , pUnit( TNULL )
    , pCloseBreakpoint( TNULL )
    , iMapIcon( -1 )
    , fTimer( 0.0f )
    , vPassVelocity( TVector4::VEC_ZERO )
{
}

// $Barnyard: FUNCTION 004c5130
AVeggiePatchDefenderGame::Fence::~Fence()
{
	Destroy();
	oCloseListener.Disconnect();
}

// $Barnyard: FUNCTION 004c51a0
void AVeggiePatchDefenderGame::Fence::Create( const AVeggiePatchFenceData* a_pData, Patch* a_pPatch )
{
	if ( pModelInstance )
		return;

	pData  = a_pData;
	pPatch = a_pPatch;

	TMatrix44 oLocator;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( a_pData->szLocator, oLocator );

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	// The gates are rotated by 90 degrees
	TMatrix44 oTransform;
	oTransform.Identity();
	oTransform.RotateZ( TMath::HALF_PI );

	TMatrix44 oRotation = oTransform;
	oTransform.Multiply( oLocator, oRotation );

	iHealth = 1;

	if ( a_pData->iGateHealth <= -1 )
	{
		// A plain piece of the fence
		pModelInstance = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data\\models\\fncveg01" ), TNULL );
		pModelInstance->SetUnlit();
		pModelInstance->SetTransform( oLocator );
		pModelInstance->SetSkeletonUpdating( TTRUE );
		return;
	}

	// A gate: the posts and the animated gate with its slats and rail
	pModelInstance = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data\\models\\fncveg03_01" ), TNULL );
	pModelInstance->SetTransform( oLocator );
	pModelInstance->SetSkeletonUpdating( TTRUE );

	pGate  = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( TPS8( fncveg03 ) ), TNULL, 0 );
	pSlats = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( TPS8( fncveg03_02 ) ), TNULL, 0 );
	pObjectManager->AttachObject( pSlats, pGate, TPString8(), TPS8( cb_slats ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

	pRail = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( TPS8( fncveg03_03 ) ), TNULL, 0 );
	pObjectManager->AttachObject( pRail, pGate, TPString8(), TPS8( cb_rail ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

	pGate->GetModelInstance()->SetTransform( oTransform );
	pGate->SetSkeletonUpdating( TTRUE, TTRUE );

	TSkeletonInstance* pSkeleton = pGate->GetModelInstance()->GetSkeletonInstance();

	// Where the raccoons stand outside of the gate
	TMatrix44 oBone;
	pSkeleton->GetBoneTransformCurrent( pSkeleton->GetBoneID( "rac_locator" ), oBone );
	TMatrix44 oBoneCopy = oBone;
	oBone.Multiply( oTransform, oBoneCopy );
	vOutside = oBone.GetTranslation();

	// Where the player stands inside of it
	pSkeleton->GetBoneTransformCurrent( pSkeleton->GetBoneID( "avatar_locator" ), oBone );
	oBoneCopy = oBone;
	oBone.Multiply( oTransform, oBoneCopy );
	vInside   = oBone.GetTranslation();
	vInside.y = ATerrainInterface::GetSingleton()->GetGroundHeightTop( vInside.x, vInside.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) - 0.15f;

	vDirection.x = vOutside.x - vInside.x;
	vDirection.y = vOutside.y - vInside.y;
	vDirection.z = vOutside.z - vInside.z;
	vDirection.w = vOutside.w;

	vCentre.Multiply( vDirection, 0.5f );
	vCentre.x += vInside.x;
	vCentre.y += vInside.y;
	vCentre.z += vInside.z;

	vDirection.w = 1.0f;
	NormaliseXZ( vDirection );

	// Move both positions a bit away from the gate
	TVector4 vOffset;
	vOffset.Multiply( vDirection, 0.1f );
	vInside.x -= vOffset.x;
	vInside.y -= vOffset.y;
	vInside.z -= vOffset.z;

	vOffset.Multiply( vDirection, 0.1f );
	vOutside.x += vOffset.x;
	vOutside.y += vOffset.y;
	vOutside.z += vOffset.z;

	// The raccoons jump through it in 1.23 seconds
	vPassVelocity.x = vOutside.x - vInside.x;
	vPassVelocity.y = vOutside.y - vInside.y;
	vPassVelocity.z = vOutside.z - vInside.z;
	vPassVelocity.w = vOutside.w;
	vPassVelocity.Multiply( 0.81300813f );
}

// $Barnyard: FUNCTION 004c50e0
void AVeggiePatchDefenderGame::Fence::Destroy()
{
	if ( pModelInstance )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( pModelInstance );
		pModelInstance = TNULL;
	}

	AAnimatableObject** ppObjects = &pGate;

	for ( TINT i = 3; i != 0; i-- )
	{
		if ( *ppObjects )
		{
			AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( *ppObjects );
			*ppObjects = TNULL;
		}

		ppObjects++;
	}
}

// $Barnyard: FUNCTION 004c5010
TINT AVeggiePatchDefenderGame::Fence::GetMapIcon()
{
	if ( iMapIcon < 0 )
	{
		iMapIcon = pPatch->pMap->AddIcon( vCentre, TVector4::VEC_POSX, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_GATE, 0.07f );
		pPatch->pMap->SetIconVisible( iMapIcon, TFALSE );
	}

	return iMapIcon;
}

// $Barnyard: FUNCTION 004c5060
void AVeggiePatchDefenderGame::Fence::SetSkeletonUpdating( TBOOL a_bUpdating )
{
	pModelInstance->SetSkeletonUpdating( a_bUpdating );
}

// $Barnyard: FUNCTION 004c4f00
void AVeggiePatchDefenderGame::Fence::Hit()
{
	if ( iHealth > 0 )
		iHealth--;
}

// $Barnyard: FUNCTION 004c5c50
TBOOL AVeggiePatchDefenderGame::Fence::Open( AVeggiePatchThief* a_pThief )
{
	if ( iHealth != 0 || eState != STATE_CLOSED || pUnit )
		return TFALSE;

	eState = STATE_OPENING;

	AAnimatableObjectManager::GetSingleton()->AttachObject( a_pThief->GetAnimatableObject(), pGate, TPString8(), TPS8( rac_locator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
	pUnit = a_pThief;

	a_pThief->GetAnimatableObject()->PlayAnimation( TPS8( fence_open ) );
	return pGate->PlayAnimation( TPS8( fence_open ) );
}

// $Barnyard: FUNCTION 004c4f10
TBOOL AVeggiePatchDefenderGame::Fence::PassIn( AVeggiePatchThief* a_pThief )
{
	if ( eState != STATE_OPEN || pUnit )
		return TFALSE;

	eState = STATE_PASS_IN;
	fTimer = 0.0f;
	pUnit  = a_pThief;

	a_pThief->GetSteer()->SetPosition( vOutside );

	TVector4 vFacing( -vDirection.x, -vDirection.y, -vDirection.z, vDirection.w );
	pUnit->GetSteer()->SetDirection( vFacing );

	return TTRUE;
}

// $Barnyard: FUNCTION 004c4fb0
TBOOL AVeggiePatchDefenderGame::Fence::PassOut( AVeggiePatchThief* a_pThief )
{
	if ( eState != STATE_OPEN || pUnit )
		return TFALSE;

	eState = STATE_PASS_OUT;
	fTimer = 0.0f;
	pUnit  = a_pThief;

	a_pThief->GetSteer()->SetPosition( vInside );
	pUnit->GetSteer()->SetDirection( vDirection );

	return TTRUE;
}

// $Barnyard: FUNCTION 004c5d50
TBOOL AVeggiePatchDefenderGame::Fence::Close( AUnit* a_pPlayer )
{
	if ( iHealth != 0 || eState != STATE_OPEN || pUnit )
		return TFALSE;

	eState = STATE_CLOSING;
	fTimer = 0.75f;

	ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, a_pPlayer->FindComponent( &TGetClass( AControlComponent ) ) );
	pControl->SetMovementEnabled( TFALSE );
	pControl->SetLauncherInput( TFALSE );
	pControl->SetSecondaryLauncherInput( TFALSE );

	pUnit = a_pPlayer;

	// The gate is closed by the animation of the player
	pCloseBreakpoint = TSTATICCAST( AAnimatedUnit, a_pPlayer )->GetAnimatableObject()->GetAnimationEmitter( TPS8( fence_close ) );
	oCloseListener.Connect( pCloseBreakpoint->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnCloseAnimEvent ), 0 );

	return TTRUE;
}

// $Barnyard: FUNCTION 004c5180
TBOOL __stdcall AVeggiePatchDefenderGame::Fence::OnCloseAnimEvent( Fence* a_pThis, void* a_pOwner, void* a_pData )
{
	a_pThis->pGate->PlayAnimation( TPS8( fence_close ) );
	a_pThis->oCloseListener.Disconnect();
	return TTRUE;
}

// $Barnyard: FUNCTION 004c5700
void AVeggiePatchDefenderGame::Fence::Update( TFLOAT a_fDeltaTime, TFLOAT a_fTime )
{
	if ( iHealth < 0 )
		return;

	AGUI2MiniMap* pMap = pPatch->pMap;

	// The icon pulses
	pMap->SetIconAlpha( GetMapIcon(), ( TMath::Sin( pPatch->fTime * TMath::TWO_PI ) + 1.0f ) * 0.5f );

	switch ( eState )
	{
		case STATE_OPENING:
			if ( pGate->IsAnimPlaying( TPS8( idle_lp2 ), 0.0f ) )
			{
				// Open
				AAnimatableObjectManager::GetSingleton()->DetachObject( TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject(), pGate );
				pUnit  = TNULL;
				eState = STATE_OPEN;

				pPatch->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
				pPatch->pMap->SetIconScale( GetMapIcon(), 1.0f );
				pPatch->pMap->SetIconVisible( GetMapIcon(), TTRUE );
			}

			break;
		case STATE_CLOSING:
		{
			APlayerSteer*      pSteer  = TSTATICCAST( APlayerSteer, pUnit->GetSteer() );
			AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject();

			if ( !pObject->GetParentObject() )
			{
				// The player walks to the gate
				TVector4 vPosition;
				pSteer->CopyPosition( vPosition );

				TVector4 vToGate;
				vToGate.x        = vInside.x - vPosition.x;
				vToGate.y        = vInside.y - vPosition.y;
				vToGate.z        = vInside.z - vPosition.z;
				vToGate.w        = 0.0f;
				TFLOAT fDistance = vToGate.NormaliseMag();

				// NOTE: retail subtracts the elapsed time of the game instead of a_fDeltaTime,
				// so the player is put at the gate as soon as the game has run for more than 0.75 seconds
				fTimer -= a_fTime;

				if ( fTimer >= 0.0f && fDistance >= 0.1f )
				{
					vToGate.Multiply( pSteer->GetTraits()->m_fLogicalWalkSpeed );
					pSteer->SetDesiredVelocity( vToGate );
					return;
				}

				pSteer->SetPosition( vInside );

				if ( fTimer >= 0.0f && !pSteer->IsFacingDesired( 0.99f ) )
				{
					pSteer->StopAndFace( vToGate );
					return;
				}

				pSteer->SetDirection( vDirection );
				pSteer->Command_StandStill();

				AAnimatableObjectManager::GetSingleton()->AttachObject( pObject, pGate, TPString8(), TPS8( avatar_locator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
				TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->PlayAnimation( TPS8( fence_close ) );
			}
			else if ( pGate->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) && pObject->IsAnimationFinished( TPS8( fence_close ) ) )
			{
				// Closed
				AAnimatableObjectManager::GetSingleton()->DetachObject( pObject, pGate );
				pSteer->EndActionState();

				ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, pUnit->FindComponent( &TGetClass( AControlComponent ) ) );
				pControl->SetAIEnabledFlag( TTRUE );
				pControl->SetMovementEnabled( TTRUE );
				pControl->SetLauncherInput( TTRUE );
				pControl->SetSecondaryLauncherInput( TTRUE );

				pPatch->ePlayerState = PLAYERSTATE_FREE;
				pUnit                = TNULL;
				eState               = STATE_CLOSED;
				iHealth              = pData->iGateHealth;

				pPatch->pMap->SetIconVisible( GetMapIcon(), TFALSE );
			}

			break;
		}
		case STATE_PASS_IN:
		{
			// The raccoon jumps from the outside in
			TVector4 vPosition;
			vPosition.Multiply( vPassVelocity, -fTimer );
			vPosition.x += vOutside.x;
			vPosition.y += vOutside.y;
			vPosition.z += vOutside.z;
			pUnit->GetSteer()->SetPosition( vPosition );

			fTimer += a_fDeltaTime;

			if ( TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->IsAnimationFinished( TPS8( passfence ) ) )
			{
				pUnit->GetSteer()->SetPosition( vInside );

				TVector4 vFacing;
				vFacing.Set( vDirection );
				vFacing.x = -vFacing.x;
				vFacing.y = -vFacing.y;
				vFacing.z = -vFacing.z;
				pUnit->GetSteer()->SetDirection( vFacing );

				AVeggiePatchThief* pThief = TSTATICCAST( AVeggiePatchThief, pUnit );
				pThief->ChangeSteer( &TGetClass( AInteriorSteer ) );
				TSTATICCAST( AInteriorSteer, pThief->GetSteer() )->ConnectToNetwork( pPatch->pInsideNetwork );
				pThief->SetState( AVeggiePatchThief::STATE_SEEK_VEGGIE );

				eState = STATE_OPEN;
				pUnit  = TNULL;
			}

			break;
		}
		case STATE_PASS_OUT:
		{
			// The raccoon jumps from the inside out
			TVector4 vPosition;
			vPosition.Multiply( vPassVelocity, fTimer );
			vPosition.x += vInside.x;
			vPosition.y += vInside.y;
			vPosition.z += vInside.z;
			pUnit->GetSteer()->SetPosition( vPosition );

			fTimer += a_fDeltaTime;

			if ( TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->IsAnimationFinished( TPS8( passfence ) ) )
			{
				pUnit->GetSteer()->SetPosition( vOutside );
				pUnit->GetSteer()->SetDirection( vDirection );

				AVeggiePatchThief* pThief = TSTATICCAST( AVeggiePatchThief, pUnit );
				pThief->ChangeSteer( &TGetClass( AInteriorSteer ) );

				AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pThief->GetSteer() );
				pPatch->pOutsideNetwork->FindContainingArea( *pSteer->GetPosition() );
				pSteer->ConnectToNetwork( pPatch->pOutsideNetwork );

				if ( pThief->GetState() == AVeggiePatchThief::STATE_PASS_OUT_VEGGIE )
					pThief->SetState( AVeggiePatchThief::STATE_CARRY_VEGGIE );
				else if ( pThief->GetState() == AVeggiePatchThief::STATE_PASS_OUT )
					pThief->SetState( AVeggiePatchThief::STATE_FLEE );

				eState = STATE_OPEN;
				pUnit  = TNULL;
			}

			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Patch
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004c75a0
AVeggiePatchDefenderGame::Patch::Patch( AGUI2MiniMap* a_pMap )
    : ePlayerState( PLAYERSTATE_FREE )
    , pCarriedThief( TNULL )
    , pNearestFence( TNULL )
    , vThrowDirection( TVector4::VEC_POSX )
    , bPlayerHit( TFALSE )
    , iLives( 0 )
    , bDefeated( TFALSE )
    , pGameData( TNULL )
    , pFenceData( TNULL )
    , pInsideNetwork( TNULL )
    , pOutsideNetwork( TNULL )
    , strUnk1d4( L"Player" )
    , iNumVeggiesAtStart( 0 )
    , iNumVeggiesTaken( 0 )
    , iNumVeggiesLost( 0 )
    , iNumVeggiesSaved( 0 )
    , iNumMilkHits( 0 )
    , iNumThrown( 0 )
    , iNumFencesClosed( 0 )
    , iNumGates( 0 )
    , pGates( TNULL )
    , iMaxLost( 0 )
    , iNumInPatch( 0 )
    , iNumFalling( 0 )
    , iNumCarried( 0 )
    , iNumLost( 0 )
    , pMap( a_pMap )
    , fTime( 0.0f )
    , ePhase( PHASE_PLAYING )
    , iMainGate( -1 )
{
}

// $Barnyard: FUNCTION 004c7760
AVeggiePatchDefenderGame::Patch::~Patch()
{
}

//-----------------------------------------------------------------------------
// AVeggiePatchDefenderGame
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004ca710
AVeggiePatchDefenderGame::AVeggiePatchDefenderGame()
    : m_bUnkbc0( TFALSE )
    , m_iUnkbc4( 0 )
    , m_pPlayer( TNULL )
    , m_pSunglasses( TNULL )
    , m_pControl( TNULL )
    , m_pGateModel( TNULL )
    , m_pArrowModel( TNULL )
    , m_pArrowModel2( TNULL )
    , m_fOldFarClip( 0.0f )
    , m_bUnkcb8( TFALSE )
    , m_bUnkcb9( TFALSE )
    , m_fSpawnTimer( 0.0f )
    , m_iUnk1200( 0 )
    , m_iInteractionLine( 0 )
    , m_oPatch( &m_oMap )
    , m_fWaveTimer( 0.0f )
    , m_iWave( 0 )
    , m_pGameTimeFXManager( TNULL )
    , m_pScarecrowModel( TNULL )
    , m_bRenderReticles( TFALSE )
{
	m_aUnkca8[ 0 ]    = 0;
	m_aUnkca8[ 1 ]    = 0;
	m_fOldCameraPitch = 0.0f;
}

// $Barnyard: FUNCTION 004cab80
// $Barnyard: FUNCTION 004ca950
// $Barnyard: FUNCTION 004ca940
AVeggiePatchDefenderGame::~AVeggiePatchDefenderGame()
{
}

// $Barnyard: FUNCTION 004c6800
TBOOL AVeggiePatchDefenderGame::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	// NOTE: retail checks the class of the device and ignores the result
	a_pInputEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceController ) );

	return AGameState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 004c6c20
TBOOL AVeggiePatchDefenderGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand != AInputCommand_Interact )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( m_pPlayer->GetActiveLauncher()->GetClass()->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
		return TTRUE;

	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );
	pSteer->GetPosition();

	if ( m_oPatch.ePlayerState != PLAYERSTATE_FREE )
		return TTRUE;

	AAnimatableObject* pObject = m_pPlayer->GetAnimatableObject();

	if ( !pObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) &&
	     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) &&
	     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( sprint_lp ), 0.0f ) &&
	     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( run_lp ), 0.0f ) )
		return TTRUE;

	Fence* pFence = m_oPatch.pNearestFence;

	if ( pFence )
	{
		// Close the gate
		if ( pFence->eState == Fence::STATE_OPEN && pFence->Close( m_pPlayer ) )
		{
			m_pControl->SetMovementEnabled( TFALSE );
			m_pControl->SetLauncherInput( TFALSE );
			m_pControl->SetSecondaryLauncherInput( TFALSE );

			m_oPatch.iNumFencesClosed++;
			m_oPatch.pNearestFence = TNULL;
			m_oPatch.ePlayerState  = PLAYERSTATE_CLOSE_FENCE;
		}

		return TTRUE;
	}

	if ( m_oPatch.pCarriedThief && pSteer->GetState() == APlayerSteer::STATE_WALKING )
	{
		// Pick the raccoon up
		m_oPatch.ePlayerState = PLAYERSTATE_PICKUP_THIEF;
		m_pControl->SetMovementEnabled( TFALSE );
		m_pControl->SetLauncherInput( TFALSE );
		m_pControl->SetSecondaryLauncherInput( TFALSE );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 004c6a50
void AVeggiePatchDefenderGame::OnRender( TFLOAT a_fDeltaTime )
{
	if ( m_bRenderReticles )
		m_oReticles.UpdateMiniGame( a_fDeltaTime );

	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Render();

	m_oMilkBottles.Render();
}

// $Barnyard: FUNCTION 004c6a90
ASoundId AVeggiePatchDefenderGame::GetSound()
{
	return 23;
}

// $Barnyard: FUNCTION 004c6aa0
void AVeggiePatchDefenderGame::OnPause()
{
	m_pPlayer->SetVisible( TTRUE );
	m_pPlayer->SetMovement( TTRUE );
	m_pPlayer->SetAIEnabled( TTRUE );
	m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );
	m_oPatch.aFences[ m_oPatch.iMainGate ].SetSkeletonUpdating( TTRUE );
	m_bRenderReticles = TTRUE;
	AGameState::OnPause();
}

// $Barnyard: FUNCTION 004ca070
void AVeggiePatchDefenderGame::OnInsertion()
{
	m_bShowPlayerFloats = TFALSE;
	m_pInputMap         = AInputMapManager::GetSingleton()->FindMap( TPS8( AVeggiePatchDefenderGame ) );

	AGameLoader::Terrain_Set( AGameLoader::Terrain_VeggieDef, TFALSE, TTRUE, 0, 0, 0, -1 );

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	pTerrain->ForceSection( pTerrain->GetSectionIndex( "MeadowSouth" ), TFALSE );
	AGameLoader::Terrain_Load();

	m_fOldFarClip = ARenderer::GetSingleton()->SetFar( 800.0f );

	// The minimap
	m_oMap.Reset();

	AGUI2MiniMap::Viewport oViewport;
	oViewport.iX      = 0;
	oViewport.iY      = 0;
	oViewport.iWidth  = TINT( m_GUIElement.GetWidth() );
	oViewport.iHeight = TINT( m_GUIElement.GetHeight() );

	m_oMap.Create(
	    "Map_Veggie_Patch",
	    0.3f,
	    0.3f,
	    oViewport,
	    TVector4( 49.06f, 0.0f, 82.62f, 1.0f ),
	    TVector2( 403.0f, 321.0f ),
	    TVector4( 34.09f, 0.0f, 98.73f, 1.0f ),
	    TVector2( 251.0f, 163.0f ),
	    TTRUE,
	    TNULL,
	    TTRUE,
	    50
	);
	m_oMap.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_GUIElement.AddChildTail( m_oMap );
	m_oMap.SetZoom( 0.5f );

	LoadData();

	m_oPatch.pOutsideNetwork = AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( TPS8( veggie_outside ) );
	m_oPatch.pInsideNetwork  = AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( TPS8( veggie ) );

	m_oMilkBottles.LoadFromLocators( pTerrain->GetLocatorList() );

	CreatePlayer();
	SetupCamera();

	ASimpleProjectileManager::GetSingleton()->CreatePool( 10, ASimpleProjectileManager::TYPE_WITH_UNIT );
	ASimpleProjectileManager::GetSingleton()->CreatePool( 250, ASimpleProjectileManager::TYPE_VEGGIE_DEFENDER_MILK );
	ASimpleProjectileManager::GetSingleton()->CreatePool( 10, ASimpleProjectileManager::TYPE_VEGGIE_THROW );
	AMilkManager::Load( "Data/MilkParticles/SimMilk.trb" );

	s_pPatch = &m_oPatch;

	CreateVeggies();
	CreateFences();
	CreateModels();

	// Where the player throws the raccoons
	m_aThrowTargets[ 0 ] = TVector4( 5.0f, -50.0f, -25.0f, 1.0f );
	m_aThrowTargets[ 1 ] = TVector4( -48.0f, -50.0f, 71.0f, 1.0f );
	m_aThrowTargets[ 2 ] = TVector4( 26.0f, -32.0f, 123.0f, 1.0f );
	m_aThrowTargets[ 3 ] = TVector4( 88.0f, -31.0f, 5.0f, 1.0f );
	m_aThrowTargets[ 4 ] = TVector4( 128.0f, -31.0f, 177.0f, 1.0f );

	pTerrain->GetLocatorPosAndRot( "sniper01", s_aSniperPositions[ 0 ], s_aSniperRotations[ 0 ] );
	pTerrain->GetLocatorPosAndRot( "sniper02", s_aSniperPositions[ 1 ], s_aSniperRotations[ 1 ] );
	pTerrain->GetLocatorPosAndRot( "sniper03", s_aSniperPositions[ 2 ], s_aSniperRotations[ 2 ] );
	pTerrain->GetLocatorPosAndRot( "sniper04", s_aSniperPositions[ 3 ], s_aSniperRotations[ 3 ] );

	CreateThieves();

	m_fTimeLimit = m_oPatch.pGameData->fTimeLimit;

	m_oInteractionHud.SetForceHide( TTRUE );
	AHudManager::GetSingleton()->AddElement( &m_oInteractionHud, TFALSE, TTRUE, TFALSE );

	m_oMap.SetColours( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF );
	m_oMap.Show();

	// The raccoon warning
	m_oWarning.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oWarning.Create( T2GUITextureSectionManager::GetTextureSection( "Raccoon_Warning" ), 0.5f );
	m_oWarning.SetShown( TFALSE );
	m_oWarning.SetEffect( AGUI2ImagePopup::EFFECT_PULSE, TTRUE );
	m_GUIElement.AddChildTail( m_oWarning );

	m_oLifeHud.Create( &m_GUIElement, TNULL, 8, "Score_Wood_Thin", T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT, 1.0f, TFALSE );

	// The light goes from the start to the end of the day during the game
	// The light goes from the afternoon to the dusk during the game
	m_pGameTimeFXManager    = AGameTimeFXManager::GetSingleton();
	AGameTimeFX* pAfternoon = m_pGameTimeFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_AFTERNOON );
	m_oFXTo                 = *pAfternoon;
	m_oFXFrom               = *pAfternoon;
	m_oFXFrom.Interpolate( *pAfternoon, *m_pGameTimeFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_DUSK ), 0.0f );
	m_oFXTo.Interpolate( *m_pGameTimeFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_AFTERNOON ), *m_pGameTimeFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_DUSK ), 1.0f );
	m_pGameTimeFXManager->SetForcedFX( &m_oFXFrom );

	ASSeqController::GetSingleton()->LoadScript( "SSVEG_Intro.tso", TVector4::VEC_ZERO );
	ASSeqController::GetSingleton()->FindScript( "SSVEG_Intro", TNULL );

	AModelLoader::ResolveTextures();
	AMiniGameState::OnInsertion();
}

// $Barnyard: FUNCTION 004ca510
void AVeggiePatchDefenderGame::OnRemoval()
{
	APositionalSoundSet::GetSingleton()->Unload();
	APlayerManager::GetSingleton()->SetCurrentPlayerSet( 0 );
	ARenderer::GetSingleton()->SetFar( m_fOldFarClip );

	ACameraManager*        pCameraManager = ACameraManager::GetSingleton();
	AGTAStyleCameraHelper* pHelper        = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );
	pHelper->SetPitch( m_fOldCameraPitch );
	pHelper->SetDefaultDistance( 5.0f );
	pHelper->RemoveCollisionFlag( 1 );
	pCameraManager->ResetHelperTargets();
	pCameraManager->DetachSplitscreenCameraHelpers();
	pCameraManager->DetachCameraHelpers();

	if ( AGameLoader::IsStoryMode() )
		AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );

	m_oMilkBottles.Clear();
	DestroyPlayer();
	ASimpleProjectileManager::GetSingleton()->DestroyAllPools();

	if ( AMilkManager::GetSingleton() )
		delete AMilkManager::GetSingleton();

	s_pPatch = TNULL;

	DestroyThieves();
	DestroyVeggies();
	DestroyFences();
	DestroyModels();

	m_oAmmoBar.RemoveSelf();
	m_oAmmoBacking.RemoveSelf();
	m_oAmmoRoot.RemoveSelf();
	m_oVeggiesText.RemoveSelf();
	m_oVeggiesBacking.RemoveSelf();
	m_oVeggiesIcon.RemoveSelf();
	m_oVeggiesRoot.RemoveSelf();
	m_oInteractionHud.Detach();

	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 004c7a90
void AVeggiePatchDefenderGame::OnActivate()
{
	SetupCamera();
	m_oInteractionHud.SetMask( AHudElement::Mask( m_oInteractionHud.GetMask() | AHudElement::MASK_CONTEXT_BUTTON ) );
	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 004c6830
void AVeggiePatchDefenderGame::OnDeactivate()
{
	m_oInteractionHud.SetMask( AHudElement::Mask( m_oInteractionHud.GetMask() & ~AHudElement::MASK_CONTEXT_BUTTON ) );
	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 004c74d0
void AVeggiePatchDefenderGame::Pause()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSVEG_Intro", "SSVEG_Intro", "SSVEG_VicAndDef_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004c73b0
void AVeggiePatchDefenderGame::PlayLoseSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSVEG_Defeat", "SSVEG_Defeat", "SSVEG_VicAndDef_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004c7440
void AVeggiePatchDefenderGame::PlayWinSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSVEG_Victory", "SSVEG_Victory", "SSVEG_VicAndDef_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004c6b10
void AVeggiePatchDefenderGame::OnPreStart()
{
	m_pPlayer->SetVisible( TFALSE );
	m_pPlayer->SetMovement( TFALSE );
	m_pPlayer->SetAIEnabled( TFALSE );
	m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
	m_oPatch.aFences[ m_oPatch.iMainGate ].SetSkeletonUpdating( TFALSE );
	m_bRenderReticles = TFALSE;
}

// $Barnyard: FUNCTION 004c6840
TBOOL AVeggiePatchDefenderGame::IsGameOver()
{
	return m_fElapsedTime >= m_fTimeLimit || m_oPatch.iNumLost >= m_oPatch.iMaxLost || m_oPatch.iLives <= 0;
}

// $Barnyard: FUNCTION 004c6880
void AVeggiePatchDefenderGame::UpdateAlways( TFLOAT a_fDeltaTime )
{
	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Update( a_fDeltaTime );

	for ( TINT i = 0; i < m_oPatch.iNumGates; i++ )
		m_oPatch.aFences[ m_oPatch.pGates[ i ] ].Update( a_fDeltaTime, m_fElapsedTime );

	ASteer* pSteer = m_pPlayer->GetSteer();
	m_oMap.SetCentre( *pSteer->GetPosition(), *pSteer->GetDirection() );
}

// $Barnyard: FUNCTION 004c91b0
void AVeggiePatchDefenderGame::UpdateGame( TFLOAT a_fDeltaTime )
{
	m_bRenderReticles = TTRUE;
	m_oMilkBottles.Update( a_fDeltaTime, TTRUE );

	if ( m_oPatch.ePhase == PHASE_ENDING )
	{
		UpdatePlayer();
		m_fElapsedTime = TMath::Min( m_fElapsedTime, m_fTimeLimit + TMath::TFLOAT_EPSILON );
		return;
	}

	if ( m_oPatch.ePhase == PHASE_ENDED )
	{
		m_fElapsedTime = TMath::Min( m_fElapsedTime, m_fTimeLimit + TMath::TFLOAT_EPSILON );
		return;
	}

	m_fWaveTimer -= a_fDeltaTime;
	m_oPatch.fTime += a_fDeltaTime;

	// The day goes by
	AGameTimeFX oFX( m_oFXFrom );
	oFX.Interpolate( m_oFXFrom, m_oFXTo, TMath::Min( m_fElapsedTime / m_oPatch.pGameData->fTimeLimit, 1.0f ) );
	m_pGameTimeFXManager->SetForcedFX( &oFX );

	if ( m_fWaveTimer < 0.0f )
	{
		// The next wave
		m_iWave++;
		m_iWave *= ( m_iWave < m_oPatch.pGameData->iNumWaves );
		StartWave( &m_oPatch.pGameData->pWaves[ m_iWave ] );
	}

	const AVeggiePatchWaveData* pWave = &m_oPatch.pGameData->pWaves[ m_iWave ];

	if ( pWave->eType == WAVE_SPAWN )
	{
		m_fSpawnTimer -= a_fDeltaTime;

		if ( m_fSpawnTimer < 0.0f )
		{
			if ( !m_oPatch.oFreeThieves.IsEmpty() )
			{
				for ( TINT i = 0; i < pWave->iCount; i++ )
				{
					if ( m_oPatch.oFreeThieves.IsEmpty() )
						break;

					m_oPatch.oFreeThieves.Head()->SetState( AVeggiePatchThief::STATE_SPAWN );
				}
			}

			m_fSpawnTimer = m_oPatch.pGameData->pWaves[ m_iWave ].fDuration;
		}
	}
	else if ( pWave->eType == WAVE_FLEE )
	{
		T2_FOREACH( m_oPatch.oThieves, it )
		{
			switch ( it->GetState() )
			{
				case AVeggiePatchThief::STATE_CARRY_VEGGIE:
				case AVeggiePatchThief::STATE_WAIT:
				case AVeggiePatchThief::STATE_PICKUP:
				case AVeggiePatchThief::STATE_REMOVED:
				case AVeggiePatchThief::STATE_MILKED:
				case AVeggiePatchThief::STATE_SLINGSHOT_FIRE:
				case AVeggiePatchThief::STATE_SLINGSHOT_GETOFF:
				case AVeggiePatchThief::STATE_PICKED_UP:
				case AVeggiePatchThief::STATE_THROWN:
				case AVeggiePatchThief::STATE_PASS_IN:
				case AVeggiePatchThief::STATE_PASS_OUT_VEGGIE:
				case AVeggiePatchThief::STATE_PASS_OUT:
					break;
				case AVeggiePatchThief::STATE_SLINGSHOT_GETON:
				case AVeggiePatchThief::STATE_SLINGSHOT_AIM:
					it->SetState( AVeggiePatchThief::STATE_SLINGSHOT_GETOFF );
					break;
				default:
					if ( it->GetState() != AVeggiePatchThief::STATE_FLEE )
						it->SetState( AVeggiePatchThief::STATE_FLEE );
					break;
			}
		}
	}
	else if ( pWave->eType == WAVE_SNIPERS )
	{
		if ( m_fWaveTimer >= 1.0f )
		{
			// The wave is over once the snipers are gone
			TBOOL bNoSnipers = TTRUE;

			T2_FOREACH( m_oPatch.oThieves, it )
			{
				if ( it->GetState() >= AVeggiePatchThief::STATE_SLINGSHOT_GETON && it->GetState() <= AVeggiePatchThief::STATE_SLINGSHOT_FIRE )
				{
					bNoSnipers = TFALSE;
					break;
				}
			}

			if ( bNoSnipers )
				m_fWaveTimer = 0.0f;
		}
		else
		{
			T2_FOREACH( m_oPatch.oThieves, it )
			{
				if ( it->GetState() >= AVeggiePatchThief::STATE_SLINGSHOT_GETON && it->GetState() <= AVeggiePatchThief::STATE_SLINGSHOT_AIM )
					it->SetState( AVeggiePatchThief::STATE_SLINGSHOT_GETOFF );
			}
		}
	}

	// Aim the launcher along the slope of the ground
	ASteer*  pSteer    = m_pPlayer->GetSteer();
	TVector4 vPosition = *pSteer->GetPosition();
	TVector4 vForward  = *pSteer->GetDirection();
	TFLOAT   fDistance = 10.0f;

	ACollisionModelRayHitList oHits;

	if ( ATerrainInterface::GetSingleton()->CollideRay( vPosition, TVector4::VEC_POSY, fDistance, &oHits, TCollisionCommon::TOSHICGROUPFLAG( 0 ), TFALSE ) )
	{
		TVector4 vNormal = oHits.aHits[ 0 ].GetNormal();
		TFLOAT   fAngle  = TMath::ACos( TVector4::DotProduct3( vNormal, vForward ) ) - TMath::HALF_PI;
		TMath::Clip( fAngle, -0.17453292f, 0.17453292f );
		m_pPlayer->GetActiveLauncher()->SetVerticalAngle( fAngle );
	}
	else
	{
		m_pPlayer->GetActiveLauncher()->SetVerticalAngle( 0.0f );
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	DevAutoPlay( a_fDeltaTime );
#endif

	UpdatePlayer();
	UpdateInteraction();

	AProjectileLauncher* pLauncher = m_pPlayer->GetActiveLauncher();
	pLauncher->Update( a_fDeltaTime );
	m_oAmmoBar.SetAmmo( TINT( pLauncher->GetAmmo()->GetAmmo() ), pLauncher->GetAmmo()->GetMaxAmmo() );
}

// $Barnyard: FUNCTION 004c9740
void AVeggiePatchDefenderGame::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	if ( m_oPatch.ePhase == PHASE_ENDING )
	{
		if ( m_oPatch.ePlayerState != PLAYERSTATE_FREE )
			return;

		// Back to the default animations
		AAnimatableObject*        pObject     = m_pPlayer->GetAnimatableObject();
		ACharacterAnimController* pController = TSTATICCAST( ACharacterAnimController, pObject->FindAnimController( &TGetClass( AAlienAnimController ) ) );
		pObject->GetToshiAnimInterface().SetAnimationSet( pController->GetAnimationSet() );

		if ( m_pPlayer->GetActiveLauncher() != m_pPlayer->GetLauncher( 0 ) )
			m_pPlayer->SelectLauncher( TFALSE );

		m_pControl->SetLauncherInput( TFALSE );
		m_pControl->SetSecondaryLauncherInput( TFALSE );
		m_pControl->SetCanSprint( TFALSE );
		m_pControl->SetMovementEnabled( TFALSE );
		m_pControl->SetCameraInput( TFALSE );
		m_pPlayer->GetSteer()->SetTilt( 0.0f, 0.0f, TFALSE );

		m_fWaveTimer    = 5.0f;
		m_oPatch.ePhase = PHASE_ENDED;
	}
	else if ( m_oPatch.ePhase == PHASE_ENDED )
	{
		if ( m_oPatch.iLives > 0 )
		{
			AAnimatableObject* pObject = m_pPlayer->GetAnimatableObject();

			if ( !pObject->IsAnimPlaying( TPS8( winner_start ), 0.0f ) &&
			     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( winner_loop ), 0.0f ) &&
			     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( loser_start ), 0.0f ) &&
			     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( loser_loop ), 0.0f ) )
			{
				// Turn to the camera, then cheer or sulk
				ASteer*  pSteer = m_pPlayer->GetSteer();
				TVector4 vToCamera;
				vToCamera.Substract( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), *pSteer->GetPosition() );
				NormaliseXZ( vToCamera );

				if ( TVector4::DotProduct3( *m_pPlayer->GetSteer()->GetDirection(), vToCamera ) <= 0.98f )
				{
					TSTATICCAST( ACommonSteer, m_pPlayer->GetSteer() )->StopAndFace( vToCamera );
				}
				else if ( m_oPatch.iNumLost < m_oPatch.iMaxLost )
				{
					m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( winner_start ) );
				}
				else
				{
					m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( loser_start ) );
				}
			}
		}
		else if ( TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->GetState() == APlayerSteer::STATE_WALKING && !m_oPatch.bDefeated )
		{
			if ( !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( defeat ), 0.0f ) )
			{
				m_oPatch.bDefeated = TTRUE;
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( defeat ) );
			}
		}

		if ( ( m_fElapsedTime < m_fTimeLimit || m_oTimeLeftHudElement.GetTimer().IsTickComplete() ) && m_fWaveTimer < 0.0f )
			AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );

		m_fWaveTimer -= a_fDeltaTime;
	}
	else
	{
		// The game has just ended: the raccoons run away
		HideInteraction();

		T2_FOREACH( m_oPatch.oThieves, it )
		{
			if ( it->GetState() != AVeggiePatchThief::STATE_FLEE )
				it->SetState( AVeggiePatchThief::STATE_FLEE );
		}

		if ( m_pPlayer->GetActiveLauncher() != m_pPlayer->GetLauncher( 0 ) )
			m_pPlayer->SelectLauncher( TFALSE );

		m_pControl->SetLauncherInput( TFALSE );
		m_pControl->SetSecondaryLauncherInput( TFALSE );
		m_pControl->SetCanSprint( TFALSE );
		m_pControl->SetMovementEnabled( TFALSE );
		m_pControl->SetCameraInput( TFALSE );
		m_pPlayer->GetSteer()->SetTilt( 0.0f, 0.0f, TFALSE );

		m_oPatch.ePhase = PHASE_ENDING;
	}
}

// $Barnyard: FUNCTION 004c8170
AMiniGameState::S1* AVeggiePatchDefenderGame::GetResults()
{
	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];
	rInfo.bUnk1       = TTRUE;
	rInfo.iUnk2       = m_oPatch.iMaxLost - m_oPatch.iNumLost;

	if ( rInfo.iUnk2 < 0 || m_oPatch.iMaxLost == m_oPatch.iNumLost || m_oPatch.ePhase != PHASE_ENDED || m_oPatch.iLives <= 0 )
		m_oResults.iResult = -1;
	else
		m_oResults.iResult = 0;

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.bWon           = m_oResults.iResult == 0;

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_MILK_HITS ), m_oPatch.iNumMilkHits } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_THROWN ), 10 * m_oPatch.iNumThrown } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_VEGGIES_LEFT ), 20 * rInfo.iUnk2 } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_LIVES_LEFT ), 20 * m_oPatch.iLives } );
	m_oResults.ComputeTotal( 0 );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;
		TINT iVariant      = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

		if ( iVariant == 0 || TUINT( iVariant - 1 ) <= 1 )
			AHighScoreManager::AddScore( HIGHSCORE_TABLE, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 004c7ab0
void AVeggiePatchDefenderGame::OnStartHudFinished()
{
	AMiniGameState::OnStartHudFinished();

	AGUI2Timer& rTimer = m_oTimeLeftHudElement.GetTimer();

	// The milk ammo, next to the clock
	m_oAmmoBar.Create( 32.5f, 1.625f, 0.3f );
	m_oAmmoBar.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oAmmoBar.SetTransform( rTimer.GetWidth(), 0.0f, 0.0f );
	m_oAmmoBacking.AddChildTail( m_oAmmoBar );
	m_oAmmoBacking.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" ) );
	m_oAmmoBacking.SetWidth( m_oAmmoBar.GetWidth() + rTimer.GetWidth() + 26.0f );
	m_oAmmoBacking.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oAmmoBacking.SetTransform( 10.0f, 0.0f, 0.0f );

	TFLOAT fHeight = rTimer.GetHeight();
	m_oAmmoRoot.SetDimensions( m_oAmmoBacking.GetWidth(), fHeight );
	m_oAmmoRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oAmmoRoot.AddChildTail( m_oAmmoBacking );
	m_GUIElement.AddChildTail( m_oAmmoRoot );

	// The number of veggies the raccoons can still take
	m_oVeggiesBacking.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" ) );
	m_oVeggiesBacking.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	T2GUITextureSection* pVeggies = T2GUITextureSectionManager::GetTextureSection( "veggies" );
	m_oVeggiesIcon.SetDimensions( TFLOAT( TMath::Abs( pVeggies->GetPoint2X() - pVeggies->GetPoint1X() ) ), TFLOAT( TMath::Abs( pVeggies->GetPoint2Y() - pVeggies->GetPoint1Y() ) ) );
	m_oVeggiesIcon.SetTextureSection( pVeggies );
	m_oVeggiesIcon.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );

	fHeight = m_oVeggiesIcon.GetHeight();
	m_oVeggiesRoot.SetDimensions( m_oVeggiesBacking.GetWidth(), fHeight );
	m_oVeggiesRoot.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_oVeggiesRoot.AddChildTail( m_oVeggiesBacking );
	m_oVeggiesRoot.AddChildTail( m_oVeggiesIcon );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	TStringManager::String16Format( m_oPatch.wszLeft, TARRAYSIZE( m_oPatch.wszLeft ), L"%d", m_oPatch.iMaxLost - m_oPatch.iNumLost );
	m_oVeggiesText.Create( pFont, pFont->GetTextWidth( m_oPatch.wszLeft, 1.0f ) );
	m_oVeggiesText.SetText( m_oPatch.wszLeft );
	m_oVeggiesText.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oVeggiesText.SetTransform( 10.0f, 0.0f, 0.0f );
	m_oVeggiesRoot.AddChildTail( m_oVeggiesText );
	m_GUIElement.AddChildTail( m_oVeggiesRoot );

	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME | AHudElement::MASK_UNK4 | AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK8 | AHudElement::MASK_UNK11 | AHudElement::MASK_UNK13 | AHudElement::MASK_UNK14 | AHudElement::MASK_UNK26 );
	m_oTimeLeftHudElement.SetForceHide( TFALSE );

	// Full milk
	ALaunchAmmo* pAmmo = m_pPlayer->GetActiveLauncher()->GetAmmo();
	pAmmo->SetAmmo( m_pPlayer->GetActiveLauncher()->GetAmmo()->GetMaxAmmo() );

	m_pControl->SetMovementEnabled( TTRUE );

	m_oPatch.iNumVeggiesAtStart = 0;
	m_oPatch.iNumVeggiesTaken   = 0;
	m_oPatch.iNumVeggiesLost    = 0;
	m_oPatch.iNumVeggiesSaved   = 0;
	m_oPatch.iNumMilkHits       = 0;
	m_oPatch.iNumThrown         = 0;
	m_oPatch.iNumFencesClosed   = 0;
	m_oPatch.iNumVeggiesAtStart = m_oPatch.iNumInPatch;

	m_oPatch.iLives = m_oPatch.pGameData->iLives;
	m_oLifeHud.SetLives( m_oPatch.iLives );
	m_oPatch.bDefeated = TFALSE;

	StartWave( m_oPatch.pGameData->pWaves );

	m_oPatch.ePhase = PHASE_PLAYING;
	m_bUnkcb9       = TFALSE;
}

// $Barnyard: FUNCTION 004c9b90
void AVeggiePatchDefenderGame::OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	if ( a_pEvent->pUnitProxy != m_pPlayer->GetSelfProxy() )
	{
		// An animation of a raccoon
		T2_FOREACH( m_oPatch.oThieves, it )
		{
			AUnit* pUnit = a_pEvent->pUnitProxy ? a_pEvent->pUnitProxy->GetUnit() : TNULL;

			if ( pUnit == it )
				it->OnUnitAction( a_pEvent );
		}

		return;
	}

	if ( a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_HD_THROW )
	{
		if ( m_oPatch.ePlayerState != PLAYERSTATE_CARRY_THIEF )
			return;

		// Throw the raccoon towards the throw target
		AVeggiePatchThief* pThief = m_oPatch.pCarriedThief;
		pThief->SetState( AVeggiePatchThief::STATE_THROWN );
		m_pPlayer->DetachUnit( pThief, TTRUE, TTRUE );

		TMatrix44         oBone;
		AModelInstanceRef pModelInstance = m_pPlayer->GetAnimatableObject()->GetModelInstance();
		pModelInstance->GetBoneTransform( pModelInstance->GetBoneIndex( TPS8( cb_rlocator ) ), oBone );

		TVector4 vPosition = oBone.GetTranslation();
		m_oPatch.pCarriedThief->GetSteer()->SetPosition( vPosition );

		TVector4 vVelocity( m_vThrowTarget.x - vPosition.x, m_vThrowTarget.y - vPosition.y, m_vThrowTarget.z - vPosition.z, 1.0f );
		TFLOAT   fDistance = vVelocity.NormaliseMag();
		TFLOAT   fSpeed    = fDistance + fDistance;

		TVector4 vDirection = vVelocity;
		NormaliseXZ( vDirection );
		m_oPatch.pCarriedThief->GetSteer()->SetDirection( vDirection );

		fSpeed = TMath::Min( fSpeed, 100.0f );
		vVelocity.Multiply( fSpeed );
		m_oPatch.pCarriedThief->m_vVelocity = vVelocity;

		m_oPatch.pCarriedThief = TNULL;
		m_oPatch.ePlayerState  = PLAYERSTATE_THROW_THIEF;
	}
	else if ( a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_PICKUP )
	{
		if ( m_oPatch.ePlayerState != PLAYERSTATE_CARRY_THIEF )
			return;

		// Pick the throw target the player is facing the most
		APlayerSteer* pSteer   = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );
		TFLOAT        fBestDot = -1.0f;
		TINT          iBest    = 0;

		TVector4 vPosition;
		TVector4 vForward;
		pSteer->CopyPosition( vPosition );
		pSteer->CopyDirection( vForward );

		TVector4 vBestDirection;

		for ( TINT i = 0; i < NUM_THROW_TARGETS; i++ )
		{
			TVector4 vDirection( m_aThrowTargets[ i ].x - vPosition.x, m_aThrowTargets[ i ].y - vPosition.y, m_aThrowTargets[ i ].z - vPosition.z, 1.0f );
			NormaliseXZ( vDirection );

			TFLOAT fDot = TVector4::DotProduct3( vDirection, vForward );

			if ( fDot > fBestDot )
			{
				fBestDot       = fDot;
				vBestDirection = vDirection;
				iBest          = i;
			}
		}

		m_oPatch.vThrowDirection = vBestDirection;
		m_vThrowTarget           = m_aThrowTargets[ iBest ];

		m_pControl->SetMovementEnabled( TFALSE );
		m_pControl->SetLauncherInput( TFALSE );
		m_pControl->SetSecondaryLauncherInput( TFALSE );

		pSteer->SetSpeed( 0.0f );
		pSteer->StopAndFace( vBestDirection );

		m_pPlayer->AttachUnit( m_oPatch.pCarriedThief, TPS8( cb_rlocator ) );
		m_oPatch.pCarriedThief->SetState( AVeggiePatchThief::STATE_PICKED_UP );
		m_oPatch.iNumThrown++;
	}
}

// $Barnyard: FUNCTION 004c7ec0
TBOOL AVeggiePatchDefenderGame::OnTomatoHit( TRef<AUnitProxy> a_pUnit )
{
	if ( !a_pUnit )
		return TFALSE;

	AUnit* pUnit = a_pUnit->GetUnit();

	if ( pUnit->GetClass()->IsA( &TGetClass( AVeggiePatchThief ) ) )
	{
		// A sniper has hit another raccoon
		T2_FOREACH( m_oPatch.oThieves, it )
		{
			if ( pUnit == it )
				return it->SetState( AVeggiePatchThief::STATE_MILKED );
		}

		return TTRUE;
	}

	if ( pUnit != m_pPlayer )
		return TTRUE;

	if ( m_oPatch.ePlayerState == PLAYERSTATE_HIT )
		return TTRUE;

	if ( m_oPatch.ePlayerState != PLAYERSTATE_FREE )
	{
		// The player is busy, he'll be hit afterwards
		m_oPatch.bPlayerHit = TTRUE;
		return TTRUE;
	}

	if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) ||
	     m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( defeat ), 0.0f ) )
		return TTRUE;

	// Lose a life
	if ( --m_oPatch.iLives < 0 )
		m_oPatch.iLives = 0;
	else
		m_oLifeHud.AddLives( -1 );

	if ( TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->GetState() == APlayerSteer::STATE_WALKING )
	{
		m_pPlayer->GetAnimatableObject()->KillAllAnimations();

		if ( m_oPatch.iLives > 0 )
		{
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
		}
		else
		{
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( defeat ) );
			m_oPatch.bDefeated = TTRUE;
		}

		if ( m_pPlayer->GetActiveLauncher() != m_pPlayer->GetLauncher( 0 ) )
			m_pPlayer->SelectLauncher( TFALSE );

		m_pControl->SetMovementEnabled( TFALSE );
		m_pControl->SetLauncherInput( TFALSE );
		m_pControl->SetSecondaryLauncherInput( TFALSE );

		m_oPatch.ePlayerState = PLAYERSTATE_HIT;
		m_oPatch.bPlayerHit   = TFALSE;
	}
	else
	{
		m_oPatch.bPlayerHit = TTRUE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 004c6b80
void AVeggiePatchDefenderGame::LoadData()
{
	TTRB& rData = m_oPatch.oData;
	rData.Load( "Data/MinigameData/VeggiePatchDefender/Data.trb", 0 );

	TINT iNumVariants = 1;

	m_oPatch.pThiefAnimData = rData.CastSymbol<AVeggiePatchThiefAnimData>( "ThiefAnimData" );
	m_oPatch.pGameData      = rData.CastSymbol<AVeggiePatchGameData>( "GameData" );

	if ( rData.GetSymbolAddress( "NumGameVariants" ) )
		iNumVariants = *rData.CastSymbol<TINT>( "NumGameVariants" );

	m_oPatch.pGameData += AMiniGameManager::GetSingleton()->GetMiniGameVariant() % iNumVariants;
	m_oPatch.pThiefParams = m_oPatch.pGameData->pThiefParams;
	m_oPatch.pFenceData   = m_oPatch.pGameData->pFences;

#ifdef BARNYARD_LATEST
	// Latest version (Wii 80201708): a shorter game and 2 more lives on Easy
	if ( APlayerProgress::GetSingleton()->IsEasyDifficulty() )
	{
		// NOTE: the data is in the TRB loaded above, which the game owns
		AVeggiePatchGameData* pGameData = const_cast<AVeggiePatchGameData*>( m_oPatch.pGameData );
		pGameData->fTimeLimit *= 0.9f;

		if ( pGameData->iLives != -1 )
			pGameData->iLives += 2;
	}
#endif
}

// $Barnyard: FUNCTION 004c8b10
void AVeggiePatchDefenderGame::CreatePlayer()
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

	if ( pPlayerManager->GetCurrentPlayerSet() != pPlayerManager->GetPlayerSet( 1 ) )
		pPlayerManager->SetCurrentPlayerSet( 1 );

	TInputDeviceController* pController = AInputHandler::GetSingleton()->GetMainController();
	pPlayerManager->GetPlayerSet( 1 )->Reset();
	pPlayerManager->GetPlayerSet( 1 );
	pPlayerManager->GetCurrentPlayerSet()->AddHumanPlayer( pController, -1 );

	if ( !m_pPlayer )
	{
		APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();
		pProfileManager->GetMainProfile()->SetStaminaWasteSpeed( 0.0f );

		TINT iGender = pProfileManager->GetMainGender();

		if ( iGender == APlayerProfile::GENDER_NONE )
			iGender = APlayerProfile::GENDER_MALE;

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, iGender, -1, TTRUE );
		m_pPlayer = TSTATICCAST( AUnitPlayer, AUnitManager::GetSingleton()->SpawnUnit( TPS8( player_veggie ), &oTransform ) );
		m_pPlayer->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );

		m_oPatch.oPlayer = m_pPlayer->GetSelfProxy();

		AUnitNameManager::GetSingleton()->SetUnitName( TPS8( simplayer ), m_pPlayer->GetId() );

		if ( AGameLoader::IsStoryMode() )
			m_pPlayer->ApplyMaterial( APlayerProfileManager::GetSingleton()->GetMainCostume(), TTRUE );

		// The player wears sunglasses
		AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
		AAnimatableObjectType*    pType          = pObjectManager->FindType( TPS8( sunglasses1 ) );

		if ( pType )
		{
			m_pSunglasses = pObjectManager->CreateAnimatableObject( pType, m_pPlayer, 0 );
			TINT iBone    = -1;

			if ( pProfileManager->GetMainGender() == APlayerProfile::GENDER_FEMALE )
				iBone = m_pSunglasses->GetModelInstance()->GetSkeleton()->GetBoneID( "americanf_loc", 0 );

			TINT iHeadBone = m_pPlayer->GetModelInstance()->GetSkeleton()->GetBoneID( "c_head", 0 );
			pObjectManager->AttachObject( m_pSunglasses, m_pPlayer->GetAnimatableObject(), iBone, iHeadBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		}
	}

	m_pPlayer->SelectLauncher( TFALSE );

	// The milk and the raccoons the player throws
	AProjectileLauncher* pLauncher = m_pPlayer->GetActiveLauncher();
	pLauncher->GetAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_VEGGIE_DEFENDER_MILK );
	m_pPlayer->GetActiveLauncher()->SetThrowAmmo( &m_oAmmo );
	m_pPlayer->GetActiveLauncher()->GetThrowAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_VEGGIE_THROW );

	m_oReticles.Create( m_pPlayer, 50.0f, &m_GUIElement );

	m_pControl = TSTATICCAST( ASimplePlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );
	m_pControl->SetMovementEnabled( TFALSE );
	m_pControl->SetUnk32( TFALSE );

	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_oPatch.oPlayer->GetUnit()->GetSteer() );
	pSteer->PlaceAtNamedGoal( TPS8( goal_spawn ), TFALSE, TTRUE );

	ASteer* pPlayerSteer = m_pPlayer->GetSteer();
	m_oMap.AddIcon( *pPlayerSteer->GetPosition(), *pPlayerSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_PLAYER, AGUI2MiniMap::ICON_PLAYER, 0.1f );
	m_oMap.SetCentre( *m_pPlayer->GetSteer()->GetPosition(), *m_pPlayer->GetSteer()->GetDirection() );

	m_pPlayer->SetNoQuery( TFALSE );
	m_pPlayer->SetHashed( TTRUE );
}

// $Barnyard: FUNCTION 004c8ef0
void AVeggiePatchDefenderGame::DestroyPlayer()
{
	if ( m_pSunglasses )
	{
		if ( m_pPlayer )
			AAnimatableObjectManager::GetSingleton()->DetachObject( m_pSunglasses, m_pPlayer->GetAnimatableObject() );

		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pSunglasses );
		m_pSunglasses = TNULL;
	}

	AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( TPS8( simplayer ) );

	if ( pUnit )
	{
		AUnitNameManager::GetSingleton()->RemoveName( TPS8( simplayer ) );
		AUnitManager::GetSingleton()->DestroyUnit( pUnit );
		m_pPlayer = TNULL;
	}

	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetStaminaWasteSpeed( 0.8f );
}

// $Barnyard: FUNCTION 004c7970
void AVeggiePatchDefenderGame::SetupCamera()
{
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->SetSplitscreenCameraHelper( 1 );

	AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );

	if ( s_bFirstCameraSetup )
	{
		m_fOldCameraPitch   = pHelper->GetDefaultPitch();
		s_bFirstCameraSetup = TFALSE;
	}

	pHelper->SetTarget( m_pPlayer->GetSelfProxy() );
	pHelper->SetTargetPitch( 20.0f );
	pHelper->SetDefaultPitch( 20.0f );
	pHelper->SetDefaultDistance( 7.0f );
	pHelper->AddCollisionFlag( 1 );
	pHelper->ResetPosition( TTRUE );
}

// $Barnyard: FUNCTION 004c66e0
void AVeggiePatchDefenderGame::CreateFences()
{
	// Count the gates
	TINT iNumGates = 0;

	for ( TINT i = 0; i < NUM_FENCES; i++ )
	{
		if ( m_oPatch.pFenceData[ i ].iGateHealth > -1 )
			iNumGates++;
	}

	m_oPatch.pGates    = TSTATICCAST( TINT, TMalloc( iNumGates * sizeof( TINT ) ) );
	m_oPatch.iNumGates = 0;

	for ( TINT i = 0; i < NUM_FENCES; i++ )
	{
		if ( TStringManager::String8Compare( m_oPatch.pFenceData[ i ].szLocator, "fncveg01_17" ) == 0 )
			m_oPatch.iMainGate = i;

		m_oPatch.aFences[ i ].Create( &m_oPatch.pFenceData[ i ], &m_oPatch );

		if ( m_oPatch.pFenceData[ i ].iGateHealth > -1 )
			m_oPatch.pGates[ m_oPatch.iNumGates++ ] = i;
	}
}

// $Barnyard: FUNCTION 004c67c0
void AVeggiePatchDefenderGame::DestroyFences()
{
	for ( TINT i = 0; i < NUM_FENCES; i++ )
		m_oPatch.aFences[ i ].Destroy();

	if ( m_oPatch.pGates )
	{
		TFree( m_oPatch.pGates );
		m_oPatch.pGates = TNULL;
	}
}

// $Barnyard: FUNCTION 004c8370
void AVeggiePatchDefenderGame::CreateVeggies()
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	m_oPatch.iMaxLost           = 0;

	for ( TINT i = 0; i < NUM_VEGGIE_KINDS; i++ )
	{
		const AVeggiePatchGameData* pGameData = m_oPatch.pGameData;
		m_oPatch.iMaxLost += pGameData->aNumVeggies[ i ];

		if ( pGameData->aNumVeggies[ i ] == 0 )
		{
			m_oPatch.apSlots[ i ] = TNULL;
			continue;
		}

		m_oPatch.apSlots[ i ] = new VeggieSlot[ pGameData->aNumVeggies[ i ] ];

		for ( TUINT j = 0; j < m_oPatch.pGameData->aNumVeggies[ i ]; j++ )
		{
			VeggieSlot*             pSlot = &m_oPatch.apSlots[ i ][ j ];
			AVeggiePatchVeggieData* pData = &m_oPatch.pGameData->apVeggies[ i ][ j ];

			pData->iKind    = i;
			pData->pLocator = pTerrain->FindLocator( pData->szLocator );
			pSlot->pData    = pData;

			pTerrain->GetLocatorTransform( pData->szLocator, pSlot->oTransform );
			pSlot->vPosition = pSlot->oTransform.GetTranslation();

			AUnitTransform oTransform( pSlot->vPosition, TVector4::VEC_POSX, 0, -1, -1, TTRUE );
			AUnit*         pUnit = TNULL;

			switch ( i )
			{
				case VEGGIE_SPROUT:
					pSlot->pModelInstance = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data\\models\\sprout" ), TNULL );
					pSlot->pModelInstance->SetSkeletonUpdating( TTRUE );
					pSlot->pModelInstance->SetTransform( pSlot->oTransform );
					break;
				case VEGGIE_CARROT:
					pUnit = AUnitManager::GetSingleton()->SpawnUnit( TPS8( vpd_carrot ), &oTransform );
					break;
				case VEGGIE_MELON:
					pUnit = AUnitManager::GetSingleton()->SpawnUnit( TPS8( vpd_melon ), &oTransform );
					break;
				case VEGGIE_CABBAGE:
					pUnit = AUnitManager::GetSingleton()->SpawnUnit( TPS8( vpd_cabbage ), &oTransform );
					break;
				case VEGGIE_PUMPKIN:
					pUnit = AUnitManager::GetSingleton()->SpawnUnit( TPS8( vpd_pumpkin ), &oTransform );
					break;
			}

			if ( i != VEGGIE_SPROUT )
			{
				m_oPatch.iNumInPatch++;

				if ( pUnit )
				{
					AVeggiePatchVeggie* pVeggie = TSTATICCAST( AVeggiePatchVeggie, pUnit );
					m_oPatch.oVeggies.PushBack( pVeggie );

					pSlot->pModelInstance = AModelRepos::GetSingleton()->InstantiateModel( pVeggie->GetModelInstance()->GetModel() );
					pSlot->pModelInstance->SetSkeletonUpdating( TFALSE );
					pVeggie->Plant( pSlot, &m_oPatch );
				}
			}

			m_oPatch.aSlotLists[ i ].PushBack( pSlot );
		}
	}

	// The game is lost when half of the veggies are gone
	m_oPatch.iMaxLost /= 2;
}

// $Barnyard: FUNCTION 004c7810
void AVeggiePatchDefenderGame::DestroyVeggies()
{
	while ( !m_oPatch.oFallingVeggies.IsEmpty() )
	{
		AVeggiePatchVeggie* pVeggie = m_oPatch.oFallingVeggies.Head();
		pVeggie->T2DList<AVeggiePatchVeggie>::Node::Remove();
		AUnitManager::GetSingleton()->DestroyUnit( pVeggie );
	}

	while ( !m_oPatch.oCarriedVeggies.IsEmpty() )
	{
		AVeggiePatchVeggie* pVeggie = m_oPatch.oCarriedVeggies.Head();
		pVeggie->T2DList<AVeggiePatchVeggie>::Node::Remove();
		AUnitManager::GetSingleton()->DestroyUnit( pVeggie );
	}

	while ( !m_oPatch.oVeggies.IsEmpty() )
	{
		AVeggiePatchVeggie* pVeggie = m_oPatch.oVeggies.Head();
		pVeggie->T2DList<AVeggiePatchVeggie>::Node::Remove();
		AUnitManager::GetSingleton()->DestroyUnit( pVeggie );
	}

	for ( TINT i = 0; i < NUM_VEGGIE_KINDS; i++ )
	{
		T2SList<VeggieSlot>& rSlots = m_oPatch.aSlotLists[ i ];

		for ( auto it = rSlots.Begin(); it != rSlots.End(); it++ )
		{
			AModelRepos::GetSingleton()->DestroyModelInstance( it->pModelInstance );
			it->pModelInstance = TNULL;
		}

		rSlots.Clear();

		if ( m_oPatch.apSlots[ i ] )
		{
			delete[] m_oPatch.apSlots[ i ];
			m_oPatch.apSlots[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 004c89f0
void AVeggiePatchDefenderGame::CreateThieves()
{
	for ( TINT i = NUM_THIEVES; i != 0; i-- )
	{
		AVeggiePatchThief* pThief = TSTATICCAST( AVeggiePatchThief, AUnitManager::GetSingleton()->SpawnUnit( TPS8( veggie_raccoon ), TVector4::VEC_ZERO, TVector4::VEC_POSX ) );
		pThief->SetNoQuery( TTRUE );
		pThief->SetHashed( TFALSE );
		pThief->SetVisible( TFALSE );
		pThief->m_pPatch = &m_oPatch;

		// Visible from further away than the other units
		TSceneObject* pSceneObject         = pThief->GetAnimatableObject()->GetModelInstance()->GetSceneObject();
		pSceneObject->GetFadeDistances().x = 400.0f;
		pSceneObject->GetFadeDistances().y = 800.0f;
		pSceneObject->GetFadeParams().x    = 0.0025f;

		pThief->m_pGame = this;

		if ( !pThief->GetSteer()->GetClass()->IsA( &TGetClass( AStationarySteer ) ) )
			pThief->ChangeSteer( &TGetClass( AStationarySteer ) );

		m_oPatch.oFreeThieves.PushBack( pThief );
	}
}

// $Barnyard: FUNCTION 004c7a00
void AVeggiePatchDefenderGame::DestroyThieves()
{
	while ( !m_oPatch.oFreeThieves.IsEmpty() )
	{
		AVeggiePatchThief* pThief = m_oPatch.oFreeThieves.Head();
		pThief->T2DList<AVeggiePatchThief>::Node::Remove();
		AUnitManager::GetSingleton()->DestroyUnit( pThief );
	}

	while ( !m_oPatch.oThieves.IsEmpty() )
	{
		AVeggiePatchThief* pThief = m_oPatch.oThieves.Head();
		pThief->T2DList<AVeggiePatchThief>::Node::Remove();
		AUnitManager::GetSingleton()->DestroyUnit( pThief );
	}
}

// $Barnyard: FUNCTION 004c8680
void AVeggiePatchDefenderGame::CreateModels()
{
	TMatrix44 oTransform;

	if ( !m_pArrowModel )
		m_pArrowModel = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data\\models\\veggie_arrow" ), TNULL );

	if ( !m_pArrowModel2 )
		m_pArrowModel2 = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data\\models\\veggie_arrow" ), TNULL );

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	if ( !m_pGateModel )
	{
		m_pGateModel = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data\\models\\fncveggate" ), TNULL );
		pTerrain->GetLocatorTransform( "fncveggate", oTransform );
		m_pGateModel->SetTransform( oTransform );
		m_pGateModel->SetSkeletonUpdating( TTRUE );
		m_pGateModel->SetUnlit();
		m_oGateCollision.Create( m_pGateModel->GetSceneObject() );
	}

	if ( !m_pScarecrowModel )
	{
		m_pScarecrowModel = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data\\models\\scarecrow_day" ), TNULL );
		pTerrain->GetLocatorTransform( "scarecrow", oTransform );
		m_pScarecrowModel->SetTransform( oTransform );
		m_pScarecrowModel->SetSkeletonUpdating( TTRUE );
		m_oScarecrowCollision.Create( m_pScarecrowModel->GetSceneObject() );
	}

	// The outer fence
	TPString8            strFence( "data\\models\\fncb02" );
	ATerrainLocatorList* pLocators = pTerrain->GetLocatorList();
	TINT                 iCount    = pLocators->GetNumLocators();

	for ( TINT i = 0; i < iCount; i++ )
	{
		const TCHAR* szName = pLocators->GetLocatorName( i );

		if ( TStringManager::String8FindString( szName, "fncb02" ) == szName && TStringManager::String8Compare( szName, "fncb02_01" ) != 0 )
		{
			AModelInstance* pModelInstance = AModelRepos::GetSingleton()->InstantiateNewModel( strFence, TNULL );

			if ( pModelInstance )
			{
				pTerrain->GetLocatorTransform( szName, oTransform );
				pModelInstance->SetSkeletonUpdating( TTRUE );
				pModelInstance->SetTransform( oTransform );
				pModelInstance->SetUnlit();

				FenceModel* pNode = new FenceModel;
				pNode->pInstance  = pModelInstance;
				m_oFenceModels.PushFront( pNode );
			}
		}
	}
}

// $Barnyard: FUNCTION 004c6db0
void AVeggiePatchDefenderGame::DestroyModels()
{
	if ( m_pArrowModel )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pArrowModel );
		m_pArrowModel = TNULL;
	}

	if ( m_pArrowModel2 )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pArrowModel2 );
		m_pArrowModel2 = TNULL;
	}

	if ( m_pGateModel )
	{
		m_oGateCollision.UnlinkCollisionObject();
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pGateModel );
		m_pGateModel = TNULL;
	}

	if ( m_pScarecrowModel )
	{
		m_oScarecrowCollision.UnlinkCollisionObject();
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pScarecrowModel );
		m_pScarecrowModel = TNULL;
	}

	while ( m_oFenceModels.IsLinked() )
	{
		FenceModel* pNode = m_oFenceModels.PopFront();
		AModelRepos::GetSingleton()->DestroyModelInstance( pNode->pInstance );
		delete pNode;
	}
}

// $Barnyard: FUNCTION 004c72a0
void AVeggiePatchDefenderGame::StartWave( const AVeggiePatchWaveData* a_pWave )
{
	if ( a_pWave->eType == WAVE_SPAWN )
	{
		m_fSpawnTimer = 0.0f;
		m_oWarning.SetShown( TFALSE );
	}
	else if ( a_pWave->eType == WAVE_FLEE )
	{
		m_oWarning.SetShown( TFALSE );
	}
	else if ( a_pWave->eType == WAVE_SNIPERS )
	{
		m_oWarning.SetShown( TTRUE );

		// The snipers take their places
		for ( TINT i = 0; i < NUM_SNIPERS && !m_oPatch.oFreeThieves.IsEmpty(); i++ )
		{
			if ( ( 1 << i ) & a_pWave->iCount )
			{
				AVeggiePatchThief* pThief = m_oPatch.oFreeThieves.Head();
				pThief->GetSteer()->SetPosition( s_aSniperPositions[ i ] );
				pThief->GetSteer()->SetDirection( *TREINTERPRETCAST( TVector4*, &s_aSniperRotations[ i ] ) );
				// NOTE: retail passes the rotation of the locator as the direction
				pThief->SetState( AVeggiePatchThief::STATE_SLINGSHOT_GETON );
			}
		}
	}
	else
	{
		return;
	}

	m_fWaveTimer = a_pWave->fDuration;
}

// $Barnyard: FUNCTION 004c6f70
void AVeggiePatchDefenderGame::UpdatePlayer()
{
	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );

	switch ( m_oPatch.ePlayerState )
	{
		case PLAYERSTATE_FREE:
			if ( m_oPatch.bPlayerHit && pSteer->GetState() == APlayerSteer::STATE_WALKING )
			{
				// Hit by a tomato while busy
				m_pPlayer->GetAnimatableObject()->KillAllAnimations();

				if ( m_oPatch.iLives > 0 )
				{
					m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
				}
				else
				{
					m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( defeat ) );
					m_oPatch.bDefeated = TTRUE;
				}

				m_pControl->SetMovementEnabled( TFALSE );
				m_pControl->SetLauncherInput( TFALSE );
				m_pControl->SetSecondaryLauncherInput( TFALSE );

				m_oPatch.ePlayerState = PLAYERSTATE_HIT;
				m_oPatch.bPlayerHit   = TFALSE;
			}

			break;
		case PLAYERSTATE_PICKUP_THIEF:
		{
			// Turn to the raccoon and pick it up
			const TVector4* pPlayerPos = pSteer->GetPosition();
			const TVector4* pThiefPos  = m_oPatch.pCarriedThief->GetSteer()->GetPosition();

			TVector4 vToThief( pThiefPos->x - pPlayerPos->x, pThiefPos->y - pPlayerPos->y, pThiefPos->z - pPlayerPos->z, pThiefPos->w );
			NormaliseXZ( vToThief );
			pSteer->StopAndFace( vToThief );

			if ( pSteer->IsFacingDesired( 0.99f ) && m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( pickup_raccoon ) ) )
			{
				m_oPatch.pCarriedThief->GetAnimatableObject()->KillAllAnimations();
				m_oPatch.pCarriedThief->GetAnimatableObject()->PlayAnimation( TPS8( picked_up ) );
				m_oPatch.ePlayerState = PLAYERSTATE_CARRY_THIEF;
			}

			break;
		}
		case PLAYERSTATE_CARRY_THIEF:
			if ( !m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( pickup_raccoon ) ) ||
			     m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) == m_oPatch.pCarriedThief )
			{
				// Turn to the throw target and throw the raccoon
				if ( TVector4::DotProduct3( *pSteer->GetDirection(), m_oPatch.vThrowDirection ) <= 0.95f )
				{
					pSteer->StopAndFace( m_oPatch.vThrowDirection );
				}
				else if ( m_oPatch.pCarriedThief->GetState() == AVeggiePatchThief::STATE_PICKED_UP && !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( throw_raccoon ), 0.0f ) )
				{
					m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( throw_raccoon ) );
					m_oPatch.pCarriedThief->GetAnimatableObject()->PlayAnimation( TPS8( thrown ) );
				}
			}
			else
			{
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( pickup_raccoon ) );
			}

			break;
		case PLAYERSTATE_THROW_THIEF:
			if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( throw_raccoon ) ) )
			{
				m_pControl->SetMovementEnabled( TTRUE );
				m_pControl->SetLauncherInput( TTRUE );
				m_pControl->SetSecondaryLauncherInput( TTRUE );
				m_oPatch.ePlayerState = PLAYERSTATE_FREE;
			}

			break;
		case PLAYERSTATE_HIT:
			if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( yuk ) ) )
			{
				m_pControl->SetMovementEnabled( TTRUE );
				m_pControl->SetLauncherInput( TTRUE );
				m_pControl->SetSecondaryLauncherInput( TTRUE );
				m_oPatch.ePlayerState = PLAYERSTATE_FREE;
			}

			break;
	}
}

// $Barnyard: FUNCTION 004c8fe0
void AVeggiePatchDefenderGame::UpdateInteraction()
{
	HideInteraction();

	if ( m_pPlayer->GetActiveLauncher()->GetClass()->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
		return;

	TVector4 vPosition = *m_pPlayer->GetSteer()->GetPosition();

	if ( m_oPatch.ePlayerState != PLAYERSTATE_FREE )
	{
		HideInteraction();
		return;
	}

	TVector4 vTarget;
	TFLOAT   fDistance = 2.0f;

	m_oPatch.pNearestFence = FindNearestOpenGate( vPosition, vTarget, fDistance );
	m_oPatch.pCarriedThief = FindNearestMilkedThief( vPosition, vTarget, fDistance );

	if ( m_oPatch.pCarriedThief )
	{
		// "Pick up <raccoon>"
		m_oPatch.pNearestFence = TNULL;
		ALocale::FormatTags( m_wszInteraction, TARRAYSIZE( m_wszInteraction ), LOCALE_PICKUP_RACCOON, PPropertyValue( const_cast<TWCHAR*>( m_oPatch.pCarriedThief->GetScreenName() ), TFALSE ) );

		TVector4 vArrow = *m_oPatch.pCarriedThief->GetSteer()->GetPosition();
		vArrow.y -= 2.0f;
		ShowInteraction( vArrow, m_wszInteraction );
	}
	else if ( m_oPatch.pNearestFence )
	{
		// "Close the gate"
		TVector4 vArrow = m_oPatch.pNearestFence->vInside;
		vArrow.y -= 3.0f;
		ShowInteraction( vArrow, ALocaleManager::Interface()->GetString( LOCALE_CLOSE_GATE ) );
	}
}

// $Barnyard: FUNCTION 004c6910
void AVeggiePatchDefenderGame::HideInteraction()
{
	m_pArrowModel->SetSkeletonUpdating( TFALSE );
	m_pArrowModel2->SetSkeletonUpdating( TFALSE );

	m_oInteractionHud.SetText( TNULL, 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, -1, -1.0f );
	m_oInteractionHud.SetText( TNULL, 2, TINT( m_pInputMap ), AInputCommand_Interact, 1, -1, -1.0f );
	m_oInteractionHud.SetForceHide( TTRUE );
	m_iInteractionLine = 0;
}

// $Barnyard: FUNCTION 004c6e80
void AVeggiePatchDefenderGame::ShowInteraction( const TVector4& a_rcPosition, const TWCHAR* a_wszText )
{
	// A spinning arrow bobs over the thing the player can use
	TFLOAT fZ   = a_rcPosition.z;
	TFLOAT fBob = TMath::Sin( m_fElapsedTime * TMath::TWO_PI );

	TMatrix44 oTransform = TMatrix44::IDENTITY;
	oTransform.m_f41     = a_rcPosition.x;
	oTransform.m_f43     = fZ;
	oTransform.m_f42     = a_rcPosition.y - fBob * 0.25f;
	oTransform.RotateY( m_fElapsedTime * TMath::PI );
	oTransform.RotateX( TMath::HALF_PI );

	m_pArrowModel2->SetSkeletonUpdating( TTRUE );
	m_pArrowModel2->SetTransform( oTransform );
	m_pArrowModel2->SetOpacity( 1.0f );

	m_oInteractionHud.SetForceHide( TFALSE );
	m_oInteractionHud.SetText( a_wszText, 2, TINT( m_pInputMap ), AInputCommand_Interact, m_iInteractionLine++, -1, AGUI2::GetRootElement()->GetWidth() * 0.45f );
}

// $Barnyard: FUNCTION 004c6980
AVeggiePatchDefenderGame::Fence* AVeggiePatchDefenderGame::FindNearestOpenGate( const TVector4& a_rcPosition, TVector4& a_rOutPosition, TFLOAT& a_rDistance )
{
	if ( m_oPatch.iNumGates == 0 )
		return TNULL;

	Fence* pNearest = TNULL;
	a_rDistance     = a_rDistance * a_rDistance;

	for ( TINT i = 0; i < m_oPatch.iNumGates; i++ )
	{
		Fence* pFence = &m_oPatch.aFences[ m_oPatch.pGates[ i ] ];

		if ( pFence->eState == Fence::STATE_OPEN )
		{
			TFLOAT fDistance = TVector4::DistanceSq( a_rcPosition, pFence->vInside );

			if ( fDistance < a_rDistance )
			{
				a_rDistance = fDistance;
				pNearest    = pFence;
			}
		}
	}

	if ( pNearest )
		a_rOutPosition = pNearest->vInside;

	a_rDistance = TMath::Sqrt( a_rDistance );
	return pNearest;
}

// $Barnyard: FUNCTION 004c7df0
AVeggiePatchThief* AVeggiePatchDefenderGame::FindNearestMilkedThief( const TVector4& a_rcPosition, TVector4& a_rOutPosition, TFLOAT& a_rDistance )
{
	AVeggiePatchThief* pNearest = TNULL;
	a_rDistance                 = a_rDistance * a_rDistance;

	T2_FOREACH( m_oPatch.oThieves, it )
	{
		if ( it->GetState() == AVeggiePatchThief::STATE_MILKED )
		{
			TFLOAT fDistance = TVector4::DistanceSq( a_rcPosition, *it->GetSteer()->GetPosition() );

			if ( fDistance < a_rDistance )
			{
				a_rDistance = fDistance;
				pNearest    = it;
			}
		}
	}

	if ( pNearest )
		pNearest->GetSteer()->CopyPosition( a_rOutPosition );

	a_rDistance = TMath::Sqrt( a_rDistance );
	return pNearest;
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" runs. The player closes the open gates, picks up the
// milked raccoons and throws them out of the patch, otherwise he squirts milk at the nearest
// raccoon in the patch. He is moved next to his target instead of walking there.
void AVeggiePatchDefenderGame::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	if ( !T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
		return;

	static TFLOAT s_fTimer = 0.0f;
	s_fTimer += a_fDeltaTime;

	if ( s_fTimer < 0.75f || m_oPatch.ePlayerState != PLAYERSTATE_FREE )
		return;

	s_fTimer = 0.0f;

	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );

	// Something to use next to the player (found by UpdateInteraction)
	if ( m_oPatch.pCarriedThief || m_oPatch.pNearestFence )
	{
		TINFO( "[vegauto] interact (%s)\n", m_oPatch.pCarriedThief ? "raccoon" : "gate" );

		TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
		TBOOL                       bStopEvents = TFALSE;
		ProcessCommand( AInputCommand_Interact, &oEvent, bStopEvents );
		return;
	}

	// A milked raccoon to pick up
	T2_FOREACH( m_oPatch.oThieves, it )
	{
		if ( it->GetState() == AVeggiePatchThief::STATE_MILKED )
		{
			TVector4 vPosition = *it->GetSteer()->GetPosition();
			vPosition.x += 1.0f;
			pSteer->SetPosition( vPosition );
			TINFO( "[vegauto] go to the milked raccoon\n" );
			return;
		}
	}

	// An open gate to close
	for ( TINT i = 0; i < m_oPatch.iNumGates; i++ )
	{
		Fence& rFence = m_oPatch.aFences[ m_oPatch.pGates[ i ] ];

		if ( rFence.eState == Fence::STATE_OPEN && !rFence.pUnit )
		{
			pSteer->SetPosition( rFence.vInside );
			TINFO( "[vegauto] go to the open gate %d\n", m_oPatch.pGates[ i ] );
			return;
		}
	}

	// Squirt the nearest raccoon in the patch
	AVeggiePatchThief* pTarget   = TNULL;
	TFLOAT             fDistance = 0.0f;

	T2_FOREACH( m_oPatch.oThieves, it )
	{
		AVeggiePatchThief::STATE_ eState = it->GetState();

		if ( eState != AVeggiePatchThief::STATE_SEEK_VEGGIE && eState != AVeggiePatchThief::STATE_CARRY_VEGGIE && eState != AVeggiePatchThief::STATE_WAIT && eState != AVeggiePatchThief::STATE_PICKUP )
			continue;

		TFLOAT fThiefDistance = TVector4::DistanceSq( *it->GetSteer()->GetPosition(), *pSteer->GetPosition() );

		if ( !pTarget || fThiefDistance < fDistance )
		{
			pTarget   = it;
			fDistance = fThiefDistance;
		}
	}

	if ( !pTarget )
		return;

	const TVector4& rcThiefPos = *pTarget->GetSteer()->GetPosition();
	TVector4        vToThief( rcThiefPos.x - pSteer->GetPosition()->x, 0.0f, rcThiefPos.z - pSteer->GetPosition()->z, 1.0f );
	NormaliseXZ( vToThief );

	if ( fDistance > 16.0f )
	{
		TVector4 vPosition( rcThiefPos.x - vToThief.x * 3.0f, rcThiefPos.y, rcThiefPos.z - vToThief.z * 3.0f, 1.0f );
		pSteer->SetPosition( vPosition );
	}

	pSteer->SetDirection( vToThief );
	m_pPlayer->GetActiveLauncher()->GetAmmo()->SetAmmo( m_pPlayer->GetActiveLauncher()->GetAmmo()->GetMaxAmmo() );
	m_pPlayer->GetActiveLauncher()->Fire();
	TINFO( "[vegauto] squirt a raccoon (state %d)\n", pTarget->GetState() );
}

#endif
