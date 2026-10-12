#include "pch.h"
#include <Render/TCameraObject.h>
#include "AVeggiePatchThief.h"
#include "AVeggiePatchVeggie.h"
#include "AUnitManager.h"
#include "ARandom.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleSystem.h"
#include "Sound/ASoundManager.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Render/ARenderer.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionManager.h"

#include <Math/TMath.h>
#include <Math/TQuaternion.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004caba0, CreateObject 004cb560, CreateObjectInPlace 004cb5c0 (the constructor is inlined in both)
TDEFINE_CLASS( AVeggiePatchThief );

TPSTRING8_DECLARE( cb_llocator );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( cb_ss3 );
TPSTRING8_DECLARE( falloff );
TPSTRING8_DECLARE( getoff );
TPSTRING8_DECLARE( geton );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( itemidle_lp );
TPSTRING8_DECLARE( itemrun_lp );
TPSTRING8_DECLARE( passfence );
TPSTRING8_DECLARE( pickup );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( scared );
TPSTRING8_DECLARE( slingshot_draw );
TPSTRING8_DECLARE( slingshot_fire );
TPSTRING8_DECLARE( thiefspawn1 );
TPSTRING8_DECLARE( tomato );
TPSTRING8_DECLARE( veggie_slingshot );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( yuk_end );
TPSTRING8_DECLARE( yuk_start );

// Speed of a raccoon carrying a veggie, per AVeggiePatchDefenderGame::VEGGIE_ (flt_773b44)
static const TFLOAT s_afCarrySpeeds[ AVeggiePatchDefenderGame::NUM_VEGGIE_KINDS ] = { 1.0f, 0.75f, 0.5f, 0.6f, 0.25f };

// AInteriorNetworkPath::m_uiHeightFlags of the steers of the raccoons (dword_773b40)
static TUINT32 s_uiPathHeightFlags = 0x80000;

// Sound of a tomato splash
static constexpr ASoundId SOUND_TOMATO_SPLASH = 448;

// $Barnyard: FUNCTION 004caf80
AVeggiePatchThief::AVeggiePatchThief()
{
	m_pPatch          = TNULL;
	m_pVeggie         = TNULL;
	m_pFence          = TNULL;
	m_pTarget         = TNULL;
	m_pSlingshot      = TNULL;
	m_pTomato         = TNULL;
	m_eState          = STATE_REMOVED;
	m_bTomatoDone     = TTRUE;
	m_iMapIcon        = -1;
	m_vTomatoVelocity = TVector4::VEC_ZERO;
	m_pStars          = TNULL;
	m_iHeadBone       = -1;
	m_iStarsEffect    = -1;
	m_iSplashEffect   = -1;
	m_eFlags |= FLAGS_THINK_ENABLED;
}

// $Barnyard: FUNCTION 004cad40
// $Barnyard: FUNCTION 004cabb0
// $Barnyard: FUNCTION 004cad30
AVeggiePatchThief::~AVeggiePatchThief()
{
}

// $Barnyard: FUNCTION 004cd6a0
void AVeggiePatchThief::OnEnterSpecialPathArea()
{
	m_pSteer->Command_CancelCurrentCommand();

	switch ( m_eState )
	{
		case STATE_SEEK_VEGGIE:
		case STATE_SEEK_FENCE:
			if ( !SetState( STATE_SEEK_FENCE ) )
				SetState( STATE_FLEE );
			break;
		case STATE_CARRY_VEGGIE:
			SetState( STATE_CARRY_VEGGIE );
			break;
		case STATE_FLEE:
			SetState( STATE_FLEE );
			break;
		case STATE_SLINGSHOT_GETON:
			if ( !SetState( STATE_SLINGSHOT_GETON ) )
				SetState( STATE_FLEE );
			break;
		default:
			break;
	}
}

// $Barnyard: FUNCTION 004cb600
TBOOL AVeggiePatchThief::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	m_pSlingshot = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( veggie_slingshot ) ), TNULL, 0 );
	m_pSlingshot->SetSkeletonUpdating( TFALSE, TTRUE );

	// The tomato shot by the slingshot
	m_pTomato = AUnitManager::GetSingleton()->SpawnUnit( TPS8( tomato ), TVector4::VEC_ZERO, TVector4::VEC_POSX );
	m_pTomato->ChangeSteer( &TGetClass( AStationarySteer ) );
	TSTATICCAST( AVisibleUnit, m_pTomato )->SetVisible( TFALSE );
	m_pTomato->GetSteer()->SetDirection( TVector4::VEC_POSX );

	// NOTE: AModelInstance::GetBoneID( const TCHAR* ) (00610e40) inlined
	AModelInstanceRef pModelInstance = GetAnimatableObject()->GetModelInstance();
	m_iHeadBone                      = pModelInstance->GetSkeleton()->GetBoneID( "c_head", 0 );

	m_iStarsEffect  = AParticleEffectManager::FindEffect( "KO_stars" );
	m_iSplashEffect = AParticleEffectManager::FindEffect( "tomatosplash" );

	return TTRUE;
}

// $Barnyard: FUNCTION 004cad60
void AVeggiePatchThief::OnDestroy()
{
	if ( m_pSlingshot )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pSlingshot );
		m_pSlingshot = TNULL;
	}

	if ( m_pTomato )
	{
		AUnitManager::GetSingleton()->DestroyUnit( m_pTomato );
		m_pTomato = TNULL;
	}

	if ( m_pStars )
	{
		m_pStars->GetInstance()->Kill();
		m_pStars = TNULL;
	}

	AAnimatedUnit::OnDestroy();
}

