#include "pch.h"
#include <Render/TCameraObject.h>
#include "AChasingChicksAI.h"
#include "AUnitManager.h"
#include "AUnitTransform.h"
#include "ASimplePlayerControlComponent.h"
#include "ARandom.h"
#include "Helpers/APlayerProgress.h"
#include "GUI/AGUI2MiniMap.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/chicks.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Units/Projectiles/ALaunchAmmo.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionCommon.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

#undef PI

TOSHI_NAMESPACE_USING

#ifdef BARNYARD_LATEST
// Scale of the movement input of the computer players (latest version: a float at 0x150 of the Wii
// ASimplePlayerControlComponent that Create sets), 0.91 on Easy
static TFLOAT s_fMovementScale = 1.0f;
#endif

// NOTE: GetClass 00508080, CreateObject 00508270, CreateObjectInPlace 005082d0 (the constructor is inlined in both)
TDEFINE_CLASS( AChasingChicksAI );

TPSTRING8_DECLARE( AREA_NA );

// Taunt played when the AI hits the player, by AUnitTransform::m_iModelIndex (dword_773d44)
static const ASoundId s_aTauntSounds[] = {
	-1,
	-1,
	soundbank::CHICKS_BUD_TAUNT,
	soundbank::CHICKS_EDDY_TAUNT,
};

// Retail calls the static NormaliseXZ of TVector4 (006c7540), see AGTAStyleCameraHelper.cpp
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

// $Barnyard: FUNCTION 00508090
AChasingChicksAI::AChasingChicksAI()
    : m_eState( STATE_SEARCH )
    , m_eMood( MOOD_NORMAL )
    , m_pGame( TNULL )
    , m_pPlayer( TNULL )
    , m_pNetwork( TNULL )
    , m_vNextPoint( TVector4::VEC_ZERO )
    , m_vLastPosition( TVector4::VEC_ZERO )
    , m_iNumStuck( 0 )
    , m_fStuckCheckTime( 0.0f )
    , m_fStuckElapsed( 0.0f )
    , m_pTargetChick( TNULL )
    , m_pTargetRival( TNULL )
    , m_pTargetPen( TNULL )
    , m_pTargetBottle( TNULL )
    , m_fAttackCooldown( 0.0f )
    , m_iUnk5C0( 0 )
    , m_fDefensiveness( 0.0f )
    , m_fAggressiveness( 0.0f )
    , m_fDecisionTime( 0.0f )
    , m_pControl( TNULL )
    , m_iMapIcon( -1 )
    , m_iTauntSound( -1 )
    , m_iTauntCue( -1 )
{
	m_oPath.m_fRadius = 0.5f;
}

// $Barnyard: FUNCTION 00508310
// $Barnyard: FUNCTION 005081d0
AChasingChicksAI::~AChasingChicksAI()
{
	if ( m_pNetwork )
	{
		m_pNetwork->DisconnectUnit( &m_oPath, this );
		m_pNetwork = TNULL;
	}
}

// $Barnyard: FUNCTION 00508330
TBOOL AChasingChicksAI::Create( AUnitTransform* a_pTransform )
{
	if ( !AUnitPlayer::Create( a_pTransform ) )
		return TFALSE;

	m_pControl = TSTATICCAST( ASimplePlayerControlComponent, FindComponent( &TGetClass( AControlComponent ) ) );

#ifdef BARNYARD_LATEST
	// Latest version (Wii 80253274): slower computer players on Easy
	s_fMovementScale = APlayerProgress::GetSingleton()->IsEasyDifficulty() ? 0.91f : 1.0f;
#endif

	// Sets m_bNPCControlled of the aimed squirt launcher and deactivates its reticle
	TSTATICCAST( AAimedSquirtProjectileLauncher, GetLauncher( 1 ) )->SetNPCControlled( TTRUE );

	m_iTauntSound = s_aTauntSounds[ a_pTransform->m_iModelIndex ];
	return TTRUE;
}

// $Barnyard: FUNCTION 00508da0
void AChasingChicksAI::SetNetwork( AInteriorNetwork* a_pNetwork )
{
	if ( m_pNetwork )
		m_pNetwork->DisconnectUnit( &m_oPath, this );

	m_pNetwork = a_pNetwork;
	m_pNetwork->ConnectUnit( &m_oPath, this, *m_pSteer->GetPosition() );
	m_oPath.m_fRadius = m_pSteer->GetTraits()->m_fRadius;

	SetState( STATE_SEARCH );
}

// NOTE: inlined in retail (ASimplePlayerControlComponent::SetMovementInput 00508060)
void AChasingChicksAI::SetMovementInput( TFLOAT a_fX, TFLOAT a_fY )
{
#ifdef BARNYARD_LATEST
	m_pControl->SetMovementInput( a_fX * s_fMovementScale, a_fY * s_fMovementScale );
#else
	m_pControl->SetMovementInput( a_fX, a_fY );
#endif
}

// NOTE: inlined in retail
void AChasingChicksAI::SendCommand( AInputCommand a_eCommand )
{
	TInputInterface::InputEvent oDownEvent( TNULL, -1, TInputInterface::EVENT_TYPE_GONE_DOWN );
	TInputInterface::InputEvent oUpEvent( TNULL, -1, TInputInterface::EVENT_TYPE_GONE_UP );

	m_pControl->ProcessCommand( a_eCommand, &oDownEvent );
	m_pControl->ProcessCommand( a_eCommand, &oUpEvent );
}

// NOTE: inlined in retail
AChasingChicksHotSpot* AChasingChicksAI::PickPen()
{
	if ( m_pGame->pAIData->iPenSelection == 1 )
		return m_pGame->FindClosestPen( *m_pSteer->GetPosition(), TMath::MAXFLOAT );
	else if ( m_pGame->pAIData->iPenSelection == 2 )
		return m_pGame->PickPenOfBestKind();
	else
		return m_pGame->PickRandomPen();
}

