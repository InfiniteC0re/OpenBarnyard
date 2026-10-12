#include "pch.h"
#include <Render/TCameraObject.h>
#include "ABeeSwarm.h"
#include "ARandom.h"
#include "Animation/AModelRepos.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/ASpline.h"
#include "MiniGame/AHoneyCollectingGame.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "Physics/ACollisionObjectMesh.h"

#include <Math/TMathInline.h>
#include <Render/TRenderInterface.h>
#include <Render/TRenderContext.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0048a2c0, CreateObject 0048bc10, CreateObjectInPlace 0048bc70 (the constructor is inlined in both)
TDEFINE_CLASS( ABeeSwarm );

TPSTRING8_DECLARE( swarm_lp );
TPSTRING8_DECLARE( pollen_lp );
TPSTRING8_DECLARE( attack_lp );
TPSTRING8_DECLARE( sting_lp );
TPSTRING8_DECLARE( sting1 );
TPSTRING8_DECLARE( sting2 );
TPSTRING8_DECLARE( sting3 );
TPSTRING8_DECLARE( sting4 );
TPSTRING8_DECLARE( disperse );
TPSTRING8_DECLARE( disperse_lp );
TPSTRING8_DECLARE( wingsBuzz );

// Sounds of the swarm
static constexpr ASoundId SOUND_BUZZ       = 280;
static constexpr ASoundId SOUND_ALERT      = 281;
static constexpr ASoundId SOUND_STING      = 283;
static constexpr ASoundId SOUND_BUZZ_ANGRY = 291;

// Fades of the buzz (seconds)
static constexpr TFLOAT BUZZ_FADE_TIME = 0.5f;

// $Barnyard: FUNCTION 0048a290
// Sets the speed modifier of the steer, unless a_fModifier is negative
static TBOOL SetSteerSpeedModifier( ASteer* a_pSteer, TFLOAT a_fModifier )
{
	if ( a_fModifier < 0.0f )
		return TFALSE;

	a_pSteer->SetSpeedModifier( a_fModifier );
	return TTRUE;
}

// $Barnyard: FUNCTION 0048b090
ABeeSwarm::ABeeSwarm()
    : m_iUnk98( 0 )
    , m_eState( STATE_IDLE )
    , m_iUnk140( 0 )
    , m_iUnk144( 0 )
    , m_vUnk148( TVector4::VEC_ZERO )
    , m_vUnk158( TVector4::VEC_ZERO )
    , m_iUnk168( 0 )
    , m_iSound( -1 )
    , m_iCue( -1 )
    , m_bUnk178( TFALSE )
    , m_bUnk179( TFALSE )
    , m_fPollenTime( 0.0f )
    , m_fAttackDelay( 0.0f )
    , m_vTarget( TVector4::VEC_ZERO )
    , m_iUnk194( 0 )
    , m_iHoney( 0 )
    , m_iUnk1a4( 0 )
    , m_iUnk1a8( 0 )
    , m_fStateTime( 0.0f )
    , m_bIgnorePlayer( TFALSE )
    , m_fAttackCooldown( 0.0f )
    , m_vFlowerBed( TVector4::VEC_ZERO )
    , m_pPatrol( TNULL )
    , m_fPatrolProgress( 0.0f )
    , m_pNetwork( TNULL )
    , m_fStingTime( 0.0f )
    , m_fFlowerBedProgress( -1.0f )
    , m_fPollenCooldown( 0.0f )
    , m_iMapIcon( -1 )
    , m_iFOVMapIcon( -1 )
    , m_bAngry( TFALSE )
    , m_fnRenderCallback( TNULL )
    , m_pRenderCallbackData( TNULL )
    , m_fDisperseTime( 0.0f )
    , m_fHideTime( 0.0f )
    , m_pFOVModel( TNULL )
    , m_pFOVCollision( TNULL )
    , m_fFadeInTime( 0.0f )
    , m_fFadeOutTime( 0.0f )
    , m_fFrequency( 1.0f )
    , m_fCurrentFrequency( 1.0f )
    , m_fWobbleTime( 0.0f )
{
}