// $Barnyard: FUNCTION 004cd7d0
TBOOL AVeggiePatchThief::Think( TFLOAT a_fDeltaTime )
{
	AGUI2MiniMap* pMap = m_pPatch->pMap;
	pMap->SetIconTransform( GetMapIcon(), *m_pSteer->GetPosition(), *m_pSteer->GetDirection() );

	// The icon of a raccoon carrying a veggie pulses
	if ( IsCarryingVeggie() )
	{
		pMap->SetIconAlpha( GetMapIcon(), ( TMath::Sin( TMath::TWO_PI * m_pPatch->fTime ) + 1.0f ) * 0.5f );
		pMap->SetIconScale( GetMapIcon(), TMath::Sin( TMath::TWO_PI * m_pPatch->fTime ) * 0.5f + 1.0f );
	}

	// The player is carrying this raccoon
	if ( m_pPatch->ePlayerState == AVeggiePatchDefenderGame::PLAYERSTATE_CARRY_THIEF && m_pPatch->pCarriedThief == this )
		return TFALSE;

	if ( TSTATICCAST( AVisibleUnit, m_pTomato )->IsVisible() )
		UpdateTomato( a_fDeltaTime );

	switch ( m_eState )
	{
		case STATE_SEEK_VEGGIE:
		case STATE_CARRY_VEGGIE:
		case STATE_WAIT:
		case STATE_SEEK_FENCE:
		case STATE_BREAK_FENCE:
			// Run away from the player when he gets close
			if ( TVector4::DistanceSq( *GetPlayerUnit()->GetSteer()->GetPosition(), *m_pSteer->GetPosition() ) < 16.0f )
				SetState( STATE_SCARED );
			break;
		default:
			break;
	}

	switch ( m_eState )
	{
		case STATE_SPAWN:
			UpdateSpawn( a_fDeltaTime );
			break;
		case STATE_SEEK_VEGGIE:
			UpdateSeekVeggie( a_fDeltaTime );
			break;
		case STATE_CARRY_VEGGIE:
			UpdateCarryVeggie( a_fDeltaTime );
			break;
		case STATE_WAIT:
			UpdateWait( a_fDeltaTime );
			break;
		case STATE_MILKED:
			UpdateMilked( a_fDeltaTime );
			break;
		case STATE_SCARED:
			UpdateScared( a_fDeltaTime );
			break;
		case STATE_FLEE:
			UpdateFlee( a_fDeltaTime );
			break;
		case STATE_WANDER:
			UpdateWander( a_fDeltaTime );
			break;
		case STATE_SEEK_FENCE:
			UpdateSeekFence( a_fDeltaTime );
			break;
		case STATE_BREAK_FENCE:
			UpdateBreakFence( a_fDeltaTime );
			break;
		case STATE_OPEN_FENCE:
			UpdateOpenFence( a_fDeltaTime );
			break;
		case STATE_SLINGSHOT_GETON:
			UpdateSlingshotGetOn( a_fDeltaTime );
			break;
		case STATE_SLINGSHOT_AIM:
			UpdateSlingshotAim( a_fDeltaTime );
			break;
		case STATE_SLINGSHOT_FIRE:
			UpdateSlingshotFire( a_fDeltaTime );
			break;
		case STATE_SLINGSHOT_GETOFF:
			UpdateSlingshotGetOff( a_fDeltaTime );
			break;
		case STATE_THROWN:
			UpdateThrown( a_fDeltaTime );
			break;
		default:
			break;
	}

	UpdateAnimation( a_fDeltaTime );

	return AAnimatedUnit::Think( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004cabd0
TClass* AVeggiePatchThief::GetSteerClass()
{
	return &TGetClass( AInteriorSteer );
}

// $Barnyard: FUNCTION 004cb750
TBOOL AVeggiePatchThief::SetState( STATE_ a_eState )
{
	switch ( a_eState )
	{
		case STATE_SPAWN:
		{
			DetachSlingshotAndTomato();

			m_pSteer->SetPosition( TVector4::VEC_ZERO );
			m_pSteer->SetDirection( TVector4::VEC_POSX );

			// Place the raccoon at "thiefspawn1" of the network outside the patch
			if ( m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
			{
				AInteriorSteer*   pSteer   = TSTATICCAST( AInteriorSteer, m_pSteer );
				AInteriorNetwork* pNetwork = m_pPatch->pOutsideNetwork;

				if ( pSteer->GetCurrentNetwork() != pNetwork )
				{
					pSteer->ConnectToNetwork( pNetwork );
					pSteer->Command_SnapToGoal( TPS8( thiefspawn1 ) );
				}
			}
			else
			{
				ChangeSteer( &TGetClass( AInteriorSteer ) );

				AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
				pSteer->ConnectToNetwork( m_pPatch->pOutsideNetwork );
				pSteer->Command_SnapToGoal( TPS8( thiefspawn1 ) );
				pSteer->SetPathHeightFlags( s_uiPathHeightFlags );
			}

			m_fTimer = 0.5f;
			m_pSteer->SetDirection( TVector4::VEC_POSX );
			m_eFlags &= ~FLAGS_MOVING;
			SetHashed( TTRUE );
			SetNoQuery( TFALSE );
			SetVisible( TTRUE );
			MoveToList( m_pPatch->oThieves );

			if ( m_pStars )
			{
				m_pStars->GetInstance()->Stop();
				m_pStars = TNULL;
			}

			m_pPatch->pMap->SetIconVisible( m_iMapIcon, TTRUE );
			break;
		}
		case STATE_SEEK_VEGGIE:
		{
			ASteer* pSteer = m_pSteer;
			m_pVeggie      = FindRandomVeggie();

			if ( !m_pVeggie )
				return SetState( STATE_FLEE );

			pSteer->Command_CancelCurrentCommand();

			if ( !TSTATICCAST( AInteriorSteer, pSteer )->SeekPosition( *m_pVeggie->GetSteer()->GetPosition() ) )
			{
				pSteer->Command_CancelCurrentCommand();
				return TFALSE;
			}

			pSteer->SetDesiredSpeed( m_pPatch->pThiefParams->fSeekVeggieSpeed );

			m_pPatch->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
			m_pPatch->pMap->SetIconScale( GetMapIcon(), 1.0f );
			m_pPatch->pMap->SetIconVisible( GetMapIcon(), TTRUE );
			m_eFlags |= FLAGS_MOVING;
			break;
		}
		case STATE_CARRY_VEGGIE:
		{
			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );

			if ( pSteer->GetCurrentNetwork() == m_pPatch->pInsideNetwork )
			{
				// Run to the nearest open gate
				TVector4 vPosition;
				pSteer->CopyPosition( vPosition );
				m_pFence = FindFence( TTRUE, vPosition, TTRUE );

				if ( m_pFence )
				{
					pSteer->Command_CancelCurrentCommand();

					if ( !pSteer->SeekPosition( vPosition ) )
						return TFALSE;

					pSteer->SetDesiredSpeed( s_afCarrySpeeds[ m_pVeggie->GetKind() ] * m_pPatch->pThiefParams->fCarrySpeed );

					m_pPatch->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
					m_pPatch->pMap->SetIconScale( GetMapIcon(), 1.0f );
					m_pPatch->pMap->SetIconVisible( GetMapIcon(), TTRUE );
					m_eFlags |= FLAGS_MOVING;
					break;
				}

				// No open gate, wait a bit
				a_eState = STATE_WAIT;
			}
			else if ( pSteer->GetCurrentNetwork() == m_pPatch->pOutsideNetwork )
			{
				// Outside, run away
				pSteer->Command_CancelCurrentCommand();
				pSteer->Command_SeekGoal( TPS8( thiefspawn1 ) );
				break;
			}

			m_fTimer = 3.0f;
			pSteer->Command_CancelCurrentCommand();
			pSteer->Command_WanderPathfindingSpecific();
			break;
		}
		case STATE_WAIT:
		{
			ASteer* pSteer = m_pSteer;
			m_fTimer       = 3.0f;
			pSteer->Command_CancelCurrentCommand();
			pSteer->Command_WanderPathfindingSpecific();
			break;
		}
		case STATE_PICKUP:
			m_pSteer->Command_CancelCurrentCommand();
			GetAnimatableObject()->PlayAnimation( TPS8( pickup ) );
			break;
		case STATE_REMOVED:
		{
			if ( m_eState == STATE_CARRY_VEGGIE )
			{
				m_pSteer->Command_CancelCurrentCommand();
				DetachUnit( m_pVeggie, TTRUE, TFALSE );
				m_pVeggie->Hide();
				m_pVeggie = TNULL;
			}

			SetVisible( TFALSE );

			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AStationarySteer ) ) )
				ChangeSteer( &TGetClass( AStationarySteer ) );

			SetHashed( TFALSE );
			SetNoQuery( TTRUE );
			m_pPatch->pMap->SetIconVisible( GetMapIcon(), TFALSE );
			m_eFlags &= ~FLAGS_MOVING;

			// Out of sight
			TVector4 vPosition( 0.0f, 0.0f, -1000000.0f, 1.0f );
			m_pSteer->SetPosition( vPosition );

			MoveToList( m_pPatch->oFreeThieves );
			break;
		}
		case STATE_MILKED:
		{
			switch ( m_eState )
			{
				case STATE_MILKED:
				case STATE_SLINGSHOT_GETON:
				case STATE_SLINGSHOT_GETOFF:
				case STATE_PICKED_UP:
				case STATE_THROWN:
				case STATE_PASS_IN:
				case STATE_PASS_OUT_VEGGIE:
				case STATE_PASS_OUT:
					return TFALSE;
				case STATE_SLINGSHOT_AIM:
				case STATE_SLINGSHOT_FIRE:
					break;
				default:
				{
					// Only inside the patch
					AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );

					if ( pSteer->GetCurrentNetwork() != m_pPatch->pInsideNetwork )
						return TFALSE;

					pSteer->Command_CancelCurrentCommand();
					break;
				}
			}

			switch ( m_eState )
			{
				case STATE_SCARED:
					if ( !GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )
						break;
					// Fallthrough
				case STATE_CARRY_VEGGIE:
				case STATE_WAIT:
				{
					// Drop the veggie
					TVector4 vPosition = *m_pSteer->GetPosition();
					vPosition.y -= 1.0f;
					vPosition.w = 1.0f;
					DetachUnit( m_pVeggie, TTRUE, TFALSE );
					m_pVeggie->Drop( vPosition );
					m_pVeggie = TNULL;
					break;
				}
				case STATE_SLINGSHOT_GETON:
				case STATE_SLINGSHOT_AIM:
				case STATE_SLINGSHOT_FIRE:
					DetachSlingshotAndTomato();
					break;
				default:
					break;
			}

			m_pPatch->iNumMilkHits++;

			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( ( m_eState == STATE_SLINGSHOT_AIM || m_eState == STATE_SLINGSHOT_FIRE ) ? TPS8( falloff ) : TPS8( yuk_start ) );
			m_eFlags &= ~FLAGS_MOVING;
			m_fTimer = m_pPatch->pThiefParams->fMilkedTime;

			if ( !m_pStars )
			{
				m_pStars = AParticleEffectManager::GetSingleton()->PlayEffect( m_iStarsEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGY );

				if ( m_pStars )
					m_pStars->AttachToBone( GetAnimatableObject()->GetModelInstance(), m_iHeadBone, TTRUE, TFALSE );
			}

			break;
		}
		case STATE_SCARED:
		{
			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			pSteer->EnableKinematicsObstructionStop( TFALSE );
			pSteer->Command_CancelCurrentCommand();

			// Face away from the player
			AUnit* pPlayer = GetPlayerUnit();

			TVector4 vDirection;
			pSteer->CopyPosition( vDirection );
			vDirection.Substract( *pPlayer->GetSteer()->GetPosition() );
			vDirection.y = 0.0f;
			vDirection.Normalise();
			pSteer->SetDesiredDirection( vDirection );

			m_fTimer = m_pPatch->pThiefParams->fScaredTime;

			m_pPatch->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
			m_pPatch->pMap->SetIconScale( GetMapIcon(), 1.0f );
			m_pPatch->pMap->SetIconVisible( GetMapIcon(), TTRUE );
			m_eFlags |= FLAGS_MOVING;
			break;
		}
		case STATE_FLEE:
		{
			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
			{
				// On the slingshot
				if ( m_eState != STATE_PICKED_UP && m_eState != STATE_THROWN )
					return SetState( STATE_SLINGSHOT_GETOFF );

				return TFALSE;
			}

			if ( m_pVeggie && GetAttachedUnit( TPS8( cb_rlocator ), -1 ) == m_pVeggie )
				return TFALSE;

			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			DetachSlingshotAndTomato();

			if ( m_pStars )
			{
				m_pStars->GetInstance()->Kill();
				m_pStars = TNULL;
			}

			GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
			m_eFlags &= ~FLAGS_MOVING;

			if ( pSteer->GetCurrentNetwork() != m_pPatch->pInsideNetwork )
			{
				if ( pSteer->GetCurrentNetwork() == m_pPatch->pOutsideNetwork )
				{
					// Outside, run away
					pSteer->Command_CancelCurrentCommand();
					pSteer->Command_SeekGoal( TPS8( thiefspawn1 ) );
					break;
				}

				// Not on a network, wander
				m_fTimer = 1.5f;
				pSteer->Command_CancelCurrentCommand();
				pSteer->Command_WanderPathfindingSpecific();
				pSteer->SetDesiredSpeed( m_pPatch->pThiefParams->fScaredSpeed );
				m_eFlags |= FLAGS_MOVING;
				break;
			}

			// Run to the nearest open gate
			TVector4 vPosition;
			pSteer->CopyPosition( vPosition );
			m_pFence = FindFence( TTRUE, vPosition, TTRUE );

			if ( !m_pFence )
			{
				// None, wander
				a_eState = STATE_WANDER;
				m_fTimer = 1.5f;
				pSteer->Command_CancelCurrentCommand();
				pSteer->Command_WanderPathfindingSpecific();
				pSteer->SetDesiredSpeed( m_pPatch->pThiefParams->fScaredSpeed );
				m_eFlags |= FLAGS_MOVING;
				break;
			}

			pSteer->Command_CancelCurrentCommand();

			if ( !pSteer->SeekPosition( vPosition ) )
				return TFALSE;

			pSteer->SetDesiredSpeed( m_pPatch->pThiefParams->fSeekFenceSpeed );
			break;
		}
		case STATE_WANDER:
		{
			ASteer* pSteer = m_pSteer;
			m_fTimer       = 1.5f;
			pSteer->Command_CancelCurrentCommand();
			pSteer->Command_WanderPathfindingSpecific();
			pSteer->SetDesiredSpeed( m_pPatch->pThiefParams->fScaredSpeed );
			m_eFlags |= FLAGS_MOVING;
			break;
		}
		case STATE_SEEK_FENCE:
		{
			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );

			if ( m_pPatch->iNumGates == 0 )
				return TFALSE;

			// Run to the nearest gate from the outside
			TVector4 vPosition;
			pSteer->CopyPosition( vPosition );
			m_pFence = FindFence( TFALSE, vPosition, TFALSE );
			pSteer->Command_CancelCurrentCommand();

			if ( !pSteer->SeekPosition( vPosition ) )
				return TFALSE;

			pSteer->SetDesiredSpeed( m_pPatch->pThiefParams->fSeekFenceSpeed );

			m_pPatch->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
			m_pPatch->pMap->SetIconScale( GetMapIcon(), 1.0f );
			m_pPatch->pMap->SetIconVisible( GetMapIcon(), TTRUE );
			m_eFlags |= FLAGS_MOVING;
			break;
		}
		case STATE_BREAK_FENCE:
			m_pSteer->Command_CancelCurrentCommand();
			m_fTimer = m_pPatch->pThiefParams->fFenceBreakTime;
			break;
		case STATE_OPEN_FENCE:
			m_pSteer->Command_CancelCurrentCommand();

			// NOTE: retail returns TFALSE (and stays in STATE_BREAK_FENCE) when the gate starts opening,
			// STATE_BREAK_FENCE then waits for the gate to be open
			if ( m_pFence->Open( this ) )
				return TFALSE;

			break;
		case STATE_SLINGSHOT_GETON:
		{
			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AStationarySteer ) ) )
				ChangeSteer( &TGetClass( AStationarySteer ) );

			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( TPS8( geton ) );

			m_pPatch->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
			m_pPatch->pMap->SetIconScale( GetMapIcon(), 1.0f );
			m_pPatch->pMap->SetIconVisible( GetMapIcon(), TTRUE );
			m_eFlags &= ~FLAGS_MOVING;

			SetVisible( TTRUE );
			SetHashed( TTRUE );
			SetNoQuery( TFALSE );
			MoveToList( m_pPatch->oThieves );
			break;
		}
		case STATE_SLINGSHOT_AIM:
		{
			AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
			AAnimatableObject*        pTomato         = TSTATICCAST( AAnimatedUnit, m_pTomato )->GetAnimatableObject();

			m_pSlingshot->SetSkeletonUpdating( TTRUE, TTRUE );
			TSTATICCAST( AVisibleUnit, m_pTomato )->SetVisible( TTRUE );

			if ( m_pSlingshot->GetParentObject() )
				pAnimObjManager->DetachObject( m_pSlingshot, GetAnimatableObject() );

			if ( pTomato->GetParentObject() )
				pAnimObjManager->DetachObject( pTomato, pTomato->GetParentObject() );

			// The tomato in the right hand, the slingshot in the left hand
			pAnimObjManager->AttachObject( pTomato, GetAnimatableObject(), TPString8(), TPS8( cb_rlocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
			pAnimObjManager->AttachObject( m_pSlingshot, GetAnimatableObject(), TPString8(), TPS8( cb_llocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

			m_pTarget = GetPlayerUnit();

			// Then the tomato in the slingshot
			pAnimObjManager->DetachObject( pTomato, GetAnimatableObject() );
			pAnimObjManager->AttachObject( pTomato, m_pSlingshot, TPString8(), TPS8( cb_ss3 ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

			GetAnimatableObject()->PlayAnimation( TPS8( slingshot_draw ) );
			m_pSlingshot->PlayAnimation( TPS8( slingshot_draw ) );

			m_fTimer = m_pPatch->pThiefParams->fSlingshotAimTime;
			m_eFlags |= FLAGS_MOVING;
			SetHashed( TTRUE );
			SetNoQuery( TFALSE );
			break;
		}
		case STATE_SLINGSHOT_FIRE:
		{
			AAnimatableObjectManager::GetSingleton()->DetachObject( TSTATICCAST( AAnimatedUnit, m_pTomato )->GetAnimatableObject(), m_pSlingshot );

			// Aim at the target, higher when it's far away
			TVector4 vDirection;
			m_pTarget->GetSteer()->CopyPosition( vDirection );
			vDirection.Substract( *m_pSteer->GetPosition() );

			TFLOAT fAngle = TMath::ASin( vDirection.NormaliseMag() * 0.0245f ) * 0.5f;

			TVector4 vAxis;
			vAxis.CrossProduct( vDirection, TVector4::VEC_NEGY );

			TVector4 vVelocity;
			TVector4::RotateAroundAxis( vDirection, vAxis, fAngle, vVelocity );

			// Launch the tomato from the slingshot
			TMatrix44         oBoneTransform;
			AModelInstanceRef pSlingshotModel = m_pSlingshot->GetModelInstance();
			pSlingshotModel->GetBoneTransform( pSlingshotModel->GetBoneIndex( TPS8( cb_ss3 ) ), oBoneTransform );

			TVector4 vTomatoPosition = oBoneTransform.GetTranslation();
			m_pTomato->GetSteer()->SetPosition( vTomatoPosition );

			m_vTomatoVelocity.Multiply( vVelocity, 20.0f );
			m_bTomatoDone = TFALSE;

			GetAnimatableObject()->PlayAnimation( TPS8( slingshot_fire ) );
			m_pSlingshot->PlayAnimation( TPS8( slingshot_fire ) );
			break;
		}
		case STATE_SLINGSHOT_GETOFF:
			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( TPS8( getoff ) );
			m_eFlags &= ~FLAGS_MOVING;
			SetHashed( TFALSE );
			SetNoQuery( TTRUE );
			break;
		case STATE_PICKED_UP:
		{
			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AStationarySteer ) ) )
			{
				ChangeSteer( &TGetClass( AStationarySteer ) );
				m_eFlags &= ~FLAGS_MOVING;
			}

			// Drop the veggie
			AUnit*              pAttached = GetAttachedUnit( TPS8( cb_rlocator ), -1 );
			AVeggiePatchVeggie* pVeggie   = TNULL;
			TVector4            vPosition;

			if ( m_pVeggie && pAttached == m_pVeggie )
			{
				vPosition = *m_pSteer->GetPosition();
				vPosition.y -= 1.0f;
				vPosition.w = 1.0f;
				DetachUnit( m_pVeggie, TTRUE, TFALSE );
				pVeggie = m_pVeggie;
			}
			else if ( pAttached && pAttached->GetClass()->IsA( &TGetClass( AVeggiePatchVeggie ) ) )
			{
				vPosition = *m_pSteer->GetPosition();
				vPosition.y -= 1.0f;
				vPosition.w = 1.0f;
				DetachUnit( pAttached, TTRUE, TFALSE );
				pVeggie = TSTATICCAST( AVeggiePatchVeggie, pAttached );
			}

			if ( pVeggie )
			{
				pVeggie->Drop( vPosition );
				m_pVeggie = TNULL;
			}

			if ( m_pStars )
			{
				m_pStars->GetInstance()->StopLooping();
				m_pStars = TNULL;
			}

			break;
		}
		case STATE_THROWN:
			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AStationarySteer ) ) )
			{
				ChangeSteer( &TGetClass( AStationarySteer ) );
				m_eFlags &= ~FLAGS_MOVING;
			}

			m_pPatch->pMap->SetIconVisible( m_iMapIcon, TFALSE );
			break;
		case STATE_PASS_IN:
		case STATE_PASS_OUT:
			if ( !( a_eState == STATE_PASS_IN ? m_pFence->PassIn( this ) : m_pFence->PassOut( this ) ) )
				return TFALSE;

			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( TPS8( passfence ) );
			ChangeSteer( &TGetClass( AStationarySteer ) );
			m_eFlags &= ~FLAGS_MOVING;
			break;
		case STATE_PASS_OUT_VEGGIE:
			if ( !m_pFence->PassOut( this ) )
				return TFALSE;

			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( TPS8( passfence ) );
			ChangeSteer( &TGetClass( AStationarySteer ) );
			m_eFlags &= ~FLAGS_MOVING;
			m_pVeggie->SetLost();
			break;
		default:
			break;
	}

	m_eState = a_eState;
	return TTRUE;
}