// $Barnyard: FUNCTION 00508860
TBOOL AChasingChicksAI::SetState( STATE a_eState )
{
	switch ( a_eState )
	{
		case STATE_SEARCH:
		{
			m_pNetwork->ResetPath( &m_oPath );

			if ( m_eMood == MOOD_DEFENSIVE )
			{
				// Wander somewhere around the centre
				TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

				for ( TINT i = 4; i != 0; i-- )
				{
					// NOTE: the angle is in degrees but passed to cos/sin as is
					TFLOAT fAngle    = rRandom.GetFloatMinMax( 0.0f, 360.0f );
					TFLOAT fDistance = rRandom.GetFloatMinMax( 5.0f, 20.0f );

					TVector4 vPoint(
					    TMath::Cos( fAngle ) * fDistance + m_pGame->vCentre.x,
					    m_pGame->vCentre.y,
					    TMath::Sin( fAngle ) * fDistance + m_pGame->vCentre.z,
					    1.0f
					);

					TINT iArea = m_pNetwork->FindClosestArea( vPoint );

					if ( iArea > -1 && *m_pNetwork->m_pINetwork->m_pAreas[ iArea ].m_strName != TPS8( AREA_NA ) )
					{
						if ( m_pNetwork->BuildPathToRandomPointInArea( &m_oPath, iArea ) )
						{
							UpdateNextPoint();
							m_eState = a_eState;
							return TTRUE;
						}

						m_pNetwork->ResetPath( &m_oPath );
						return TFALSE;
					}
				}
			}

			if ( m_pNetwork->BuildPathToRandomArea( &m_oPath ) )
			{
				UpdateNextPoint();
				m_eState = a_eState;
				return TTRUE;
			}

			m_pNetwork->ResetPath( &m_oPath );
			return TFALSE;
		}
		case STATE_CHASE_CHICK:
			m_pNetwork->ResetPath( &m_oPath );

			if ( m_pNetwork->BuildPathToContainingArea( &m_oPath, *m_pTargetChick->GetSteer()->GetPosition() ) )
			{
				UpdateNextPoint();
				m_eState = a_eState;
				return TTRUE;
			}

			m_pNetwork->ResetPath( &m_oPath );
			return TFALSE;
		case STATE_GRAB_CHICK:
		case STATE_RAM_RIVAL:
			m_pNetwork->ResetPath( &m_oPath );
			m_eState = a_eState;
			return TTRUE;
		case STATE_CHASE_RIVAL:
			m_pNetwork->ResetPath( &m_oPath );

			if ( m_pNetwork->BuildPathToContainingArea( &m_oPath, *m_pTargetRival->GetSteer()->GetPosition() ) )
			{
				UpdateNextPoint();
				m_eState = a_eState;
				return TTRUE;
			}

			m_pNetwork->ResetPath( &m_oPath );
			return TFALSE;
		case STATE_GO_TO_PEN:
			m_pNetwork->ResetPath( &m_oPath );
			m_pTargetPen = PickPen();

			if ( !m_pTargetPen )
				return TFALSE;

			if ( m_pNetwork->BuildPathToContainingArea( &m_oPath, m_pTargetPen->vPosition ) )
			{
				UpdateNextPoint();
				m_eState = a_eState;
				return TTRUE;
			}

			m_pNetwork->ResetPath( &m_oPath );
			return TFALSE;
		case STATE_AIM_SQUIRT:
			SetActiveLauncher( GetLauncher( 1 ), TTRUE );
			// Fall through
		case STATE_AIM:
			m_pNetwork->ResetPath( &m_oPath );
			m_eState = a_eState;
			return TTRUE;
		case STATE_GET_MILK:
			if ( !m_pTargetBottle )
				return TFALSE;

			m_pNetwork->ResetPath( &m_oPath );

			if ( m_pNetwork->BuildPathToContainingArea( &m_oPath, m_pTargetBottle->vPosition ) )
			{
				UpdateNextPoint();
				m_eState = a_eState;
				return TTRUE;
			}

			m_pNetwork->ResetPath( &m_oPath );
			return TFALSE;
		default:
			m_eState = a_eState;
			return TTRUE;
	}
}

// $Barnyard: FUNCTION 00508380
void AChasingChicksAI::UpdateMood( TFLOAT a_fDeltaTime )
{
	m_fAggressiveness = TMath::LERP( m_fAggressiveness, 0.0f, a_fDeltaTime );
	m_fDefensiveness  = TMath::LERP( m_fDefensiveness, 0.0f, a_fDeltaTime );

	AChasingChicksAIData* pAIData = m_pGame->pAIData;

	if ( &m_pGame->aPlayers[ m_pGame->iLeader ] == m_pPlayer )
	{
		// Leading, gets defensive
		TFLOAT fValue = TFLOAT( m_pPlayer->iScore - m_pGame->aPlayers[ m_pGame->iRunnerUp ].iScore ) * pAIData->fLeadingRate * a_fDeltaTime;

		if ( *m_pGame->pTimeLimit - *m_pGame->pElapsedTime < pAIData->fLeadingTime )
			fValue += fValue;

		m_fDefensiveness = fValue + m_fDefensiveness;
	}
	else
	{
		// Trailing, gets aggressive
		TFLOAT fValue = TFLOAT( m_pGame->aPlayers[ m_pGame->iLeader ].iScore - m_pPlayer->iScore ) * pAIData->fTrailingRate * a_fDeltaTime;

		if ( *m_pGame->pElapsedTime < pAIData->fTrailingTime )
			fValue += fValue;

		m_fAggressiveness = fValue + m_fAggressiveness;
	}

	TFLOAT fMood = m_fAggressiveness - m_fDefensiveness;

	switch ( m_eMood )
	{
		case MOOD_NORMAL:
			if ( TFLOAT( GetActiveLauncher()->GetAmmo()->GetMaxAmmo() ) * 0.5f > GetActiveLauncher()->GetAmmo()->GetAmmo() )
				m_eMood = MOOD_LOW_AMMO;
			else if ( fMood > 1.5f )
				m_eMood = MOOD_AGGRESSIVE;
			else if ( fMood < -1.5f )
				m_eMood = MOOD_DEFENSIVE;
			break;
		case MOOD_AGGRESSIVE:
			if ( TFLOAT( GetActiveLauncher()->GetAmmo()->GetMaxAmmo() ) * 0.25f > GetActiveLauncher()->GetAmmo()->GetAmmo() )
				m_eMood = MOOD_LOW_AMMO;
			else if ( fMood < 1.0f )
				m_eMood = MOOD_NORMAL;
			break;
		case MOOD_DEFENSIVE:
			if ( TFLOAT( GetActiveLauncher()->GetAmmo()->GetMaxAmmo() ) * 0.25f > GetActiveLauncher()->GetAmmo()->GetAmmo() )
				m_eMood = MOOD_LOW_AMMO;
			else if ( fMood > -1.0f )
				m_eMood = MOOD_NORMAL;
			break;
		case MOOD_LOW_AMMO:
			if ( TFLOAT( GetActiveLauncher()->GetAmmo()->GetMaxAmmo() ) * 0.5f >= GetActiveLauncher()->GetAmmo()->GetAmmo() )
				return;

			if ( fMood > 1.5f )
				m_eMood = MOOD_AGGRESSIVE;
			else if ( fMood > -1.0f )
				m_eMood = MOOD_NORMAL;
			else if ( fMood < -1.5f )
				m_eMood = MOOD_DEFENSIVE;
			break;
	}
}