// $Barnyard: FUNCTION 0048a5f0
// $Barnyard: FUNCTION 0048b2b0
// $Barnyard: FUNCTION 0048a6c0
ABeeSwarm::~ABeeSwarm()
{
	AModelRepos::GetSingleton()->DestroyModelInstance( m_pFOVModel );

	if ( m_pFOVCollision )
		delete m_pFOVCollision;
}

// $Barnyard: FUNCTION 0048a4d0
TBOOL ABeeSwarm::Create( AUnitTransform* a_pTransform )
{
	SetHashed( TTRUE );
	return AAnimatedUnit::Create( a_pTransform );
}

// $Barnyard: FUNCTION 0048bcb0
TBOOL ABeeSwarm::Think( TFLOAT a_fDeltaTime )
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( m_iCue != -1 )
		pSoundManager->SetCuePosition( m_iCue, *GetSteer()->GetPosition() );

	if ( m_fFrequency != m_fCurrentFrequency && pSoundManager->IsStreamCueActive( m_iCue ) == TTRUE )
	{
		pSoundManager->SetCueFrequency( m_iCue, m_fFrequency );
		m_fCurrentFrequency = m_fFrequency;
	}

	if ( m_eState == STATE_DISPERSE || m_eState == STATE_DISPERSE_MILKED )
		RemoveFlags( FLAGS_MILK_TARGET );
	else
		AddFlags( FLAGS_MILK_TARGET );

	switch ( m_eState )
	{
		case STATE_IDLE:
			GetSteer()->Command_CancelCurrentCommand();
			break;
		case STATE_PATROL:
			UpdatePatrol( a_fDeltaTime );
			break;
		case STATE_RETURN:
			UpdateReturn( a_fDeltaTime );
			break;
		case STATE_SEEK_POLLEN:
			UpdateSeekPollen( a_fDeltaTime );
			break;
		case STATE_SEEK_HIVE:
			UpdateSeekHive( a_fDeltaTime );
			break;
		case STATE_POLLEN:
			UpdatePollen( a_fDeltaTime );
			break;
		case STATE_ATTACK:
			UpdateAttack( a_fDeltaTime );
			break;
		case STATE_ATTACK_DELAYED:
			UpdateAttackDelayed( a_fDeltaTime );
			break;
		case STATE_STING:
			UpdateSting( a_fDeltaTime );
			break;
		case STATE_DISPERSE:
		case STATE_DISPERSE_MILKED:
			UpdateDisperse( a_fDeltaTime );
			break;
		case STATE_ALERT:
			UpdateAlert( a_fDeltaTime );
			break;
	}

	// The field of view cone follows the swarm
	TMatrix44 matTransform;
	GetAnimatableObject()->GetModelInstance()->GetLocalTransform( matTransform );
	matTransform.RotateZ( TMath::PI );
	m_pFOVModel->GetTransform().SetMatrix( matTransform );
	m_pFOVCollision->GetModelInstance()->SetTransformObject( &m_pFOVModel->GetTransform() );

	return AVisibleUnit::Think( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0048b2d0
TBOOL ABeeSwarm::Setup( AInteriorNetwork* a_pNetwork, TBOOL a_bUnk, ASpline* a_pPatrol, TINT a_iMapIcon, TINT a_iFOVMapIcon )
{
	m_pPatrol     = a_pPatrol;
	m_bUnk178     = a_bUnk;
	m_iFOVMapIcon = a_iFOVMapIcon;
	m_pNetwork    = a_pNetwork;
	m_iMapIcon    = a_iMapIcon;

	Reset();

	// Remember the render method of the model, SetAngry replaces it
	TModelInstance* pInstance = GetAnimatableObject()->GetModelInstance()->GetSceneObject()->GetInstance();
	m_fnRenderCallback        = pInstance->GetCustomRenderMethod();
	m_pRenderCallbackData     = pInstance->GetCustomRenderUserData();

	// The field of view cone
	m_pFOVModel = AModelRepos::GetSingleton()->InstantiateNewModel( TPS8D( "data\\models\\pie_fov" ), TNULL );

	TMatrix44 matTransform = TMatrix44::IDENTITY;
	matTransform.RotateX( TMath::HALF_PI );
	m_pFOVModel->GetTransform().SetMatrix( matTransform );
	m_pFOVModel->SetSkeletonUpdating( TTRUE );

	m_pFOVCollision = new ACollisionObjectMesh();
	m_pFOVCollision->Create( m_pFOVModel->GetSceneObject() );

	GetAnimatableObject()->PlayAnimation( TPS8( wingsBuzz ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 0048af20
void ABeeSwarm::Reset()
{
	m_eState = STATE_IDLE;
	ChangeSteer( &TGetClass( AStationarySteer ) );

	m_fPatrolProgress = 0.0f;

	TVector4 vPosition;
	TVector4 vDirection;
	m_pPatrol->GetPosition( 0.0f, vPosition );
	m_pPatrol->GetDirection( m_fPatrolProgress, vDirection );
	vDirection.y = 0.0f;
	vDirection.Normalise();

	vPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vPosition, TCollisionCommon::TOSHICGROUPFLAG_NONE );

	GetSteer()->SetPosition( vPosition );
	GetSteer()->SetDirection( vDirection );
	SetMovement( TTRUE );

	m_strAnimation = TPS8( swarm_lp );
	GetAnimatableObject()->PlayAnimation( m_strAnimation );

	m_bAngry    = TFALSE;
	m_fHideTime = 0.0f;
}

// $Barnyard: FUNCTION 0048a8b0
void ABeeSwarm::SetState( STATE a_eState )
{
	ASoundId iSound = SOUND_BUZZ;
	m_fFrequency    = 1.0f;

	if ( a_eState != m_eState )
		GetSteer()->Command_CancelCurrentCommand();

	m_eState = a_eState;

	TPString8 strAnimation;

	switch ( a_eState )
	{
		case STATE_IDLE:
			m_fFadeOutTime = BUZZ_FADE_TIME;
			m_fFadeInTime  = BUZZ_FADE_TIME;
			iSound         = SOUND_BUZZ;
			GetSteer()->Command_CancelCurrentCommand();
			strAnimation = TPS8( swarm_lp );
			break;
		case STATE_PATROL:
		{
			m_fFadeOutTime = BUZZ_FADE_TIME;
			m_fFadeInTime  = BUZZ_FADE_TIME;

			if ( m_bAngry == TTRUE )
			{
				m_fFrequency = 1.25f;
				iSound       = SOUND_BUZZ_ANGRY;
			}
			else
			{
				iSound = SOUND_BUZZ;
			}

			ChangeSteer( &TGetClass( AStationarySteer ) );
			strAnimation = TPS8( swarm_lp );

			TVector4 vPosition;
			TVector4 vDirection;
			m_pPatrol->GetPosition( m_fPatrolProgress, vPosition );
			m_pPatrol->GetDirection( m_fPatrolProgress, vDirection );
			vDirection.y = 0.0f;
			vDirection.Normalise();

			GetSteer()->SetPosition( vPosition );
			GetSteer()->SetDirection( vDirection );
			m_fWobbleTime = 0.0f;
			break;
		}
		case STATE_RETURN:
		{
			m_fFadeOutTime = BUZZ_FADE_TIME;
			m_fFadeInTime  = BUZZ_FADE_TIME;

			if ( m_bAngry == TTRUE )
			{
				m_fFrequency = 1.25f;
				iSound       = SOUND_BUZZ_ANGRY;
			}
			else
			{
				iSound = SOUND_BUZZ;
			}

			// Fly back to the closest point of the patrol
			TVector4 vPosition;
			GetSteer()->CopyPosition( vPosition );
			m_fPatrolProgress = m_pPatrol->FindClosestProgress( vPosition, 100.0f );
			m_pPatrol->GetPosition( m_fPatrolProgress, m_vFlowerBed );
			m_vFlowerBed.y = ATerrainInterface::GetSingleton()->GetGroundHeight( m_vFlowerBed, TCollisionCommon::TOSHICGROUPFLAG_NONE );

			if ( GetSteer()->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) == TTRUE )
			{
				AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, GetSteer() );
				pSteer->Command_CancelCurrentCommand();
				pSteer->SeekPosition( m_vFlowerBed );

				SetSteerSpeedModifier( pSteer, ( m_bAngry == TTRUE ) ? 3.0f : 1.0f );
				strAnimation = TPS8( swarm_lp );
			}
			else
			{
				SetState( STATE_PATROL );
				strAnimation = TPS8( swarm_lp );
			}

			break;
		}
		case STATE_POLLEN:
			m_fFadeOutTime = BUZZ_FADE_TIME;
			m_fFadeInTime  = BUZZ_FADE_TIME;
			iSound         = SOUND_BUZZ;
			strAnimation   = TPS8( pollen_lp );
			break;
		case STATE_ATTACK:
		{
			iSound = SOUND_BUZZ_ANGRY;

			if ( m_bAngry == TTRUE )
				m_fFrequency = 1.25f;

			m_fFadeOutTime = 0.0f;
			m_fFadeInTime  = 0.0f;

			// Chase the player through the network
			ChangeSteer( &TGetClass( AInteriorSteer ) );
			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, GetSteer() );
			pSteer->ConnectToNetwork( m_pNetwork );
			pSteer->SetFlags( pSteer->GetFlags() | AInteriorSteer::FLAGS_KINEMATICS_NO_PUSH );
			pSteer->SetFlags( pSteer->GetFlags() | AInteriorSteer::FLAGS_KINEMATICS_NO_AVOID );
			pSteer->EnableKinematicsObstructionStop( TFALSE );
			SetSteerSpeedModifier( pSteer, 3.0f );

			m_vTarget  = *APlayerManager::GetSingleton()->GetPlayer( 0 )->GetSteer()->GetPosition();
			TINT iArea = TSTATICCAST( AInteriorNetwork, TSTATICCAST( ACommonSteer, pSteer )->GetCurrentNetwork() )->FindContainingArea( m_vTarget );

			if ( iArea == -1 )
			{
				// The player can't be reached
				strAnimation = TPS8( swarm_lp );
				SetState( STATE_RETURN );
				SetSteerSpeedModifier( pSteer, 1.0f );
				m_fAttackDelay = 0.0f;
			}
			else
			{
				pSteer->Command_CancelCurrentCommand();
				pSteer->SeekPointInArea( m_vTarget, iArea );
				strAnimation = TPS8( attack_lp );
			}

			break;
		}
		case STATE_ATTACK_DELAYED:
			strAnimation   = TPS8( swarm_lp );
			m_fAttackDelay = 1.0f;
			break;
		case STATE_STING:
			iSound         = SOUND_STING;
			m_fFadeOutTime = 0.0f;
			m_fFadeInTime  = 0.0f;
			strAnimation   = TPS8( sting_lp );
			m_fStingTime   = 0.0f;
			break;
		case STATE_DISPERSE:
			strAnimation    = TPS8( disperse );
			m_fDisperseTime = 4.0f;
			m_fFadeOutTime  = BUZZ_FADE_TIME;
			m_fFadeInTime   = BUZZ_FADE_TIME;
			iSound          = SOUND_BUZZ_ANGRY;
			m_fFrequency    = 1.25f;
			m_fStateTime    = 0.0f;
			break;
		case STATE_DISPERSE_MILKED:
			m_fFadeOutTime  = BUZZ_FADE_TIME;
			m_fFadeInTime   = BUZZ_FADE_TIME;
			iSound          = SOUND_BUZZ_ANGRY;
			m_fFrequency    = 1.25f;
			strAnimation    = TPS8( disperse );
			m_fDisperseTime = 2.0f;
			m_fStateTime    = 0.0f;
			break;
		case STATE_ALERT:
			strAnimation = TPS8( disperse );
			iSound       = SOUND_BUZZ_ANGRY;

			if ( m_bAngry == TTRUE )
				m_fFrequency = 1.25f;

			m_fFadeOutTime = 0.0f;
			m_fFadeInTime  = 0.0f;
			ASoundManager::GetSingleton()->PlayCueAtPositionEx( SOUND_ALERT, *GetSteer()->GetPosition(), 0.25f, TFALSE, 0.0f, -1 );
			m_fStateTime = 0.0f;
			break;
		default:
			break;
	}

	// Change the buzz
	if ( iSound != m_iSound )
	{
		ASoundManager* pSoundManager = ASoundManager::GetSingleton();

		if ( m_fFadeOutTime <= 0.0f )
		{
			pSoundManager->StopCue( m_iCue );
		}
		else
		{
			pSoundManager->FadeOutCue( m_iCue, m_fFadeOutTime, TTRUE, 3.0f );
			m_fFadeOutTime = 0.0f;
		}

		if ( m_fFadeInTime <= 0.0f )
		{
			m_iCue = pSoundManager->PlayCueAtPosition( iSound, *GetSteer()->GetPosition() );
		}
		else
		{
			m_iCue = pSoundManager->PlayCueAtPositionEx( iSound, *GetSteer()->GetPosition(), 0.0f, TFALSE, 0.0f, -1 );

			if ( m_iCue != -1 )
				pSoundManager->FadeCue( m_iCue, m_fFadeInTime, 1.0f, 3.0f );

			m_fFadeInTime = 0.0f;
		}

		m_iSound = iSound;
	}

	if ( strAnimation != m_strAnimation )
	{
		GetAnimatableObject()->PlayAnimation( strAnimation );
		m_strAnimation = strAnimation;
	}
}