// $Barnyard: FUNCTION 004cd710
void AVeggiePatchThief::OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	AUnit* pUnit = a_pEvent->pUnitProxy.Get() ? a_pEvent->pUnitProxy->GetUnit() : TNULL;

	if ( pUnit != this )
		return;

	if ( a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_PICKUP )
	{
		if ( m_pVeggie->PickUp() )
		{
			AttachUnit( m_pVeggie, TPS8( cb_rlocator ) );
			SetState( STATE_CARRY_VEGGIE );
		}
		else
		{
			// Somebody else took it
			SetState( STATE_SEEK_VEGGIE );
		}
	}
	else if ( a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_PUTDOWN )
	{
		DetachUnit( m_pVeggie, TTRUE, TFALSE );
		m_pVeggie->Hide();
		m_pVeggie = TNULL;
	}
}

// $Barnyard: FUNCTION 004cabe0
void AVeggiePatchThief::UpdateMoveState( TFLOAT a_fSpeed )
{
	switch ( m_eMoveState )
	{
		case MOVESTATE_IDLE:
			if ( a_fSpeed >= 0.2f )
				m_eMoveState = MOVESTATE_WALK;
			break;
		case MOVESTATE_WALK:
			if ( a_fSpeed < 0.1f )
				m_eMoveState = MOVESTATE_IDLE;
			else if ( a_fSpeed >= 4.0f )
				m_eMoveState = MOVESTATE_RUN;
			break;
		case MOVESTATE_RUN:
			if ( a_fSpeed < 3.0f )
				m_eMoveState = MOVESTATE_WALK;
			break;
		default:
			m_eMoveState = MOVESTATE_IDLE;
			break;
	}
}