// $Barnyard: FUNCTION 00508630
void AChasingChicksAI::UpdateStuck( TFLOAT a_fElapsedTime )
{
	ASteer*  pSteer    = m_pSteer;
	TVector4 vPosition = *m_pSteer->GetPosition();
	TVector2 vPush     = TVector2::VEC_ZERO;

	m_pNetwork->UpdateHardAvoidance( &m_oPath, vPosition, &vPush, TTRUE );

	if ( vPush.y * vPush.y + vPush.x * vPush.x > 0.0f )
		m_pSteer->SetPosition( vPosition );

	APlayerSteer::STATE eSteerState = TSTATICCAST( APlayerSteer, pSteer )->GetState();

	if ( eSteerState < APlayerSteer::STATE_JUMPING || eSteerState > APlayerSteer::STATE_FALLING )
	{
		const TVector2& rcMovement = m_pControl->GetMovementInput();

		if ( rcMovement.x * rcMovement.x + rcMovement.y * rcMovement.y <= 0.0f )
		{
			m_iNumStuck = 0;
		}
		else
		{
			TVector4 vMoved = vPosition - m_vLastPosition;
			vMoved.w        = vPosition.w;

			TFLOAT fSpeed = vMoved.Magnitude() / a_fElapsedTime;
			TFLOAT fRatio;

			switch ( m_pSteer->GetMovementType() )
			{
				case ASteer::MOVEMENT_WALK:
					fRatio = 0.1f;
					break;
				case ASteer::MOVEMENT_RUN:
					fRatio = 0.3f;
					break;
				case ASteer::MOVEMENT_SPRINT:
					fRatio = 0.5f;
					break;
				default:
					fRatio = 0.0f;
					break;
			}

			if ( m_pSteer->GetMeasuredSpeed() * fRatio <= fSpeed )
			{
				m_iNumStuck = 0;
			}
			else
			{
				// Moving way slower than expected, jump
				SendCommand( AInputCommand_Jump );
				m_iNumStuck++;
			}
		}
	}

	m_vLastPosition = vPosition;
}

// $Barnyard: FUNCTION 00508bc0
AChasingChicksChick* AChasingChicksAI::FindChick( TFLOAT& a_rfDistance )
{
	TFLOAT               fBestScore = TMath::MAXFLOAT;
	AChasingChicksChick* pBestChick = TNULL;

	// Chicks targeted by the other AI rivals
	AChasingChicksChick* apOtherTargets[ 2 ] = { TNULL, TNULL };

	if ( m_pGame->iNumPlayers > 1 )
	{
		AChasingChicksChick** ppOtherTarget = apOtherTargets;

		for ( TINT i = 1; i < m_pGame->iNumPlayers; i++ )
		{
			AUnitProxy*       pProxy = m_pGame->aPlayers[ i ].oUnit;
			AChasingChicksAI* pAI    = pProxy ? TSTATICCAST( AChasingChicksAI, pProxy->GetUnit() ) : TNULL;

			// NOTE: retail doesn't check pAI here
			if ( pAI != this )
				*ppOtherTarget++ = pAI->m_pTargetChick;
		}
	}

	TINT                    iNumFound = 0;
	AUnitHash::QueryResult* pResults  = AUnitManager::GetSingleton()->FindUnits( *m_pSteer->GetPosition(), 40.0f, &iNumFound, TFALSE, 420, TFALSE, TNULL );

	if ( iNumFound == 0 )
		return TNULL;

	for ( TUINT i = 0; i < TUINT( iNumFound ); i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit && pUnit->GetClass()->IsA( &TGetClass( AChasingChicksChick ) ) )
		{
			AChasingChicksChick* pChick = TSTATICCAST( AChasingChicksChick, pUnit );

			if ( TUINT( pChick->GetState() ) <= AChasingChicksChick::STATE_FLEEING )
			{
				TFLOAT fDistanceSq     = TVector4::DistanceSq( *pChick->GetSteer()->GetPosition(), *m_pSteer->GetPosition() );
				TINT   iNumTargeting   = ( pChick == apOtherTargets[ 0 ] ) + ( pChick == apOtherTargets[ 1 ] );
				TFLOAT fWeightedDistSq = ( TFLOAT( iNumTargeting ) * 0.25f + 1.0f ) * fDistanceSq;
				TFLOAT fScore          = ( ARandom::GetSingleton()->m_oRandom.GetFloat() + 0.5f ) * fWeightedDistSq;

				if ( fScore < fBestScore )
				{
					fBestScore   = fScore;
					a_rfDistance = fDistanceSq;
					pBestChick   = pChick;
				}
			}
		}
	}

	if ( pBestChick )
		a_rfDistance = TMath::Sqrt( a_rfDistance );

	return pBestChick;
}