// $Barnyard: FUNCTION 0048b010
void ABeeSwarm::SetAngry( TBOOL a_bAngry )
{
	TModelInstance* pInstance = GetAnimatableObject()->GetModelInstance()->GetSceneObject()->GetInstance();

	if ( a_bAngry == TTRUE )
	{
		pInstance->SetCustomRenderMethod( RenderCallback, this );
		m_bAngry = TTRUE;
	}
	else
	{
		pInstance->SetCustomRenderMethod( m_fnRenderCallback, m_pRenderCallbackData );
		m_bAngry = a_bAngry;
	}
}

// $Barnyard: FUNCTION 0048a480
void ABeeSwarm::SetFlowerBed( const TVector4& a_rcPosition )
{
	m_vFlowerBed         = a_rcPosition;
	m_fFlowerBedProgress = m_pPatrol->FindClosestProgress( a_rcPosition, 100.0f );
}

// $Barnyard: FUNCTION 0048a5c0
void ABeeSwarm::PauseSound( TBOOL a_bPause )
{
	if ( m_iCue != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_iCue, a_bPause );
}

// $Barnyard: FUNCTION 0048a4f0
TBOOL ABeeSwarm::CanSeePlayer()
{
	// NOTE: the position and the direction of the swarm are read and not used
	TVector4 vDirection;
	TVector4 vPosition;
	GetSteer()->CopyDirection( vDirection );
	GetSteer()->CopyPosition( vPosition );

	AUnit*   pPlayer         = APlayerManager::GetSingleton()->GetPlayer( 0 );
	TVector4 vPlayerPosition = *pPlayer->GetSteer()->GetPosition();
	TVector4 vPlayerDir      = *pPlayer->GetSteer()->GetDirection();

	TFLOAT   fDistance = 0.5f;
	TVector4 vNormal;
	TFLOAT   fStepHeight;

	return TSTATICCAST( AVisibleUnit, pPlayer )->GetCollisionObject()->CollideSweptSphere( m_pFOVCollision, fDistance, vPlayerDir, vPlayerPosition, vNormal, 0.0f, fStepHeight ) == TTRUE;
}