// $Barnyard: FUNCTION 004cac70
TINT AVeggiePatchThief::GetMapIcon()
{
	if ( m_iMapIcon < 0 )
	{
		m_iMapIcon = m_pPatch->pMap->AddIcon( *m_pSteer->GetPosition(), *m_pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_ENEMY, 0.1f );
		m_pPatch->pMap->SetIconVisible( m_iMapIcon, TFALSE );
	}

	return m_iMapIcon;
}

// $Barnyard: FUNCTION 004cb030
void AVeggiePatchThief::UpdateTomato( TFLOAT a_fDeltaTime )
{
	if ( m_bTomatoDone || !TSTATICCAST( AVisibleUnit, m_pTomato )->IsVisible() )
		return;

	TVector4 vPosition;
	m_pTomato->GetSteer()->CopyPosition( vPosition );

	TVector4 vDirection = m_vTomatoVelocity;
	TFLOAT   fLength    = vDirection.NormaliseMag() * a_fDeltaTime;

	// The world
	TFLOAT                    fWorldDistance = fLength;
	ACollisionModelRayHitList oHitList;
	oHitList.Clear();

	if ( ATerrainInterface::GetSingleton()->CollideRay( vPosition, vDirection, fWorldDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG( 0x80000 ), TFALSE ) )
	{
		if ( fWorldDistance <= 0.0f )
		{
			// Inside something already
			ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_TOMATO_SPLASH, vPosition );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, vPosition, oHitList.aHits[ 0 ].GetNormal() );

			m_bTomatoDone     = TTRUE;
			m_vTomatoVelocity = TVector4::VEC_ZERO;
			TSTATICCAST( AVisibleUnit, m_pTomato )->SetVisible( TFALSE );
			return;
		}
	}
	else
	{
		fWorldDistance = TMath::MAXFLOAT;
	}

	// The units (ignoring the raccoon and the tomato)
	ARayData         oRay( vPosition, vDirection, fLength );
	ACollisionHit    oHit;
	TRef<AUnitProxy> oHitUnit;
	AUnit*           apIgnore[ 2 ];
	apIgnore[ 0 ] = this;
	apIgnore[ 1 ] = m_pTomato;

	if ( ACollisionManager::CollideRayUnits( oRay, fLength, oHitUnit, oHit, 2, apIgnore ) &&
	     oHit.fDistance < fWorldDistance && oHit.fDistance < fLength && oHitUnit.Get() )
	{
		TVector4 vHitPosition;
		vHitPosition.Multiply( vDirection, oHit.fDistance );
		vHitPosition.x = vPosition.x + vHitPosition.x;
		vHitPosition.y = vPosition.y + vHitPosition.y;
		vHitPosition.z = vPosition.z + vHitPosition.z;

		// The splash faces the camera
		TVector4 vNormal;
		vNormal.Substract( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), vHitPosition );
		vNormal.Normalise();

		ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_TOMATO_SPLASH, vHitPosition );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, vHitPosition, vNormal );

		m_bTomatoDone     = TTRUE;
		m_vTomatoVelocity = TVector4::VEC_ZERO;
		TSTATICCAST( AVisibleUnit, m_pTomato )->SetVisible( TFALSE );

		m_pGame->OnTomatoHit( oHitUnit );
	}
	else if ( fWorldDistance < fLength && oHitList.iNumHits != 0 )
	{
		ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_TOMATO_SPLASH, vPosition );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, vPosition, oHitList.aHits[ 0 ].GetNormal() );

		m_bTomatoDone     = TTRUE;
		m_vTomatoVelocity = TVector4::VEC_ZERO;
		TSTATICCAST( AVisibleUnit, m_pTomato )->SetVisible( TFALSE );
	}
	else
	{
		// Keep flying
		TVector4 vNewPosition;
		vNewPosition.Multiply( m_vTomatoVelocity, a_fDeltaTime );
		vNewPosition.x = vNewPosition.x + vPosition.x;
		vNewPosition.y = vNewPosition.y + vPosition.y;
		vNewPosition.z = vNewPosition.z + vPosition.z;
		m_pTomato->GetSteer()->SetPosition( vNewPosition );

		m_vTomatoVelocity.y = a_fDeltaTime * 9.8f + m_vTomatoVelocity.y;
	}
}