// $Barnyard: FUNCTION 00508e00
AUnit* AChasingChicksAI::FindRival( TFLOAT& a_rfDistance )
{
	TFLOAT fBestDistanceSq = a_rfDistance * a_rfDistance;
	AUnit* pBestRival      = TNULL;

	if ( m_pGame->iNumPlayers <= 0 )
		return TNULL;

	for ( TINT i = 0; i < m_pGame->iNumPlayers; i++ )
	{
		AChasingChicksPlayer& rPlayer = m_pGame->aPlayers[ i ];

		if ( ( !rPlayer.pHotSpot || rPlayer.eState != AChasingChicksPlayer::STATE_CARRY ) &&
		     ( m_eMood == MOOD_DEFENSIVE || rPlayer.eState == AChasingChicksPlayer::STATE_CARRY ) )
		{
			AUnit* pUnit = rPlayer.oUnit->GetUnit();

			if ( pUnit && pUnit != this )
			{
				TVector4 vToRival = *pUnit->GetSteer()->GetPosition() - *m_pSteer->GetPosition();
				vToRival.w        = pUnit->GetSteer()->GetPosition()->w;

				TFLOAT fDistanceSq = vToRival.MagnitudeSq();

				// Prefer rivals carrying a chick
				if ( rPlayer.eState == AChasingChicksPlayer::STATE_CARRY )
					fDistanceSq *= 0.25f;

				// NOTE: retail compares the squared distance with a_rfDistance instead of
				// fBestDistanceSq, so it picks the last rival within sqrt( a_rfDistance )
				if ( fDistanceSq < a_rfDistance )
				{
					fBestDistanceSq = fDistanceSq;
					pBestRival      = pUnit;
				}
			}
		}
	}

	if ( pBestRival )
		a_rfDistance = TMath::Sqrt( fBestDistanceSq );

	return pBestRival;
}

// $Barnyard: FUNCTION 00508f40
void AChasingChicksAI::UpdateSearch( TFLOAT a_fDeltaTime )
{
	if ( m_pPlayer->pChick )
	{
		if ( m_pPlayer->eState == AChasingChicksPlayer::STATE_FREE )
		{
			// A chick is in reach, pick it up
			m_pPlayer->uiFlags |= AChasingChicksPlayer::FLAGS_INTERACT;
			m_pTargetChick = m_pPlayer->pChick;
			m_pNetwork->ResetPath( &m_oPath );
			m_eState = STATE_GRAB_CHICK;
		}
		else if ( m_pPlayer->eState == AChasingChicksPlayer::STATE_CARRY )
		{
			// NOTE: inlined in retail
			SetState( STATE_GO_TO_PEN );
			return;
		}
	}

	TVector4 vDirection = m_vNextPoint - *m_pSteer->GetPosition();
	vDirection.w        = m_vNextPoint.w;
	vDirection.y        = 0.0f;

	TFLOAT fDistanceSq = vDirection.MagnitudeSq();

	if ( fDistanceSq < 0.25f )
	{
		m_pNetwork->UpdatePath( &m_oPath );
		UpdateNextPoint();

		if ( m_oPath.m_oPath.m_iNumPathAreas < 1 || m_oPath.m_iStartArea == m_oPath.m_oPath.GetPathAreaIndex( m_oPath.m_oPath.m_iNumPathAreas - 1 ) )
		{
			m_pNetwork->ResetPath( &m_oPath );
			SetState( STATE_SEARCH );
		}
	}

	m_fDecisionTime -= a_fDeltaTime;

	if ( m_fDecisionTime < 0.0f )
	{
		m_fDecisionTime = 0.1f;

		// NOTE: the SetState calls below are inlined in retail
		AChasingChicksAIData* pAIData = m_pGame->pAIData;

		switch ( m_eMood )
		{
			case MOOD_AGGRESSIVE:
			{
				TFLOAT fDistance = pAIData->fAggressiveChickDistance;
				m_pTargetChick   = FindChick( fDistance );

				if ( m_pTargetChick )
				{
					if ( SetState( STATE_CHASE_CHICK ) ) return;
					break;
				}

				fDistance      = pAIData->fAggressiveRivalDistance;
				m_pTargetRival = FindRival( fDistance );

				if ( m_pTargetRival && SetState( STATE_CHASE_RIVAL ) ) return;
				break;
			}
			case MOOD_DEFENSIVE:
			{
				TFLOAT fDistance = pAIData->fDefensiveRivalDistance;
				m_pTargetRival   = FindRival( fDistance );

				if ( m_pTargetRival && SetState( STATE_CHASE_RIVAL ) ) return;
				break;
			}
			case MOOD_LOW_AMMO:
			{
				TFLOAT fDistance = 500.0f;
				m_pTargetBottle  = m_pGame->pMilkBottles->FindClosestBottle( *m_pSteer->GetPosition(), fDistance );

				if ( m_pTargetBottle && SetState( STATE_GET_MILK ) ) return;
			}
				// Fall through
			case MOOD_NORMAL:
			{
				TFLOAT               fChickDistance = pAIData->fChickDistance;
				AChasingChicksChick* pChick         = FindChick( fChickDistance );

				TFLOAT fRivalDistance = ( pAIData->fRivalDistance >= fChickDistance ) ? fChickDistance : pAIData->fRivalDistance;
				AUnit* pRival         = FindRival( fRivalDistance );

				TFLOAT fBottleDistance = 500.0f;

				if ( TFLOAT( GetActiveLauncher()->GetAmmo()->GetMaxAmmo() ) <= GetActiveLauncher()->GetAmmo()->GetAmmo() )
					m_pTargetBottle = TNULL;
				else
					m_pTargetBottle = m_pGame->pMilkBottles->FindClosestBottle( *m_pSteer->GetPosition(), fBottleDistance );

				if ( pChick )
				{
					if ( pRival && fRivalDistance < fChickDistance )
					{
						// The rival is closer than the chick
						if ( fBottleDistance < fRivalDistance && m_pTargetBottle && SetState( STATE_GET_MILK ) ) return;

						m_pTargetRival = pRival;
						if ( SetState( STATE_CHASE_RIVAL ) ) return;
						break;
					}

					if ( fBottleDistance < fChickDistance && m_pTargetBottle && SetState( STATE_GET_MILK ) ) return;

					m_pTargetChick = pChick;
					if ( SetState( STATE_CHASE_CHICK ) ) return;
					break;
				}

				if ( !pRival )
				{
					if ( m_pTargetBottle && SetState( STATE_GET_MILK ) ) return;
					break;
				}

				if ( fBottleDistance < fRivalDistance && m_pTargetBottle && SetState( STATE_GET_MILK ) ) return;

				m_pTargetRival = pRival;
				if ( SetState( STATE_CHASE_RIVAL ) ) return;
				break;
			}
		}
	}

	vDirection.Divide( TMath::Sqrt( fDistanceSq ) );
	SetMovementInput( vDirection.x, vDirection.z );
}