// $Barnyard: FUNCTION 0048b680
void ABeeSwarm::UpdatePatrol( TFLOAT a_fDeltaTime )
{
	m_fPollenCooldown -= a_fDeltaTime;

	if ( m_fPollenCooldown < 0.0f )
		m_fPollenCooldown = 0.0f;

	// Rest above the flower bed when passing by it
	if ( m_fFlowerBedProgress != -1.0f && m_eState == STATE_PATROL && m_fPollenCooldown <= 0.0f && !m_bAngry &&
	     TMath::Abs( m_fPatrolProgress - m_fFlowerBedProgress ) <= 0.01f )
	{
		SetState( STATE_POLLEN );
		m_fPollenTime = 5.0f;
		return;
	}

	// Follow the patrol, the angry swarm is faster
	if ( m_bAngry == TTRUE )
		m_fPatrolProgress += a_fDeltaTime * 0.05f;
	else
		m_fPatrolProgress += a_fDeltaTime * 0.022222223f;

	TFLOAT fProgress  = TFLOAT( fmod( m_fPatrolProgress, 1.0 ) );
	m_fPatrolProgress = fProgress;

	TVector4 vPosition;
	TVector4 vDirection;
	m_pPatrol->GetPosition( fProgress, vPosition );
	m_pPatrol->GetDirection( m_fPatrolProgress, vDirection );
	vDirection.y = 0.0f;
	vDirection.Normalise();

	TFLOAT fGroundHeight = ATerrainInterface::GetSingleton()->GetGroundHeight( vPosition, TCollisionCommon::TOSHICGROUPFLAG_NONE );
	vPosition.y          = TMath::LERP( vPosition.y, fGroundHeight, a_fDeltaTime );
	GetSteer()->SetPosition( vPosition );

	m_fWobbleTime += a_fDeltaTime;

	if ( m_bAngry == TTRUE )
		vDirection.AsVector3().RotateY( TFLOAT( sin( m_fWobbleTime ) ) * TMath::HALF_PI * 0.5f );

	GetSteer()->SetDirection( vDirection );

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( pState->GetClass()->IsA( &TGetClass( AHoneyCollectingGame ) ) == TTRUE && !m_bIgnorePlayer && m_fAttackCooldown <= 0.0f &&
	     !TSTATICCAST( AHoneyCollectingGame, pState )->IsHiding() && CanSeePlayer() == TTRUE &&
	     !TSTATICCAST( AHoneyCollectingGame, pState )->IsDefeated() )
	{
		// The player walked into the field of view
		AUnit*   pPlayer         = APlayerManager::GetSingleton()->GetPlayer( 0 );
		TVector4 vPlayerPosition = *pPlayer->GetSteer()->GetPosition();
		pPlayer->GetSteer()->GetDirection();

		// NOTE: retail subtracts an uninitialised vector from the player's position and then overwrites
		// the result with the swarm's position, so the distance is measured from the origin of the world
		// instead of from the player. The swarm only attacks right away when it is already angry.
		TVector4 vOffset = *GetSteer()->GetPosition();

		if ( vOffset.x * vOffset.x + vOffset.z * vOffset.z <= 25.0f || m_bAngry == TTRUE )
			SetState( STATE_ATTACK );
		else
			SetState( STATE_ALERT );
	}

	m_fAttackCooldown -= a_fDeltaTime;

	if ( m_fAttackCooldown < 0.0f )
		m_fAttackCooldown = 0.0f;
}