// $Barnyard: FUNCTION 004cd3e0
void AVeggiePatchThief::UpdateAnimation( TFLOAT a_fDeltaTime )
{
	if ( m_eState == STATE_PICKUP || m_eState == STATE_REMOVED || m_eState == STATE_THROWN || m_eState == STATE_SLINGSHOT_AIM ||
	     m_eState == STATE_OPEN_FENCE || m_eState == STATE_MILKED || m_eState == STATE_SCARED )
		return;

	AToshiAnimationInterface& rAnimInterface = GetAnimatableObject()->GetToshiAnimInterface();
	ASteer*                   pSteer         = m_pSteer;

	UpdateMoveState( pSteer->GetMeasuredSpeed() );

	TPString8 strIdle;
	TPString8 strWalk;
	TPString8 strRun;

	if ( m_eState < STATE_CARRY_VEGGIE || m_eState > STATE_WAIT )
	{
		strIdle = TPS8( idle_lp );
		strWalk = TPS8( walk_lp );
		strRun  = TPS8( run_lp );
	}
	else
	{
		// Carrying a veggie
		strIdle = TPS8( itemidle_lp );
		strWalk = TPS8( itemrun_lp );
		strRun  = TPS8( itemrun_lp );
	}

	const AVeggiePatchThiefAnimData* pAnimData = m_pPatch->pThiefAnimData;

	switch ( m_eMoveState )
	{
		case MOVESTATE_IDLE:
			if ( !rAnimInterface.IsAnimPlaying( strIdle, TFALSE ) )
				rAnimInterface.PlayAnim( strIdle, -1.0f, TFALSE );
			break;
		case MOVESTATE_WALK:
		{
			AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), strWalk );

			if ( !pAnimRef )
			{
				if ( rAnimInterface.IsAnimPlaying( strWalk, TFALSE ) )
					break;

				pAnimRef = rAnimInterface.PlayAnim( strWalk, -1.0f, TFALSE );

				if ( !pAnimRef )
					break;
			}

			TFLOAT fSpeed = ( strWalk == TPS8( itemrun_lp ) ) ? pSteer->GetDesiredSpeed() / pAnimData->fItemRunSpeed : pSteer->GetDesiredSpeed() / pAnimData->fWalkSpeed;
			rAnimInterface.SetAnimSpeed( pAnimRef, fSpeed );
			break;
		}
		case MOVESTATE_RUN:
		{
			AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), strRun );

			if ( !pAnimRef )
			{
				if ( rAnimInterface.IsAnimPlaying( strRun, TFALSE ) )
					break;

				pAnimRef = rAnimInterface.PlayAnim( strRun, -1.0f, TFALSE );

				if ( !pAnimRef )
					break;
			}

			// NOTE: the original compares the walk animation here too
			TFLOAT fSpeed = ( strWalk == TPS8( itemrun_lp ) ) ? pSteer->GetDesiredSpeed() / pAnimData->fItemRunSpeed : pSteer->GetDesiredSpeed() / pAnimData->fRunSpeed;
			rAnimInterface.SetAnimSpeed( pAnimRef, fSpeed );
			break;
		}
		default:
			break;
	}
}