// $Barnyard: FUNCTION 00509750
void AChasingChicksAI::UpdateChaseChick( TFLOAT a_fDeltaTime )
{
	m_fDecisionTime -= a_fDeltaTime;

	if ( m_fDecisionTime < 0.0f )
	{
		m_fDecisionTime = 0.1f;

		TFLOAT fChickDistance = 40.0f;
		m_pTargetChick        = FindChick( fChickDistance );

		TFLOAT fRivalDistance = 20.0f;
		AUnit* pRival         = FindRival( fRivalDistance );

		if ( pRival && fRivalDistance < fChickDistance )
		{
			// NOTE: inlined in retail
			m_pTargetRival = pRival;
			SetState( STATE_CHASE_RIVAL );
			return;
		}

		if ( m_pPlayer->pChick )
		{
			if ( m_pPlayer->eState == AChasingChicksPlayer::STATE_FREE )
			{
				m_pPlayer->uiFlags |= AChasingChicksPlayer::FLAGS_INTERACT;
				m_pTargetChick = m_pPlayer->pChick;
				m_pNetwork->ResetPath( &m_oPath );
				m_eState = STATE_GRAB_CHICK;
				return;
			}

			if ( m_pPlayer->eState == AChasingChicksPlayer::STATE_CARRY )
			{
				// NOTE: inlined in retail
				SetState( STATE_GO_TO_PEN );
				return;
			}
		}
	}

	AChasingChicksChick::STATE eChickState;

	if ( !m_pTargetChick ||
	     ( ( eChickState = m_pTargetChick->GetState() ) >= AChasingChicksChick::STATE_HELD &&
	       ( eChickState <= AChasingChicksChick::STATE_THROWN || eChickState == AChasingChicksChick::STATE_RESPAWN ) ) )
	{
		SetState( STATE_SEARCH );
		return;
	}

	TVector4 vDirection = m_vNextPoint - *m_pSteer->GetPosition();
	vDirection.w        = m_vNextPoint.w;
	vDirection.y        = 0.0f;

	TFLOAT fDistanceSq = vDirection.MagnitudeSq();

	if ( fDistanceSq < 0.25f )
	{
		m_pNetwork->UpdatePath( &m_oPath );
		UpdateNextPoint();

		if ( m_oPath.m_oPath.m_iNumPathAreas < 1 || m_oPath.m_iStartArea == m_oPath.m_oPath.GetPathAreaIndex( m_oPath.m_oPath.m_iNumPathAreas - 1 ) )
		{
			// NOTE: SetState is inlined in retail
			m_pNetwork->ResetPath( &m_oPath );
			SetState( STATE_CHASE_CHICK );
		}
	}

	TINT    iChickArea;
	ASteer* pChickSteer = m_pTargetChick->GetSteer();

	if ( m_pTargetChick->GetState() == AChasingChicksChick::STATE_JUMPING )
	{
		TVector4 vChickPosition;
		pChickSteer->CopyPosition( vChickPosition );
		iChickArea = m_pNetwork->FindContainingArea( vChickPosition );
	}
	else
	{
		iChickArea = TSTATICCAST( AInteriorSteer, pChickSteer )->GetPath().m_iStartArea;
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail doesn't check for an empty path (after ResetPath above or a failed
	// BuildPathToContainingArea) and reads the area before the path's head
	TBOOL bAtPathEnd = m_oPath.m_oPath.m_iNumPathAreas > 0 && m_oPath.m_iStartArea == m_oPath.m_oPath.GetPathAreaIndex( m_oPath.m_oPath.m_iNumPathAreas - 1 );
#else
	TBOOL bAtPathEnd = m_oPath.m_iStartArea == m_oPath.m_oPath.GetPathAreaIndex( m_oPath.m_oPath.m_iNumPathAreas - 1 );
#endif

	if ( m_oPath.m_iStartArea == iChickArea || bAtPathEnd )
	{
		// Close enough to walk straight to it
		m_pNetwork->ResetPath( &m_oPath );
		m_eState = STATE_GRAB_CHICK;
	}

	if ( m_iNumStuck > 3 )
	{
		// NOTE: inlined in retail
		SetState( STATE_CHASE_CHICK );
	}

	vDirection.Divide( TMath::Sqrt( fDistanceSq ) );
	SetMovementInput( vDirection.x, vDirection.z );
}

// $Barnyard: FUNCTION 00509bf0
void AChasingChicksAI::UpdateGrabChick( TFLOAT a_fDeltaTime )
{
	m_fDecisionTime -= a_fDeltaTime;

	if ( m_fDecisionTime < 0.0f )
	{
		m_fDecisionTime = 0.1f;

		TFLOAT fChickDistance = 40.0f;
		m_pTargetChick        = FindChick( fChickDistance );

		TFLOAT fRivalDistance = 10.0f;
		AUnit* pRival         = FindRival( fRivalDistance );

		if ( pRival && fRivalDistance < fChickDistance )
		{
			// NOTE: inlined in retail
			m_pTargetRival = pRival;
			SetState( STATE_CHASE_RIVAL );
			return;
		}

		if ( m_pPlayer->pChick )
		{
			if ( m_pPlayer->eState == AChasingChicksPlayer::STATE_FREE )
			{
				m_pPlayer->uiFlags |= AChasingChicksPlayer::FLAGS_INTERACT;
				m_pTargetChick = m_pPlayer->pChick;
			}
			else if ( m_pPlayer->eState == AChasingChicksPlayer::STATE_CARRY )
			{
				// NOTE: inlined in retail
				SetState( STATE_GO_TO_PEN );
				return;
			}
		}
	}

	AChasingChicksChick::STATE eChickState;

	if ( !m_pTargetChick ||
	     ( ( eChickState = m_pTargetChick->GetState() ) >= AChasingChicksChick::STATE_HELD &&
	       ( eChickState <= AChasingChicksChick::STATE_THROWN || eChickState == AChasingChicksChick::STATE_RESPAWN ) ) )
	{
		SetState( STATE_SEARCH );
		return;
	}

	m_pTargetChick->GetSteer()->CopyPosition( m_vNextPoint );

	TVector4 vDirection = m_vNextPoint - *m_pSteer->GetPosition();
	vDirection.w        = m_vNextPoint.w;
	vDirection.y        = 0.0f;

	if ( vDirection.NormaliseMag() > 20.0f || m_iNumStuck > 3 )
		SetState( STATE_SEARCH );

	SetMovementInput( vDirection.x, vDirection.z );
}

// $Barnyard: FUNCTION 00509ec0
void AChasingChicksAI::UpdateChaseRival( TFLOAT a_fDeltaTime )
{
	TINT iRival = -1;

	for ( TINT i = 0; i < m_pGame->iNumPlayers; i++ )
	{
		if ( m_pGame->aPlayers[ i ].oUnit.Get() == m_pTargetRival->GetSelfProxy().Get() )
		{
			iRival = i;
			break;
		}
	}

	AChasingChicksPlayer* pRival = &m_pGame->aPlayers[ iRival ];

	switch ( m_eMood )
	{
		case MOOD_DEFENSIVE:
			if ( pRival->eState == AChasingChicksPlayer::STATE_STUNNED )
			{
				// Done with this one
				m_fDefensiveness -= 3.0f;
				SetState( STATE_SEARCH );
				return;
			}
			break;
		case MOOD_LOW_AMMO:
		{
			TFLOAT fDistance = 500.0f;
			m_pTargetBottle  = m_pGame->pMilkBottles->FindClosestBottle( *m_pSteer->GetPosition(), fDistance );

			// NOTE: inlined in retail
			if ( m_pTargetBottle && SetState( STATE_GET_MILK ) ) return;
		}
			// Fall through
		case MOOD_NORMAL:
		case MOOD_AGGRESSIVE:
			if ( pRival->eState != AChasingChicksPlayer::STATE_CARRY )
			{
				SetState( STATE_SEARCH );
				return;
			}
			break;
	}

	if ( m_fDecisionTime < 0.0f )
	{
		m_fDecisionTime = 0.1f;

		TVector4 vRivalPosition;
		m_pTargetRival->GetSteer()->CopyPosition( vRivalPosition );

		TVector4 vToRival = vRivalPosition - *m_pSteer->GetPosition();
		vToRival.w        = vRivalPosition.w;

		TFLOAT                fDistance = vToRival.NormaliseMag();
		AChasingChicksAIData* pAIData   = m_pGame->pAIData;

		if ( fDistance < pAIData->fRamDistance )
		{
			ARayData oRay( *m_pSteer->GetPosition(), vRivalPosition );

			if ( !ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fDistance, 0 ) )
			{
				// NOTE: SetState( STATE_RAM_RIVAL ) inlined in retail
				m_pNetwork->ResetPath( &m_oPath );
				m_eState = STATE_RAM_RIVAL;
			}
		}
		else if ( fDistance < pAIData->fSquirtDistance && TVector4::DotProduct3( vToRival, *m_pSteer->GetDirection() ) > 0.707f )
		{
			ARayData oRay( *m_pSteer->GetPosition(), vRivalPosition );

			if ( !ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fDistance, 0 ) && m_fAttackCooldown < 0.0f )
			{
				// Squirt the rival with the aimed launcher
				SetActiveLauncher( GetLauncher( 1 ), TTRUE );

				if ( GetLauncher( 1 ) == GetActiveLauncher() )
				{
					TSTATICCAST( AAimedSquirtProjectileLauncher, GetLauncher( 1 ) )->FireAt( vRivalPosition, TFALSE );
					m_fAttackCooldown = m_pGame->pAIData->fAttackCooldown;
				}

				SetActiveLauncher( GetLauncher( 0 ), TTRUE );
			}
		}
	}

	TVector4 vDirection = m_vNextPoint - *m_pSteer->GetPosition();
	vDirection.w        = m_vNextPoint.w;
	vDirection.y        = 0.0f;

	TFLOAT fDistanceSq = vDirection.MagnitudeSq();

	if ( fDistanceSq < 0.25f )
	{
		m_pNetwork->UpdatePath( &m_oPath );
		UpdateNextPoint();

		if ( m_oPath.m_oPath.m_iNumPathAreas < 1 || m_oPath.m_iStartArea == m_oPath.m_oPath.GetPathAreaIndex( m_oPath.m_oPath.m_iNumPathAreas - 1 ) )
		{
			// NOTE: SetState is inlined in retail
			m_pNetwork->ResetPath( &m_oPath );
			SetState( STATE_CHASE_RIVAL );
		}
	}

	vDirection.Divide( TMath::Sqrt( fDistanceSq ) );
	SetMovementInput( vDirection.x, vDirection.z );
}