// $Barnyard: FUNCTION 0048b9e0
void ABeeSwarm::UpdateReturn( TFLOAT a_fDeltaTime )
{
	if ( GetSteer()->HasArrived() == TTRUE )
		SetState( STATE_PATROL );
}

// $Barnyard: FUNCTION 0048b970
void ABeeSwarm::UpdateSeekPollen( TFLOAT a_fDeltaTime )
{
	if ( GetSteer()->HasArrived() == TTRUE )
	{
		SetState( STATE_POLLEN );
		m_fPollenTime = 5.0f;
	}
}

// $Barnyard: FUNCTION 0048ba00
void ABeeSwarm::UpdateSeekHive( TFLOAT a_fDeltaTime )
{
	if ( GetSteer()->HasArrived() == TTRUE )
	{
		SetState( STATE_AT_HIVE );
		m_iHoney++;
	}
}

// $Barnyard: FUNCTION 0048b9a0
void ABeeSwarm::UpdatePollen( TFLOAT a_fDeltaTime )
{
	m_fPollenTime -= a_fDeltaTime;

	if ( m_fPollenTime <= 0.0f )
	{
		SetState( STATE_PATROL );
		m_fPollenCooldown = 2.0f;
	}
}

// $Barnyard: FUNCTION 0048b480
void ABeeSwarm::UpdateAttack( TFLOAT a_fDeltaTime )
{
	m_fStateTime += a_fDeltaTime;
	ASteer* pSteer = GetSteer();

	if ( m_fStateTime >= 0.2f )
	{
		// Follow the player
		m_vTarget  = *APlayerManager::GetSingleton()->GetPlayer( 0 )->GetSteer()->GetPosition();
		TINT iArea = TSTATICCAST( AInteriorNetwork, TSTATICCAST( ACommonSteer, pSteer )->GetCurrentNetwork() )->FindContainingArea( m_vTarget );

		if ( iArea == -1 )
		{
			SetState( STATE_RETURN );
			pSteer->SetSpeedModifier( ( m_bAngry == TTRUE ) ? 3.0f : 1.0f );
			m_fAttackDelay = 0.0f;
		}
		else
		{
			pSteer->Command_CancelCurrentCommand();
			TSTATICCAST( AInteriorSteer, pSteer )->SeekPointInArea( m_vTarget, iArea );
		}

		m_fStateTime = 0.0f;
	}

	TVector4 vPlayerPosition = *APlayerManager::GetSingleton()->GetPlayer( 0 )->GetSteer()->GetPosition();
	TVector4 vOffset;
	vOffset.Substract( vPlayerPosition, *pSteer->GetPosition() );
	vOffset.w = vPlayerPosition.w;

	// NOTE: the state is assumed to be the honey collecting game
	AHoneyCollectingGame* pGame = TSTATICCAST( AHoneyCollectingGame, AGameStateController::GetSingleton()->GetCurrentState() );

	if ( vOffset.MagnitudeSq() > 4.0f || pGame->IsDefeated() )
	{
		// Too far away, give up
		if ( vOffset.MagnitudeSq() >= 400.0f )
			SetState( STATE_RETURN );
	}
	else
	{
		SetState( STATE_STING );
		pGame->ResetStingTime();
	}
}