// $Barnyard: FUNCTION 004cadd0
AVeggiePatchDefenderGame::Fence* AVeggiePatchThief::FindFence( TBOOL a_bInside, TVector4& a_rPosition, TBOOL a_bOnlyOpen )
{
	AVeggiePatchDefenderGame::Fence* pNearest           = TNULL;
	TFLOAT                           fNearestDistanceSq = TMath::MAXFLOAT;

	for ( TINT i = 0; i < m_pPatch->iNumGates; i++ )
	{
		AVeggiePatchDefenderGame::Fence* pFence = &m_pPatch->aFences[ m_pPatch->pGates[ i ] ];

		if ( pFence->iHealth != 0 )
			continue;

		TFLOAT fDistanceSq = TVector4::DistanceSq( a_rPosition, a_bInside ? pFence->vInside : pFence->vOutside );

		if ( !pNearest || fDistanceSq < fNearestDistanceSq )
		{
			fNearestDistanceSq = fDistanceSq;
			pNearest           = pFence;
		}
	}

	if ( pNearest )
	{
		a_rPosition = a_bInside ? pNearest->vInside : pNearest->vOutside;
		return pNearest;
	}

	if ( a_bOnlyOpen )
		return TNULL;

	// Any gate
	TINT                             iGate  = ARandom::GetSingleton()->m_oRandom.GetInt( m_pPatch->iNumGates );
	AVeggiePatchDefenderGame::Fence* pFence = &m_pPatch->aFences[ m_pPatch->pGates[ iGate ] ];
	a_rPosition                             = pFence->vOutside;

	return pFence;
}

// $Barnyard: FUNCTION 004cb4f0
AVeggiePatchVeggie* AVeggiePatchThief::FindRandomVeggie()
{
	T2DList<AVeggiePatchVeggie>& rVeggies = m_pPatch->oVeggies;

	if ( rVeggies.IsEmpty() )
		return TNULL;

	auto it      = rVeggies.Begin();
	TINT iVeggie = ARandom::GetSingleton()->m_oRandom.GetInt( rVeggies.Size() );

	for ( TINT i = iVeggie; i > 0; i-- )
		it = it.Next();

	return it.Get();
}

// $Barnyard: FUNCTION 004caf50
// $Barnyard: FUNCTION 004c7560
void AVeggiePatchThief::MoveToList( T2DList<AVeggiePatchThief>& a_rList )
{
	// NOTE: the original makes an iterator of the raccoon (004caf50) and calls T2DList::Erase (00457a20, which
	// only unlinks the node) on an unrelated list, then pushes the raccoon back to a_rList (004c7560)
	TSTATICCAST( T2DList<AVeggiePatchThief>::Node, this )->Remove();
	a_rList.PushBack( this );
}

// Inlined in SetState
void AVeggiePatchThief::DetachSlingshotAndTomato()
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	if ( m_pSlingshot->GetParentObject() )
	{
		pAnimObjManager->DetachObject( m_pSlingshot, GetAnimatableObject() );
		m_pSlingshot->SetSkeletonUpdating( TFALSE, TTRUE );
	}

	AAnimatableObject* pTomato = TSTATICCAST( AAnimatedUnit, m_pTomato )->GetAnimatableObject();

	if ( pTomato->GetParentObject() )
	{
		pAnimObjManager->DetachObject( pTomato, pTomato->GetParentObject() );
		TSTATICCAST( AVisibleUnit, m_pTomato )->SetVisible( TFALSE );
	}
}