// $Barnyard: FUNCTION 0050a420
void AChasingChicksAI::UpdateRamRival( TFLOAT a_fDeltaTime )
{
	TINT iRival = -1;

	for ( TINT i = 0; i < m_pGame->iNumPlayers; i++ )
	{
		if ( m_pGame->aPlayers[ i ].oUnit.Get() == m_pTargetRival->GetSelfProxy().Get() )
		{
			iRival = i;
			break;
		}
	}

	AChasingChicksPlayer* pRival = &m_pGame->aPlayers[ iRival ];

	switch ( m_eMood )
	{
		case MOOD_DEFENSIVE:
			if ( pRival->eState == AChasingChicksPlayer::STATE_STUNNED )
			{
				m_fDefensiveness -= 3.0f;
				SetState( STATE_SEARCH );
				return;
			}
			break;
		case MOOD_LOW_AMMO:
		{
			TFLOAT fDistance = 500.0f;
			m_pTargetBottle  = m_pGame->pMilkBottles->FindClosestBottle( *m_pSteer->GetPosition(), fDistance );

			// NOTE: inlined in retail
			if ( m_pTargetBottle && SetState( STATE_GET_MILK ) ) return;
		}
			// Fall through
		case MOOD_NORMAL:
		case MOOD_AGGRESSIVE:
			if ( pRival->eState != AChasingChicksPlayer::STATE_CARRY )
			{
				SetState( STATE_SEARCH );
				return;
			}
			break;
	}

	TVector4 vRivalPosition;
	m_pTargetRival->GetSteer()->CopyPosition( vRivalPosition );

	TVector4 vDirection = vRivalPosition - *m_pSteer->GetPosition();
	vDirection.w        = vRivalPosition.w;

	TFLOAT fDistance = vDirection.NormaliseMag();

	if ( m_fDecisionTime < 0.0f )
	{
		m_fDecisionTime = 0.1f;

		if ( fDistance < m_pGame->pAIData->fRamDistance )
		{
			if ( m_fAttackCooldown < 0.0f )
			{
				SendCommand( AInputCommand_Shoot );
				m_fAttackCooldown = m_pGame->pAIData->fAttackCooldown;
			}

			// Slow down while ramming
			NormaliseXZ( vDirection );
			vDirection.Multiply( 0.3f );
			SetMovementInput( vDirection.x, vDirection.z );
			return;
		}

		if ( fDistance > 20.0f || m_iNumStuck > 3 )
			SetState( STATE_SEARCH );
	}

	vDirection.y = 0.0f;
	vDirection.Normalise();
	SetMovementInput( vDirection.x, vDirection.z );
}