// $Barnyard: FUNCTION 0048b650
void ABeeSwarm::UpdateAttackDelayed( TFLOAT a_fDeltaTime )
{
	m_fAttackDelay -= a_fDeltaTime;

	if ( m_fAttackDelay <= 0.0f )
		SetState( STATE_ATTACK );
}

// $Barnyard: FUNCTION 0048a6d0
void ABeeSwarm::UpdateSting( TFLOAT a_fDeltaTime )
{
	ASteer*  pSteer          = GetSteer();
	TVector4 vPlayerPosition = *APlayerManager::GetSingleton()->GetPlayer( 0 )->GetSteer()->GetPosition();
	TVector4 vPosition       = *pSteer->GetPosition();

	// Stick to the player
	TVector4 vDirection;
	vDirection.x = vPlayerPosition.x - vPosition.x;
	vDirection.y = 0.0f;
	vDirection.z = vPlayerPosition.z - vPosition.z;
	vDirection.w = vPlayerPosition.w;
	vDirection.Normalise();

	vPosition.Lerp4( vPosition, vPlayerPosition, a_fDeltaTime + a_fDeltaTime );
	pSteer->SetPosition( vPosition );
	pSteer->SetDirection( vDirection );

	m_fStingTime -= a_fDeltaTime;

	if ( m_fStingTime <= 0.0f )
	{
		TPString8 strAnimation;

		switch ( ARandom::GetSingleton()->m_oRandom.GetInt( 4 ) )
		{
			case 0:
				strAnimation = TPS8( sting1 );
				break;
			case 1:
				strAnimation = TPS8( sting2 );
				break;
			case 2:
				strAnimation = TPS8( sting3 );
				break;
			case 3:
				strAnimation = TPS8( sting4 );
				break;
		}

		GetAnimatableObject()->PlayAnimation( strAnimation );
		m_fStingTime = 2.0f;
	}
}