// $Barnyard: FUNCTION 004cca10
void AVeggiePatchThief::UpdateSpawn( TFLOAT a_fDeltaTime )
{
	m_fTimer -= a_fDeltaTime;

	if ( m_fTimer <= 0.0f )
	{
		AVeggiePatchDefenderGame::PHASE_ ePhase = m_pPatch->ePhase;

		// Try again a bit later if no gate can be reached
		if ( ePhase != AVeggiePatchDefenderGame::PHASE_ENDING && ePhase != AVeggiePatchDefenderGame::PHASE_ENDED && !SetState( STATE_SEEK_FENCE ) )
			m_fTimer = 0.5f;
	}
}

// $Barnyard: FUNCTION 004cca60
void AVeggiePatchThief::UpdateSeekVeggie( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = m_pSteer;

	if ( m_pVeggie->GetState() != AVeggiePatchVeggie::STATE_GROWING )
	{
		// Somebody else took it, find another one
		m_pVeggie = TNULL;
		pSteer->Command_CancelCurrentCommand();

		if ( !SetState( STATE_SEEK_VEGGIE ) )
			SetState( STATE_FLEE );
	}
	else if ( pSteer->HasArrived() == TTRUE )
	{
		SetState( STATE_PICKUP );
	}
}

// $Barnyard: FUNCTION 004ccac0
void AVeggiePatchThief::UpdateCarryVeggie( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = m_pSteer;
	pSteer->SetDesiredSpeed( s_afCarrySpeeds[ m_pVeggie->GetKind() ] * m_pPatch->pThiefParams->fCarrySpeed );

	if ( TSTATICCAST( AInteriorSteer, pSteer )->GetCurrentNetwork() == m_pPatch->pInsideNetwork )
	{
		// The gate was closed, find another one
		if ( m_pFence->iHealth != 0 )
			SetState( STATE_CARRY_VEGGIE );
		else if ( pSteer->HasArrived() == TTRUE )
			SetState( STATE_PASS_OUT_VEGGIE );
	}
	else if ( TSTATICCAST( AInteriorSteer, pSteer )->GetCurrentNetwork() == m_pPatch->pOutsideNetwork )
	{
		if ( pSteer->HasArrived() == TTRUE )
			SetState( STATE_REMOVED );
	}
}

// $Barnyard: FUNCTION 004ccb80
void AVeggiePatchThief::UpdateWait( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = m_pSteer;
	pSteer->SetDesiredSpeed( s_afCarrySpeeds[ m_pVeggie->GetKind() ] * m_pPatch->pThiefParams->fCarrySpeed );

	m_fTimer -= a_fDeltaTime;

	if ( m_fTimer < 0.0f || pSteer->HasArrived() )
		SetState( STATE_CARRY_VEGGIE );
}

// $Barnyard: FUNCTION 004ccbf0
void AVeggiePatchThief::UpdateMilked( TFLOAT a_fDeltaTime )
{
	if ( !GetAnimatableObject()->IsAnimationFinished( TPS8( yuk_start ) ) || !GetAnimatableObject()->IsAnimationFinished( TPS8( falloff ) ) )
		return;

	if ( m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
	{
		if ( m_fTimer >= 0.0f )
		{
			m_fTimer -= a_fDeltaTime;
			return;
		}

		if ( m_pStars )
		{
			m_pStars->GetInstance()->Stop();
			m_pStars = TNULL;
		}

		if ( GetAnimatableObject()->IsAnimPlaying( TPS8( scared ), 0.0f ) )
			SetState( STATE_SCARED );
		else if ( !GetAnimatableObject()->IsAnimPlaying( TPS8( yuk_end ), 0.0f ) )
			GetAnimatableObject()->PlayAnimation( TPS8( yuk_end ) );
	}
	else
	{
		// Fell off the slingshot
		if ( m_pStars )
		{
			m_pStars->GetInstance()->Stop();
			m_pStars = TNULL;
		}

		SetState( STATE_REMOVED );
	}
}

// $Barnyard: FUNCTION 004ccd10
void AVeggiePatchThief::UpdateScared( TFLOAT a_fDeltaTime )
{
	AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
	m_fTimer -= a_fDeltaTime;

	TVector4 vDirection;
	pSteer->CopyDirection( vDirection );

	AUnit*          pPlayer          = GetPlayerUnit();
	const TVector4& rcPlayerPosition = *pPlayer->GetSteer()->GetPosition();
	const TVector4& rcPosition       = *pSteer->GetPosition();

	TVector4 vAway( rcPosition.x - rcPlayerPosition.x, 0.0f, rcPosition.z - rcPlayerPosition.z, rcPosition.w );
	TFLOAT   fDistanceSq = vAway.MagnitudeSqXZ();

	if ( fDistanceSq >= 25.0f )
	{
		// Far enough, run around randomly
		// NOTE: TVector3::RotateY (006c7820) is called on the TVector4
		TVector3 vDirection3( vDirection.x, vDirection.y, vDirection.z );
		vDirection3.RotateY( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -TMath::PI, TMath::PI * 0.25f ) );
		vDirection.x = vDirection3.x;
		vDirection.y = vDirection3.y;
		vDirection.z = vDirection3.z;
	}
	else
	{
		// Run away from the player
		vAway.Divide( TMath::Sqrt( fDistanceSq ) );
		vDirection.x = vDirection.x + vAway.x;
		vDirection.y = vDirection.y + vAway.y;
		vDirection.z = vDirection.z + vAway.z;
		vDirection.Normalise();
	}

	TFLOAT fSpeed;

	if ( IsCarryingVeggie() )
		fSpeed = s_afCarrySpeeds[ m_pVeggie->GetKind() ] * m_pPatch->pThiefParams->fScaredSpeed;
	else
		fSpeed = m_pPatch->pThiefParams->fScaredSpeed;

	vDirection.Multiply( fSpeed );
	pSteer->SetDesiredVelocity( vDirection );

	AToshiAnimationInterface& rAnimInterface = GetAnimatableObject()->GetToshiAnimInterface();
	AToshiAnimationRef*       pAnimRef       = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), TPS8( scared ) );

	if ( pAnimRef || ( pAnimRef = rAnimInterface.PlayAnim( TPS8( scared ), -1.0f, TFALSE ) ) )
		rAnimInterface.SetAnimSpeed( pAnimRef, fSpeed / m_pPatch->pThiefAnimData->fScaredSpeed );

	if ( m_fTimer < 0.0f )
	{
		TBOOL bResult;

		if ( pSteer->GetCurrentNetwork() == m_pPatch->pInsideNetwork )
		{
			if ( IsCarryingVeggie() )
			{
				SetState( STATE_CARRY_VEGGIE );
				return;
			}

			bResult = SetState( STATE_SEEK_VEGGIE );
		}
		else
		{
			if ( pSteer->GetCurrentNetwork() != m_pPatch->pOutsideNetwork )
				return;

			if ( IsCarryingVeggie() )
			{
				SetState( STATE_CARRY_VEGGIE );
				return;
			}

			bResult = SetState( STATE_SEEK_FENCE );
		}

		if ( !bResult )
			SetState( STATE_FLEE );
	}
}