// $Barnyard: FUNCTION 0050a7c0
void AChasingChicksAI::UpdateGoToPen( TFLOAT a_fDeltaTime )
{
	AChasingChicksPlayer::STATE_ ePlayerState = m_pPlayer->eState;

	if ( ePlayerState == AChasingChicksPlayer::STATE_FREE || ePlayerState == AChasingChicksPlayer::STATE_STUNNED || !m_pPlayer->pChick )
	{
		SetState( STATE_SEARCH );
		return;
	}

	if ( m_pTargetPen->bClosed )
	{
		// The pen was closed, pick another one
		// NOTE: inlined in retail
		SetState( STATE_GO_TO_PEN );
	}
	else if ( ( m_oPath.m_oPath.m_iNumPathAreas != 0 || HASANYFLAG( m_oPath.m_oPath.m_uiFlags, AInteriorNetworkPath::BuildPathState::FLAGS_NOT_FOUND ) ) && TVector4::DistanceSqXZ( *m_pSteer->GetPosition(), m_pTargetPen->vPosition ) >= 0.25f )
	{
		TVector4 vDirection = m_vNextPoint;
		vDirection -= *m_pSteer->GetPosition();

		TFLOAT fDistanceSq = vDirection.MagnitudeSqXZ();

		if ( fDistanceSq >= 0.25f )
		{
			if ( fDistanceSq < 1.0f )
				vDirection.Multiply( 0.5f );
		}
		else
		{
			m_pNetwork->UpdatePath( &m_oPath );
			UpdateNextPoint();
		}

		vDirection.Divide( TMath::Sqrt( fDistanceSq ) );
		SetMovementInput( vDirection.x, vDirection.z );
	}
	else
	{
		// At the pen, aim and throw the chick
		// NOTE: SetState is inlined in retail
		m_pNetwork->ResetPath( &m_oPath );
		SetState( STATE_AIM );
	}
}

// $Barnyard: FUNCTION 0050aa00
void AChasingChicksAI::UpdateAim( TFLOAT a_fDeltaTime )
{
	AChasingChicksPlayer::STATE_ ePlayerState = m_pPlayer->eState;

	if ( ePlayerState == AChasingChicksPlayer::STATE_FREE || ePlayerState == AChasingChicksPlayer::STATE_STUNNED || !m_pPlayer->pChick )
	{
		SetState( STATE_SEARCH );
		return;
	}

	if ( ePlayerState != AChasingChicksPlayer::STATE_THROW )
	{
		TVector4 vDirection = m_pGame->vCentre - *m_pSteer->GetPosition();
		vDirection.w        = m_pGame->vCentre.w;
		vDirection.y        = 0.0f;
		vDirection.Normalise();

		ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pSteer );
		pSteer->StopAndFace( vDirection );

		// Throw once facing the centre
		if ( TVector4::DotProduct3( *pSteer->GetDirection(), vDirection ) > 0.98f )
			m_pPlayer->uiFlags |= AChasingChicksPlayer::FLAGS_INTERACT;
	}
}

// $Barnyard: FUNCTION 0050aaf0
void AChasingChicksAI::UpdateGetMilk( TFLOAT a_fDeltaTime )
{
	if ( !m_pTargetBottle || m_pTargetBottle->bCollected )
	{
		SetState( STATE_SEARCH );
		m_pTargetBottle = TNULL;
		return;
	}

	TVector4 vDirection = m_vNextPoint - *m_pSteer->GetPosition();
	vDirection.w        = m_vNextPoint.w;

	TFLOAT fDistanceSq = vDirection.MagnitudeSqXZ();

	if ( fDistanceSq >= 0.25f )
	{
		if ( fDistanceSq < 1.0f )
			vDirection.Multiply( 0.5f );
	}
	else
	{
		m_pNetwork->UpdatePath( &m_oPath );
		UpdateNextPoint();
	}

	vDirection.Divide( TMath::Sqrt( fDistanceSq ) );
	SetMovementInput( vDirection.x, vDirection.z );
}