// $Barnyard: FUNCTION 0048ba30
void ABeeSwarm::UpdateDisperse( TFLOAT a_fDeltaTime )
{
	m_fStateTime += a_fDeltaTime;

	if ( GetAnimatableObject()->IsAnimPlaying( TPS8( disperse_lp ), 0.0f ) == TTRUE && m_fStateTime > m_fDisperseTime )
	{
		GetAnimatableObject()->FinishAnimation( TPS8( disperse_lp ) );
		return;
	}

	if ( GetAnimatableObject()->IsAnimPlaying( TPS8( swarm_lp ), 0.0f ) != TTRUE )
		return;

	// The swarm is back together
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( pState->GetClass()->IsA( &TGetClass( AHoneyCollectingGame ) ) == TTRUE && !m_bIgnorePlayer && m_fAttackCooldown <= 0.0f &&
	     !TSTATICCAST( AHoneyCollectingGame, pState )->IsHiding() && CanSeePlayer() &&
	     !TSTATICCAST( AHoneyCollectingGame, pState )->IsDefeated() )
	{
		SetState( STATE_ATTACK );
		m_strAnimation = TPS8( swarm_lp );
		return;
	}

	SetState( STATE_RETURN );
	m_strAnimation = TPS8( swarm_lp );
}

// $Barnyard: FUNCTION 0048bbb0
void ABeeSwarm::UpdateAlert( TFLOAT a_fDeltaTime )
{
	m_fStateTime += a_fDeltaTime;

	if ( m_fStateTime > 2.0f )
	{
		SetAngry( TTRUE );

		if ( CanSeePlayer() )
			SetState( STATE_ATTACK );
		else
			SetState( STATE_PATROL );

		m_fStateTime = 0.0f;
	}
}