// $Barnyard: FUNCTION 004ccfb0
void AVeggiePatchThief::UpdateFlee( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = m_pSteer;

	if ( TSTATICCAST( AInteriorSteer, pSteer )->GetCurrentNetwork() == m_pPatch->pInsideNetwork )
	{
		// The gate was closed, find another one
		if ( m_pFence->iHealth != 0 )
			SetState( STATE_FLEE );
		else if ( pSteer->HasArrived() == TTRUE )
			SetState( STATE_PASS_OUT );
	}
	else if ( TSTATICCAST( AInteriorSteer, pSteer )->GetCurrentNetwork() == m_pPatch->pOutsideNetwork )
	{
		if ( pSteer->HasArrived() == TTRUE )
			SetState( STATE_REMOVED );
	}
}

// $Barnyard: FUNCTION 004cd040
void AVeggiePatchThief::UpdateWander( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = m_pSteer;
	m_fTimer -= a_fDeltaTime;

	if ( m_fTimer < 0.0f || pSteer->HasArrived() )
		SetState( STATE_FLEE );
}

// $Barnyard: FUNCTION 004cd080
void AVeggiePatchThief::UpdateSeekFence( TFLOAT a_fDeltaTime )
{
	if ( m_pSteer->HasArrived() != TTRUE )
		return;

	if ( m_pFence->iHealth != 0 || m_pFence->eState != AVeggiePatchDefenderGame::Fence::STATE_OPEN )
	{
		// Closed, break it
		SetSteerPosition( m_pFence->vOutside );
		SetState( STATE_BREAK_FENCE );
	}
	else if ( SetState( STATE_PASS_IN ) )
	{
		m_pFence = TNULL;
	}
}

// $Barnyard: FUNCTION 004cd0e0
void AVeggiePatchThief::UpdateBreakFence( TFLOAT a_fDeltaTime )
{
	m_fTimer -= a_fDeltaTime;

	if ( m_fTimer >= 0.0f )
		return;

	if ( m_pFence->iHealth != 0 )
	{
		m_pFence->Hit();
		m_fTimer = m_pPatch->pThiefParams->fFenceBreakTime;
	}
	else if ( m_pFence->eState == AVeggiePatchDefenderGame::Fence::STATE_OPEN )
	{
		if ( SetState( STATE_PASS_IN ) )
			m_pFence = TNULL;
	}
	else if ( m_pFence->eState == AVeggiePatchDefenderGame::Fence::STATE_CLOSED )
	{
		SetState( STATE_OPEN_FENCE );
	}
}

// $Barnyard: FUNCTION 004cd160
void AVeggiePatchThief::UpdateOpenFence( TFLOAT a_fDeltaTime )
{
	m_fTimer -= a_fDeltaTime;

	if ( GetAnimatableObject()->GetParentObject() || !GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
		return;

	if ( m_pFence->iHealth != 0 )
	{
		SetState( STATE_BREAK_FENCE );
	}
	else if ( m_pFence->eState == AVeggiePatchDefenderGame::Fence::STATE_OPEN )
	{
		if ( SetState( STATE_PASS_IN ) )
			m_pFence = TNULL;
	}
}

// $Barnyard: FUNCTION 004cd1e0
void AVeggiePatchThief::UpdateSlingshotGetOn( TFLOAT a_fDeltaTime )
{
	if ( GetAnimatableObject()->IsAnimationFinished( TPS8( geton ) ) )
		SetState( STATE_SLINGSHOT_AIM );
}

// $Barnyard: FUNCTION 004cd240
void AVeggiePatchThief::UpdateSlingshotAim( TFLOAT a_fDeltaTime )
{
	m_fTimer -= a_fDeltaTime;

	// Face the target
	TVector4 vDirection;
	m_pTarget->GetSteer()->CopyPosition( vDirection );
	vDirection.Substract( *m_pSteer->GetPosition() );

	// NOTE: NormaliseXZ (006c7540, see AGTAStyleCameraHelper.cpp) inlined
	vDirection.y    = 0.0f;
	TFLOAT fMagSqXZ = vDirection.x * vDirection.x + vDirection.z * vDirection.z;

	if ( fMagSqXZ == 0.0f )
	{
		vDirection.x = 0.0f;
		vDirection.z = 1.0f;
	}
	else
	{
		TFLOAT fMagXZ = TMath::Sqrt( fMagSqXZ );
		vDirection.x  = 1.0f / fMagXZ * vDirection.x;
		vDirection.z  = 1.0f / fMagXZ * vDirection.z;
	}

	m_pSteer->SetDesiredDirection( vDirection );

	if ( m_fTimer <= 0.0f )
		SetState( STATE_SLINGSHOT_FIRE );
}

// $Barnyard: FUNCTION 004cd2e0
void AVeggiePatchThief::UpdateSlingshotFire( TFLOAT a_fDeltaTime )
{
	if ( GetAnimatableObject()->IsAnimationFinished( TPS8( slingshot_fire ) ) )
	{
		m_pSlingshot->SetSkeletonUpdating( TFALSE, TTRUE );
		SetState( STATE_SLINGSHOT_AIM );
	}
}

// $Barnyard: FUNCTION 004cd210
void AVeggiePatchThief::UpdateSlingshotGetOff( TFLOAT a_fDeltaTime )
{
	if ( GetAnimatableObject()->IsAnimationFinished( TPS8( getoff ) ) )
		SetState( STATE_REMOVED );
}

// $Barnyard: FUNCTION 004cd320
void AVeggiePatchThief::UpdateThrown( TFLOAT a_fDeltaTime )
{
	TVector4 vPosition;
	vPosition.Multiply( m_vVelocity, a_fDeltaTime );

	const TVector4* pPosition = m_pSteer->GetPosition();
	vPosition.x               = vPosition.x + pPosition->x;
	vPosition.y               = vPosition.y + pPosition->y;
	vPosition.z               = vPosition.z + pPosition->z;

	m_vVelocity.y = a_fDeltaTime * 9.8f + m_vVelocity.y;
	m_pSteer->SetPosition( vPosition );

	// Gone once it's out of sight
	TFLOAT fFar = ARenderer::GetSingleton()->GetFar();

	if ( TVector4::DistanceSq( *GetPlayerUnit()->GetSteer()->GetPosition(), vPosition ) > fFar * fFar )
		SetState( STATE_REMOVED );
}

AUnit* AVeggiePatchThief::GetPlayerUnit() const
{
	return m_pPatch->oPlayer.Get() ? m_pPatch->oPlayer.Get()->GetUnit() : TNULL;
}

TBOOL AVeggiePatchThief::IsCarryingVeggie()
{
	return m_pVeggie && GetAttachedUnit( TPS8( cb_rlocator ), -1 ) == m_pVeggie;
}