// $Barnyard: FUNCTION 0050ac20
void AChasingChicksAI::UpdateAimSquirt( TFLOAT a_fDeltaTime )
{
	AChasingChicksPlayer::STATE_ ePlayerState = m_pPlayer->eState;

	if ( ePlayerState == AChasingChicksPlayer::STATE_FREE || ePlayerState == AChasingChicksPlayer::STATE_STUNNED || !m_pPlayer->pChick )
	{
		SetActiveLauncher( GetLauncher( 0 ), TTRUE );
		SetState( STATE_SEARCH );
		return;
	}

	if ( ePlayerState != AChasingChicksPlayer::STATE_THROW )
	{
		TVector4 vDirection = m_pGame->vCentre - *m_pSteer->GetPosition();
		vDirection.w        = m_pGame->vCentre.w;
		vDirection.y        = 0.0f;
		vDirection.Normalise();

		ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pSteer );
		pSteer->StopAndFace( vDirection );

		if ( TVector4::DotProduct3( *pSteer->GetDirection(), vDirection ) > 0.98f )
			m_pPlayer->uiFlags |= AChasingChicksPlayer::FLAGS_INTERACT;
	}
}

// $Barnyard: FUNCTION 0050ad30
void AChasingChicksAI::OnHitUnit( const TRef<AUnitProxy>& a_rcUnit )
{
	const AUnitProxy* pProxy = a_rcUnit.Get();

	if ( !pProxy || !pProxy->GetUnit()->GetClass()->IsA( &TGetClass( AUnitPlayer ) ) )
		return;

	TINT iPlayer = 0;

	for ( ; iPlayer < m_pGame->iNumPlayers; iPlayer++ )
	{
		if ( m_pGame->aPlayers[ iPlayer ].oUnit.Get() == pProxy )
			break;
	}

	if ( iPlayer >= m_pGame->iNumPlayers )
		return;

	// Taunt the player (always the first slot)
	if ( iPlayer == 0 && ( m_iTauntCue < 0 || !ASoundManager::GetSingleton()->IsCueUsed( m_iTauntCue ) ) )
		m_iTauntCue = ASoundManager::GetSingleton()->PlayCueAtUnit( m_iTauntSound, this );

	// Go for the chick they dropped
	AChasingChicksChick* pChick = m_pGame->aPlayers[ iPlayer ].pChick;

	if ( pChick && m_pTargetChick != pChick )
	{
		// NOTE: inlined in retail
		m_pTargetChick = pChick;
		SetState( STATE_CHASE_CHICK );
	}
}

// $Barnyard: FUNCTION 0050ae80
TBOOL AChasingChicksAI::Think( TFLOAT a_fDeltaTime )
{
	TVector4 vPosition       = *m_pSteer->GetPosition();
	m_oPath.m_vStartPosition = vPosition;

	if ( m_iMapIcon >= 0 )
	{
		m_pGame->pMap->SetIconTransform( m_iMapIcon, vPosition, *m_pSteer->GetDirection() );
	}
	else
	{
		m_iMapIcon = m_pGame->pMap->AddIcon( vPosition, *m_pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_DOT_WHITE, 0.1f );
		m_pGame->pMap->SetIconColour( m_iMapIcon, 0xffff0500 );
	}

	if ( m_pGame->ePhase > 0 && m_pGame->ePhase <= 3 )
	{
		// The game isn't running, stand still
		SetMovementInput( 0.0f, 0.0f );
		return AUnitPlayer::Think( a_fDeltaTime );
	}

	TINT iArea = m_pNetwork->FindContainingArea( vPosition );

	if ( iArea > -1 )
		m_oPath.m_iStartArea = iArea;

	m_fDecisionTime -= a_fDeltaTime;
	m_fAttackCooldown -= a_fDeltaTime;

	UpdateMood( a_fDeltaTime );

	switch ( m_eState )
	{
		case STATE_SEARCH:
			UpdateSearch( a_fDeltaTime );
			break;
		case STATE_CHASE_CHICK:
			UpdateChaseChick( a_fDeltaTime );
			break;
		case STATE_GRAB_CHICK:
			UpdateGrabChick( a_fDeltaTime );
			break;
		case STATE_CHASE_RIVAL:
			UpdateChaseRival( a_fDeltaTime );
			break;
		case STATE_RAM_RIVAL:
			UpdateRamRival( a_fDeltaTime );
			break;
		case STATE_GO_TO_PEN:
			UpdateGoToPen( a_fDeltaTime );
			break;
		case STATE_AIM:
			UpdateAim( a_fDeltaTime );
			break;
		case STATE_AIM_SQUIRT:
			UpdateAimSquirt( a_fDeltaTime );
			break;
		case STATE_GET_MILK:
			UpdateGetMilk( a_fDeltaTime );
			break;
	}

	m_fStuckCheckTime -= a_fDeltaTime;
	m_fStuckElapsed += a_fDeltaTime;

	if ( m_fStuckCheckTime < 0.0f )
	{
		m_fStuckCheckTime += 0.5f;
		UpdateStuck( m_fStuckElapsed );
		m_fStuckElapsed = 0.0f;
	}

	// Picks up milk bottles on the way (refills the ammo)
	TFLOAT                      fDistance = 2.0f;
	AMilkBottlePickups::Bottle* pBottle   = m_pGame->pMilkBottles->FindClosestBottle( *m_pSteer->GetPosition(), fDistance );

	if ( pBottle )
	{
		ALaunchAmmo* pAmmo = GetActiveLauncher()->GetAmmo();

		if ( pAmmo )
			pAmmo->SetAmmo( pAmmo->GetMaxAmmo() );

		pBottle->SetCollected( TTRUE );
	}

	return AUnitPlayer::Think( a_fDeltaTime );
}