// $Barnyard: FUNCTION 0048a2d0
void ABeeSwarm::RenderCallback( TModelInstance* a_pInstance, void* a_pUserData )
{
	ABeeSwarm*        pSwarm  = TSTATICCAST( ABeeSwarm, a_pUserData );
	TRenderInterface* pRender = TRenderInterface::GetSingleton();

	TMatrix44 matOldLightColour = pRender->GetLightColour();
	TVector4  vOldAmbientColour = pRender->GetCurrentContext()->GetAmbientColour();

	// Pulses between white and red
	TFLOAT fPulse = TFLOAT( ( sin( TMath::TWO_PI * g_oSystemManager.GetScheduler()->GetTotalTime() ) + 1.0 ) * 0.5 );
	TFLOAT fRed   = 0.8f * fPulse + 1.0f - fPulse;
	TFLOAT fGreen = 0.2f * fPulse + 1.0f - fPulse;
	TFLOAT fBlue  = 0.2f * fPulse + 1.0f - fPulse;

	pRender->GetCurrentContext()->SetAmbientColour( TVector4( fRed, fGreen, fBlue, 1.0f ) );

	TMatrix44 matLightColour;
	matLightColour.AsBasisVector4( BASISVECTOR_RIGHT ) = TVector4( fRed, fGreen, fBlue, 1.0f );
	TUtil::MemSet( &matLightColour.AsBasisVector4( BASISVECTOR_UP ), 0, sizeof( TVector4 ) * 3 );
	pRender->SetLightColourMatrix( matLightColour );

	pSwarm->m_fnRenderCallback( a_pInstance, pSwarm->m_pRenderCallbackData );

	pRender->GetCurrentContext()->SetAmbientColour( vOldAmbientColour );
	pRender->SetLightColourMatrix( matOldLightColour );
}

// NOTE: folded with AVeggiePatchThief::GetSteerClass (004cabd0) in retail
TClass* ABeeSwarm::GetSteerClass()
{
	return &TGetClass( AInteriorSteer );
}
