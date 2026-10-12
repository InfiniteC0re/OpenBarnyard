#include "pch.h"
#include <Render/TCameraObject.h>
#include "ACowTippingMiniGameState.h"
#include "ARandom.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ACharacterMovementComponent.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/Steering/ASteerKinematics.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/ANamedAnimation.h"
#include "Animation/AModel.h"
#include "Units/AKinematicsObstruction.h"
#include "FX/AParticleEffectManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "SoundBank/dagboss.h"
#include "Locale/ALocaleManager.h"
#include "Terrain/ATerrainInterface.h"

#include <Math/TQuaternion.h>
#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

#ifdef PI
#  undef PI
#endif

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( backhit );
TPSTRING8_DECLARE( default );
TPSTRING8_DECLARE( distress_idle );
TPSTRING8_DECLARE( fire );
TPSTRING8_DECLARE( fronthit );
TPSTRING8_DECLARE( gasp );
TPSTRING8_DECLARE( getup );
TPSTRING8_DECLARE( growl );
TPSTRING8_DECLARE( headhit );
TPSTRING8_DECLARE( howl );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( l );
TPSTRING8_DECLARE( push_end );
TPSTRING8_DECLARE( push_lp );
TPSTRING8_DECLARE( push_start );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( spin );
TPSTRING8_DECLARE( spin_idle );
TPSTRING8_DECLARE( tip_1rac_end );
TPSTRING8_DECLARE( tip_1rac_loop );
TPSTRING8_DECLARE( tip_1rac_start );
TPSTRING8_DECLARE( walk_lp );

// NOTE: ACowTippingMiniGameState is split over several .cpp files, this one has the [Enemies] part.

//-----------------------------------------------------------------------------
// Typed views of the provisional structs of the header (same layout, the header
// still declares plain TINT fields).
//-----------------------------------------------------------------------------

using Enemy = ACowTippingMiniGameState::Enemy;

using Cow = ACowTippingMiniGameState::Cow;

using Crate = ACowTippingMiniGameState::Crate;

static constexpr TINT NUM_PICKUP_GROUPS     = 6;
static constexpr TINT NUM_PICKUPS_PER_GROUP = 5;

#define CT_ENEMY_LIST( STATE ) ( *TREINTERPRETCAST( T2GenericSList*, &( STATE )->m_llActiveEnemies ) )
#define CT_DONE_LIST( STATE )  ( *TREINTERPRETCAST( T2GenericSList*, &( STATE )->m_llDoneEnemies ) )
#define CT_COWS( STATE )       ( TREINTERPRETCAST( Cow*, ( STATE )->m_aCows ) )
#define CT_NETWORK( STATE )    ( TREINTERPRETCAST( AInteriorNetwork*, ( STATE )->m_pNetwork ) )
#define CT_STAGE_TYPE( STATE ) ( ( STATE )->m_aStages[ ( STATE )->m_iStage ].iStageType )
#define CT_AS_FLOAT( MEMBER )  ( TREINTERPRETCAST( TFLOAT&, MEMBER ) )
#define CT_ENEMY_ANIM( ENEMY ) ( ( ENEMY )->pUnit->GetAnimatableObject() )
#define CT_OBSTRUCTION( UNIT ) ( TSTATICCAST( AKinematicsObstruction, ( UNIT )->GetObstruction() ) )

// The bit sets m_auiPickupBits, m_auiCowSideReservedBits and m_auiCowSideTakenBits have 30 bits (4 bytes each)
static void ClearBit( TUINT8* a_puiBits, TUINT a_uiBit )
{
	a_puiBits[ a_uiBit >> 3 ] &= ~( 1 << ( a_uiBit & 7 ) );
}

static TBOOL IsBitSet( const TUINT8* a_puiBits, TUINT a_uiBit )
{
	return ( a_puiBits[ a_uiBit >> 3 ] & TUINT8( 1 << ( a_uiBit & 7 ) ) ) != 0;
}

// NOTE: same as the static RotateVector of AModel.cpp (006c9740)
static void RotateVector( TVector4& a_rOutVector, const TQuaternion& a_rcQuaternion, const TVector4& a_rcVector )
{
	TQuaternion quatConjugate( -a_rcQuaternion.x, -a_rcQuaternion.y, -a_rcQuaternion.z, a_rcQuaternion.w );
	TQuaternion quatVector( a_rcVector.x, a_rcVector.y, a_rcVector.z, 0.0f );

	TQuaternion quatResult = a_rcQuaternion;
	quatResult *= quatVector;
	quatResult *= quatConjugate;

	a_rOutVector.Set( quatResult.x, quatResult.y, quatResult.z, 1.0f );
}

// NOTE: same as the static LerpClamped3 of APhysics.cpp (006b6700)
static TVector4 LerpClamped3( TVector4 a_vStart, TVector4 a_vEnd, TFLOAT a_fProgress )
{
	TFLOAT afResult[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fStart = a_vStart.AsArray()[ i ];
		TFLOAT fEnd   = a_vEnd.AsArray()[ i ];
		TFLOAT fValue = ( fEnd - fStart ) * a_fProgress + fStart;

		if ( ( fStart < fEnd && fValue > fEnd ) || ( fStart > fEnd && fValue < fEnd ) )
			fValue = fEnd;

		afResult[ i ] = fValue;
	}

	return TVector4( afResult[ 0 ], afResult[ 1 ], afResult[ 2 ], a_vStart.w );
}

// Stops the interior steer of the unit (if it's visible)
static void StopUnitSteer( AAnimatedUnit* a_pUnit )
{
	if ( a_pUnit->IsVisible() )
	{
		ASteer* pSteer = a_pUnit->GetSteer();

		if ( pSteer->IsA( &TGetClass( AInteriorSteer ) ) )
		{
			pSteer->Command_CancelCurrentCommand();
			TSTATICCAST( AInteriorSteer, pSteer )->SetMode( AInteriorSteer::MODE_STILL );
		}
	}
}

// Faces the steer along its current direction projected on the XZ plane
static void FlattenSteerDirection( ASteer* a_pSteer )
{
	TVector4 vecDirection = *a_pSteer->GetDirection();
	vecDirection.y        = 0.0f;
	vecDirection.Normalise();

	a_pSteer->SetDirection( vecDirection );
}

// $Barnyard: FUNCTION 004b0840
static TVector4* GetGoalPosition( ACowTippingMiniGameState* a_pState, TVector4* a_pOutPosition, TINT a_iIndex, const TCHAR* a_szName )
{
	AInteriorGoal* pGoal = TREINTERPRETCAST( AInteriorGoal*, ACowTippingMiniGameState::FindRandomGoal( a_pState, a_iIndex, const_cast<TCHAR*>( a_szName ) ) );

	TVector4 vecPosition;
	pGoal->GetPosition( vecPosition );
	*a_pOutPosition = vecPosition;

	return a_pOutPosition;
}

// Builds the transform with the given forward axis and position, a_pUnk1 is the up direction
// $Barnyard: FUNCTION 004a6f10
TINT ACowTippingMiniGameState::BuildTransform( TFLOAT* a_pUnk1, TINT a_iUnk2, TFLOAT* a_pUnk3, TFLOAT* a_pUnk4 )
{
	const TVector4& rcUp       = *TREINTERPRETCAST( TVector4*, a_pUnk1 );
	const TVector4& rcForward  = *TREINTERPRETCAST( TVector4*, a_iUnk2 );
	const TVector4& rcPosition = *TREINTERPRETCAST( TVector4*, a_pUnk3 );
	TMatrix44&      rOutMatrix = *TREINTERPRETCAST( TMatrix44*, a_pUnk4 );

	rOutMatrix = TMatrix44::IDENTITY;

	TVector4 vecRight;
	vecRight.CrossProduct( rcForward, rcUp );
	vecRight.Normalise();

	TVector4 vecUp;
	vecUp.CrossProduct( vecRight, rcForward );

	rOutMatrix.AsBasisVector3( BASISVECTOR_RIGHT ) = vecRight.AsVector3();
	rOutMatrix.AsBasisVector3( BASISVECTOR_UP ).Set( -vecUp.x, -vecUp.y, -vecUp.z );
	rOutMatrix.AsBasisVector3( BASISVECTOR_FORWARD )     = rcForward.AsVector3();
	rOutMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = rcPosition;

	return 0;
}

// Returns the number of pickups that are active
// $Barnyard: FUNCTION 004a7080
TINT ACowTippingMiniGameState::CountActivePickups()
{
	TINT iCount = 0;

	for ( TINT i = NUM_PICKUP_GROUPS - 1; i >= 0; i-- )
	{
		for ( TINT k = NUM_PICKUPS_PER_GROUP - 1; k >= 0; k-- )
		{
			if ( m_aCrates[ i ][ k ].iState != Crate::CRATESTATE_FREE )
				iCount++;
		}
	}

	return iCount;
}

// Returns the group of the pickup with the object a_iUnk1 (-1 if none)
// $Barnyard: FUNCTION 004a7120
TINT ACowTippingMiniGameState::GetPickupGroup( TINT a_iUnk1 )
{
	for ( TINT i = 0; i < NUM_PICKUP_GROUPS; i++ )
	{
		for ( TINT k = NUM_PICKUPS_PER_GROUP - 1; k >= 0; k-- )
		{
			if ( TREINTERPRETCAST( TINT, m_aCrates[ i ][ k ].pObject ) == a_iUnk1 )
				return i;
		}
	}

	return -1;
}

// Adds a_iUnk2 to the lives of the player a_iUnk1 (PlayerData*)
// $Barnyard: FUNCTION 004a7870
TINT ACowTippingMiniGameState::AddLives2( TINT a_iUnk1, TINT a_iUnk2 )
{
	if ( m_eTurnEnd == TURNEND_NONE )
	{
		PlayerData* pPlayer = TREINTERPRETCAST( PlayerData*, a_iUnk1 );
		TINT        iCount  = a_iUnk2;

		pPlayer->iLives2 += a_iUnk2;

		if ( pPlayer->iLives2 < 0 )
		{
			pPlayer->iLives2 = 0;
			m_oLives2Hud.AddLives( m_aStages[ m_iStage ].iMaxLives );
		}
		else
		{
			if ( pPlayer->iLives2 > m_aStages[ m_iStage ].iMaxLives2 )
			{
				iCount           = m_aStages[ m_iStage ].iMaxLives2 - pPlayer->iLives;
				pPlayer->iLives2 = m_aStages[ m_iStage ].iMaxLives2;
			}

			m_oLives2Hud.AddLives( iCount );
		}
	}

	return 0;
}

// Picks the side of a_pUnk2 (cow) closer to (a_iUnk3, a_iUnk4, a_iUnk5, a_iUnk6) (floats).
// a_pUnk1 gets that position, a_pUnk7 the direction to face there, a_pUnk8 the position on the other side.
// $Barnyard: FUNCTION 004a9c20
void* ACowTippingMiniGameState::GetCowSidePosition( void* a_pUnk1, void* a_pUnk2, TINT a_iUnk3, TINT a_iUnk4, TINT a_iUnk5, TINT a_iUnk6, void* a_pUnk7, void* a_pUnk8 )
{
	TVector4* pOutPosition = TSTATICCAST( TVector4, a_pUnk1 );

	if ( a_pUnk2 )
	{
		TVector4 vecTarget( CT_AS_FLOAT( a_iUnk3 ), CT_AS_FLOAT( a_iUnk4 ), CT_AS_FLOAT( a_iUnk5 ), CT_AS_FLOAT( a_iUnk6 ) );

		TVector4 vecPositionA;
		TVector4 vecPositionB;
		TVector4 vecDirectionA;
		TVector4 vecDirectionB;
		GetCowHitPoints( TREINTERPRETCAST( ACowTippingMiniGameState::Cow*, a_pUnk2 ), &vecPositionA, &vecPositionB, &vecDirectionA, &vecDirectionB );

		TFLOAT fDistanceA = TVector4::DistanceXZ( vecPositionA, vecTarget );

		if ( TVector4::DistanceXZ( vecPositionB, vecTarget ) <= fDistanceA )
		{
			*TSTATICCAST( TVector4, a_pUnk7 ) = vecDirectionB;
			*TSTATICCAST( TVector4, a_pUnk8 ) = vecPositionA;
			*pOutPosition                     = vecPositionB;
		}
		else
		{
			*TSTATICCAST( TVector4, a_pUnk7 ) = vecDirectionA;
			*TSTATICCAST( TVector4, a_pUnk8 ) = vecPositionB;
			*pOutPosition                     = vecPositionA;
		}
	}
	else
	{
		*pOutPosition = TVector4::VEC_ZERO;
	}

	return pOutPosition;
}

// Resets the power-up timers of the player a_iUnk1, except the one of the power-up a_iUnk2 (index 3 is never reset)
// $Barnyard: FUNCTION 004aa830
TINT ACowTippingMiniGameState::ResetOtherPowerUpTimers( TINT a_iUnk1, TINT a_iUnk2 )
{
	PlayerData& rPlayer = m_aPlayers[ a_iUnk1 ];

	if ( a_iUnk2 != 0 )
	{
		rPlayer.aPowerUps[ 0 ].fTime    = 0.0f;
		rPlayer.aPowerUps[ 0 ].iCharges = 0;
	}

	if ( a_iUnk2 != 1 )
	{
		rPlayer.aPowerUps[ 1 ].fTime    = 0.0f;
		rPlayer.aPowerUps[ 1 ].iCharges = 0;
	}

	if ( a_iUnk2 != 2 )
	{
		rPlayer.aPowerUps[ 2 ].fTime    = 0.0f;
		rPlayer.aPowerUps[ 2 ].iCharges = 0;
	}

	if ( a_iUnk2 != 4 )
	{
		rPlayer.aPowerUps[ 4 ].fTime    = 0.0f;
		rPlayer.aPowerUps[ 4 ].iCharges = 0;
	}

	if ( a_iUnk2 != 5 )
	{
		rPlayer.aPowerUps[ 5 ].fTime    = 0.0f;
		rPlayer.aPowerUps[ 5 ].iCharges = 0;
	}

	return a_iUnk1;
}

// T2Vector<WeightedChoice::Entry, 30>::Begin
// $Barnyard: FUNCTION 004ab160
void* ACowTippingMiniGameState::WeightedChoice_Begin( void* a_pThis, void* a_pUnk1 )
{
	TINT* pIterator = TSTATICCAST( TINT, a_pUnk1 );
	pIterator[ 0 ]  = 0;
	pIterator[ 1 ]  = TREINTERPRETCAST( TINT, a_pThis );

	return a_pUnk1;
}

// Makes the enemy (a_iUnk1) stand still and wait (howl/growl), a_iUnk2 makes the timer depend on the stage
// $Barnyard: FUNCTION 004ab890
TBOOL ACowTippingMiniGameState::StartWaiting( TINT a_iUnk1, TINT a_iUnk2 )
{
	Enemy*         pEnemy = TREINTERPRETCAST( Enemy*, a_iUnk1 );
	AAnimatedUnit* pUnit  = pEnemy->pUnit;

	StopUnitSteer( pUnit );

	if ( pEnemy->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		pEnemy->fWaitTime = -ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( m_fCoyoteHowlTimeMin, m_fCoyoteHowlTimeMax );
		return pUnit->GetAnimatableObject()->PlayAnimation( TPS8( howl ) );
	}

	TFLOAT fScale = 1.0f;

	if ( TBOOL( a_iUnk2 ) )
	{
		TINT iStageType = CT_STAGE_TYPE( this );

		if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
		{
			fScale = m_fLargeStageTimeScale;
			pUnit->GetAnimatableObject()->PlayAnimation( TPS8( howl ) );
		}
	}

	pEnemy->fWaitTime = -( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( m_fEnemyHowlTimeMin, m_fEnemyHowlTimeMax ) * fScale );
	return TFALSE;
}

// Returns the position the enemy a_pUnk2 runs away to
// $Barnyard: FUNCTION 004ac5c0
void* ACowTippingMiniGameState::GetEnemyExitPosition( Toshi::TVector4* a_pUnk1, void* a_pUnk2 )
{
	Enemy* pEnemy = TSTATICCAST( Enemy, a_pUnk2 );

	if ( pEnemy->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		const TVector4* pPosition = pEnemy->pUnit->GetSteer()->GetPosition();
		GetClosestGoalPosition( a_pUnk1, pEnemy->iCoyoteGoal, const_cast<TCHAR*>( "coyote" ), pPosition );
	}
	else
	{
		const TVector4* pPosition = pEnemy->pUnit->GetSteer()->GetPosition();
		AInteriorGoal*  pGoal     = CT_NETWORK( this )->FindClosestGoalContaining( "start", *pPosition );

		TVector4 vecPosition;
		pGoal->GetPosition( vecPosition );
		*a_pUnk1 = vecPosition;
	}

	return a_pUnk1;
}

// Picks a random entry of the weighted list a_pThis (WeightedChoice), returns a pointer to the entry or TNULL
// $Barnyard: FUNCTION 004ad3d0
TINT ACowTippingMiniGameState::PickWeighted( void* a_pThis )
{
	struct Entry
	{
		TINT iValue;
		TINT iWeight;
	};

	struct WeightedChoice
	{
		TRandom* pRandom;
		TINT     iTotalWeight;
		TINT     iNumEntries;
		Entry    aEntries[ 1 ];
	};

	WeightedChoice* pChoice = TSTATICCAST( WeightedChoice, a_pThis );

	if ( pChoice->iNumEntries == 0 || ARandom::IsWeightZero( pChoice->iTotalWeight ) )
		return 0;

	TINT iRandom = ARandom::GetRandomWeight( pChoice->pRandom, pChoice->iTotalWeight );
	TINT iWeight = 0;

	for ( TINT i = 0; i < pChoice->iNumEntries; i++ )
	{
		iWeight += pChoice->aEntries[ i ].iWeight;

		if ( iWeight > iRandom )
			return TREINTERPRETCAST( TINT, &pChoice->aEntries[ i ] );
	}

	return 0;
}

// The farmer (m_pOtis) turns towards a_iUnk1 (TVector4*) in the stages of type 2 and 3
// $Barnyard: FUNCTION 004ae260
void ACowTippingMiniGameState::TurnFarmerTowards( TINT a_iUnk1 )
{
	TINT iStageType = CT_STAGE_TYPE( this );

	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
	{
		AAnimatedUnit*  pFarmer   = TREINTERPRETCAST( AAnimatedUnit*, m_pOtis );
		const TVector4& rcTarget  = *TREINTERPRETCAST( TVector4*, a_iUnk1 );
		TVector4&       rUp       = m_vFarmerFacing;
		TVector4&       rToTarget = m_vFarmerToTarget;

		pFarmer->GetAnimatableObject()->GetModelInstance()->GetUpVector( rUp );
		rUp.x = -rUp.x;
		rUp.y = -rUp.y;
		rUp.z = -rUp.z;

		TVector4 vecPosition;
		pFarmer->GetAnimatableObject()->GetModelInstance()->GetPosition( vecPosition );

		rToTarget.x = rcTarget.x - vecPosition.x;
		rToTarget.y = 0.0f;
		rToTarget.z = rcTarget.z - vecPosition.z;
		rToTarget.w = rcTarget.w;
		rToTarget.Normalise();

		m_fFarmerTurnProgress = 0.0f;

		if ( TVector4::DotProduct3( rToTarget, m_vUnk_0139C ) >= m_fFarmerFacingMinDot )
			m_bShooterTurning = TTRUE;
	}
}

// Hides the pickup a_pUnk1
// $Barnyard: FUNCTION 004ae720
TINT ACowTippingMiniGameState::HidePickup( void* a_pUnk1 )
{
	Crate*             pPickup = TSTATICCAST( Crate, a_pUnk1 );
	AAnimatableObject* pObject = pPickup->pObject;

	if ( pObject->GetParentObject() )
		AAnimatableObjectManager::GetSingleton()->DetachObject( pObject, pObject->GetParentObject() );

	pPickup->iState = Crate::CRATESTATE_FREE;
	pObject->SetSkeletonUpdating( TFALSE, TTRUE );
	pObject->SetVisible( TFALSE );

	if ( pPickup->iBit != -1 )
		ClearBit( m_auiPickupBits, pPickup->iBit );

	pPickup->pObject->GetModelInstance()->SetOpacity( 1.0f );
	pPickup->pObject->SetAnimControllersDisabled( TTRUE );

	TINT iNumActive = CountActivePickups();

	if ( iNumActive == 0 )
		m_oRunnerIcon.SetShown( TFALSE );

	return iNumActive;
}

// The enemy a_pUnk1 leaves the field (reached its exit), frees its cow
// $Barnyard: FUNCTION 004ae9f0
void* ACowTippingMiniGameState::RemoveEnemy( void* a_pUnk1 )
{
	Enemy*             pEnemy     = TSTATICCAST( Enemy, a_pUnk1 );
	AAnimatableObject* pAnim      = CT_ENEMY_ANIM( pEnemy );
	TINT               iStageType = CT_STAGE_TYPE( this );
	TUINT              uiBit;

	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		uiBit = pEnemy->iCow;
	else
		uiBit = pEnemy->iCowSide + 2 * pEnemy->iCow;

	// Drop what it carries
	AAnimatableObject* pAttached = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pAnim, m_iCarryBone, 0 );

	if ( pAttached )
	{
		AAnimatableObjectManager::GetSingleton()->DetachObject( pAttached, CT_ENEMY_ANIM( pEnemy ) );

		Cow* pCow = TNULL;

		for ( TINT i = 0; i < m_iNumCows; i++ )
		{
			if ( CT_COWS( this )[ i ].pObject == pAttached )
			{
				pCow = &CT_COWS( this )[ i ];
				break;
			}
		}

		pAttached->SetSkeletonUpdating( TFALSE, TTRUE );
		pAttached->GetModelInstance()->SetTransform( pCow->matStart );
	}

	AInteriorNetwork*       pNetwork     = CT_NETWORK( this );
	AKinematicsObstruction* pObstruction = CT_OBSTRUCTION( pEnemy->pUnit );

	if ( CT_ENEMY_ANIM( pEnemy )->GetParentObject() && pNetwork->m_vecObstructions.Find( pObstruction ) != pNetwork->m_vecObstructions.End() )
		pNetwork->m_vecObstructions.FindAndErase( pObstruction );

	pEnemy->pUnit->SetVisible( TFALSE );

	if ( pNetwork->m_vecObstructions.Find( pObstruction ) == pNetwork->m_vecObstructions.End() )
		pNetwork->m_vecObstructions.PushBack( pObstruction );

	CT_ENEMY_ANIM( pEnemy )->SetAnimControllersDisabled( TTRUE );
	pEnemy->pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
	pEnemy->pUnit->SetHashed( TFALSE );
	pEnemy->pUnit->SetNoQuery( TTRUE );
	pEnemy->iHits = 0;

	if ( pEnemy->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		// The coyote is respawned by SpawnCoyote
		T2GenericSList::Node* pNode = TREINTERPRETCAST( T2GenericSList::Node*, pEnemy );
		CT_ENEMY_LIST( this ).Erase( pNode, pNode->Next() );
	}
	else
	{
		CT_ENEMY_LIST( this ).Transfer( TREINTERPRETCAST( T2GenericSList::Node*, pEnemy ), CT_DONE_LIST( this ) );
		pEnemy->bUnk4D = TFALSE;

		if ( !pEnemy->bUnk4F && uiBit <= 29 )
		{
			ClearBit( m_auiCowSideReservedBits, uiBit );
			ClearBit( m_auiCowSideTakenBits, uiBit );
		}
	}

	// NOTE: unnamed flag of AUnit (0x200)
	pEnemy->pUnit->RemoveFlags( AUnit::FLAGS_MILK_TARGET );
	pEnemy->iCow     = -1;
	pEnemy->iCowSide = -1;
	pEnemy->eState   = Enemy::ENEMYSTATE_INACTIVE;

	// Projectiles don't target the unit anymore
	for ( TINT i = 0; i < 2; i++ )
	{
		T2GenericSList& rList = *TREINTERPRETCAST( T2GenericSList*, &m_aProjectileLists[ i ].llHeld );

		for ( T2GenericSList::Node* pNode = rList.Begin(); pNode != rList.End(); pNode = pNode->Next() )
		{
			Projectile* pProjectile = TREINTERPRETCAST( Projectile*, pNode );

			if ( pProjectile->pTarget == pEnemy->pUnit )
				pProjectile->pTarget = 0;
		}
	}

	return TNULL;
}

// Returns the cow closest to a_pUnk1 (TNULL if none is closer than a_fUnk2)
// $Barnyard: FUNCTION 004aee00
void* ACowTippingMiniGameState::FindClosestCow( const Toshi::TVector4* a_pUnk1, TFLOAT a_fUnk2 )
{
	Cow*   pClosest     = TNULL;
	TFLOAT fMinDistance = a_fUnk2;

	for ( TINT i = 0; i < m_iNumCows; i++ )
	{
		Cow*     pCow       = &CT_COWS( this )[ i ];
		TINT     iStageType = CT_STAGE_TYPE( this );
		TVector4 vecCowPosition;

		if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
			vecCowPosition = pCow->vTipSpot1;
		else
			pCow->pObject->GetModelInstance()->GetPosition( vecCowPosition );

		TFLOAT fDistance = TVector4::DistanceXZ( *a_pUnk1, vecCowPosition );

		if ( fDistance < fMinDistance )
		{
			fMinDistance = fDistance;
			pClosest     = pCow;
		}
	}

	return pClosest;
}

// Removes the pickup a_iUnk1 with a puff
// $Barnyard: FUNCTION 004af6e0
TINT ACowTippingMiniGameState::PuffPickup( TINT a_iUnk1 )
{
	Crate* pPickup = TREINTERPRETCAST( Crate*, a_iUnk1 );

	TVector4 vecPosition;
	pPickup->pObject->GetModelInstance()->GetPosition( vecPosition );

	AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, vecPosition, TVector4::VEC_NEGY );
	return HidePickup( pPickup );
}

// Sets the walking speed and the wait timer of the enemy a_pUnk1 (if it's flagged)
// $Barnyard: FUNCTION 004b0470
TINT ACowTippingMiniGameState::ResetEnemySpeed( void* a_pUnk1 )
{
	Enemy* pEnemy = TSTATICCAST( Enemy, a_pUnk1 );

	if ( pEnemy->iRunning == 0 )
		return 0;

	pEnemy->iRunning = 0;

	TRandom&                     rRandom   = ARandom::GetSingleton()->m_oRandom;
	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, pEnemy->pUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) ) );
	pMovement->SetWalkingSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, ( rRandom.GetFloatMinMax( 0.0f, m_fWalkSpeedRandom ) + m_fWalkSpeedBase ) * m_afUnitRunSpeed[ pEnemy->eType ] );

	TFLOAT fTimer;

	if ( pEnemy->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		fTimer = rRandom.GetFloatMinMax( m_fCoyoteWalkTimeMin, m_fCoyoteWalkTimeMax );
	}
	else
	{
		TINT   iStageType = CT_STAGE_TYPE( this );
		TFLOAT fScale     = 1.0f;

		if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
			fScale = m_fLargeStageTimeScale;

		fTimer = rRandom.GetFloatMinMax( m_fEnemyWalkTimeMin, m_fEnemyWalkTimeMax ) * fScale;
	}

	pEnemy->fWaitTime = fTimer;

	// Back to the default animation set
	ANamedAnimationSetRef pAnimationSet = CT_ENEMY_ANIM( pEnemy )->GetObjectType()->FindAnimationSetOrDefault( TPS8( default ) );

	if ( CT_ENEMY_ANIM( pEnemy )->GetToshiAnimInterface().GetAnimationSet() != pAnimationSet )
		CT_ENEMY_ANIM( pEnemy )->GetToshiAnimInterface().SetAnimationSet( pAnimationSet );

	return 0;
}

// Returns the position the enemy a_pUnk2 starts from
// $Barnyard: FUNCTION 004b0640
void* ACowTippingMiniGameState::GetEnemyStartPosition( Toshi::TVector4* a_pUnk1, void* a_pUnk2 )
{
	Enemy* pEnemy = TSTATICCAST( Enemy, a_pUnk2 );

	if ( pEnemy->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		const TVector4* pPosition = pEnemy->pUnit->GetSteer()->GetPosition();
		GetClosestGoalPosition( a_pUnk1, pEnemy->iCoyoteGoal, const_cast<TCHAR*>( "coyote" ), pPosition );
		return a_pUnk1;
	}

	TCHAR szGoalName[ 100 ];
	T2String8::Format( szGoalName, "start%d%c", pEnemy->iCow + 1, TCHAR( ( pEnemy->iCowSide != 0 ) + 'a' ) );

	AInteriorGoal* pGoal = CT_NETWORK( this )->FindRandomGoal( TPString8( szGoalName ) );

	TVector4 vecPosition;
	pGoal->GetPosition( vecPosition );
	*a_pUnk1 = vecPosition;

	return a_pUnk1;
}

// Position of the bone m_iLauncherCartridge1Bone of m_pLauncher
// $Barnyard: FUNCTION 004b1e50
void* ACowTippingMiniGameState::GetLauncherBonePosition1( void* a_pUnk1 )
{
	TMatrix44 matBone;
	m_pLauncher->GetModelInstance()->GetBoneTransform( m_iLauncherCartridge1Bone, matBone );

	*TSTATICCAST( TVector4, a_pUnk1 ) = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	return a_pUnk1;
}

// Position of the bone m_iLauncherCartridge2Bone of m_pLauncher
// $Barnyard: FUNCTION 004b1ef0
void* ACowTippingMiniGameState::GetLauncherBonePosition2( void* a_pUnk1 )
{
	TMatrix44 matBone;
	m_pLauncher->GetModelInstance()->GetBoneTransform( m_iLauncherCartridge2Bone, matBone );

	*TSTATICCAST( TVector4, a_pUnk1 ) = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	return a_pUnk1;
}

// Forward axis of the bone m_iLauncherCrateBone of m_pLauncher, turned around by 180 degrees around its right axis
// $Barnyard: FUNCTION 004b1f90
void* ACowTippingMiniGameState::GetLauncherBoneDirection( void* a_pUnk1 )
{
	TMatrix44 matBone;
	m_pLauncher->GetModelInstance()->GetBoneTransform( m_iLauncherCrateBone, matBone );

	// NOTE: retail leaves w uninitialised
	TVector4 vecForward = matBone.AsBasisVector4( BASISVECTOR_FORWARD );
	TVector4 vecRight   = matBone.AsBasisVector4( BASISVECTOR_RIGHT );
	vecForward.RotateAroundAxis( vecRight, TMath::PI );

	*TSTATICCAST( TVector4, a_pUnk1 ) = vecForward;
	return a_pUnk1;
}

// Moves the pickup a_pUnk1 (Crate*) towards the bone m_iLauncherCartridge1Bone (in an arc). Returns TTRUE once it's there.
// $Barnyard: FUNCTION 004b2080
TBOOL ACowTippingMiniGameState::MovePickupToLauncher( TFLOAT* a_pUnk1, TFLOAT a_fUnk2 )
{
	Crate* pPickup = TREINTERPRETCAST( Crate*, a_pUnk1 );

	TVector4 vecTargetDirection;
	GetLauncherBoneDirection( &vecTargetDirection );

	TVector4 vecTarget;
	GetLauncherBonePosition1( &vecTarget );

	TVector4 vecPosition;
	GetCratePos( &vecPosition, TREINTERPRETCAST( TINT, pPickup ) );

	TVector4 vecToTarget( vecTarget.x - vecPosition.x, vecTarget.y - vecPosition.y, vecTarget.z - vecPosition.z, vecTarget.w );
	vecToTarget.y -= vecToTarget.AsVector3().Magnitude() * m_fPickupLift;

	TFLOAT fDistance = vecToTarget.NormaliseMag();
	TFLOAT fStep     = a_fUnk2 * m_fPickupMoveSpeed;

	TVector4 vecUp;
	pPickup->pObject->GetModelInstance()->GetUpVector( vecUp );

	if ( fDistance - 0.3f > fStep )
	{
		TVector4 vecMove;
		vecMove.Multiply( vecToTarget, fStep );

		TVector4 vecNewPosition( vecMove.x + vecPosition.x, vecMove.y + vecPosition.y, vecMove.z + vecPosition.z, vecMove.w );
		pPickup->pObject->GetModelInstance()->SetTranslation( vecNewPosition.AsVector3() );

		if ( fDistance - fStep <= m_fPickupArriveDistance )
		{
			// Close enough, turn to the target direction
			TQuaternion quatFrom;
			quatFrom.SetVectorDirection( pPickup->vDirection.AsVector3(), TVector3::VEC_POSZ );

			TQuaternion quatTo;
			quatTo.SetVectorDirection( vecTargetDirection.AsVector3(), TVector3::VEC_POSZ );

			pPickup->fTurnProgress = a_fUnk2 * m_fPickupTurnSpeed + pPickup->fTurnProgress;
			TMath::Clip( pPickup->fTurnProgress, 0.0f, 1.0f );

			TQuaternion::Slerp( quatFrom, quatFrom, quatTo, pPickup->fTurnProgress );
			RotateVector( vecUp, quatFrom, TVector4::VEC_POSZ );

			pPickup->pObject->GetModelInstance()->SetDirection( vecUp, TVector4::VEC_POSZ );
		}
		else
		{
			// Spin while flying
			vecUp.AsVector3().RotateX( -( a_fUnk2 * m_fPickupTiltSpeed ) );

			pPickup->pObject->GetModelInstance()->SetDirection( vecUp, TVector4::VEC_POSZ );
			pPickup->vDirection    = vecUp;
			pPickup->fTurnProgress = 0.0f;
		}
	}
	else
	{
		pPickup->pObject->GetModelInstance()->SetTranslation( vecTarget.AsVector3() );

		TQuaternion quatFrom;
		quatFrom.SetVectorDirection( pPickup->vDirection.AsVector3(), TVector3::VEC_POSZ );

		TQuaternion quatTo;
		quatTo.SetVectorDirection( vecTargetDirection.AsVector3(), TVector3::VEC_POSZ );

		pPickup->fTurnProgress = a_fUnk2 * m_fPickupTurnSpeed + pPickup->fTurnProgress;
		TMath::Clip( pPickup->fTurnProgress, 0.0f, 1.0f );

		TQuaternion::Slerp( quatFrom, quatFrom, quatTo, pPickup->fTurnProgress );
		RotateVector( vecUp, quatFrom, TVector4::VEC_POSZ );

		pPickup->pObject->GetModelInstance()->SetDirection( vecUp, TVector4::VEC_POSZ );

		TVector4 vecDifference( vecUp.x - vecTargetDirection.x, vecUp.y - vecTargetDirection.y, vecUp.z - vecTargetDirection.z, vecUp.w );

		if ( vecDifference.MagnitudeSq() < 0.5f )
			return TTRUE;
	}

	return TFALSE;
}

// Moves the pickup a_iUnk1 (Crate*) towards the bone m_iLauncherCartridge2Bone. Returns TTRUE once it's there and turned.
// $Barnyard: FUNCTION 004b25a0
TBOOL ACowTippingMiniGameState::MovePickupToBone2( TINT a_iUnk1, TFLOAT a_fUnk2 )
{
	Crate* pPickup = TREINTERPRETCAST( Crate*, a_iUnk1 );

	TVector4 vecTargetDirection;
	GetLauncherBoneDirection( &vecTargetDirection );

	TVector4 vecTarget;
	GetLauncherBonePosition2( &vecTarget );

	TVector4 vecPosition;
	GetCratePos( &vecPosition, a_iUnk1 );

	TVector4 vecToTarget( vecTarget.x - vecPosition.x, vecTarget.y - vecPosition.y, vecTarget.z - vecPosition.z, vecTarget.w );
	TFLOAT   fDistance = vecToTarget.NormaliseMag();
	TFLOAT   fStep     = a_fUnk2 * m_fPickupMoveSpeed2;

	TVector4 vecUp;
	pPickup->pObject->GetModelInstance()->GetUpVector( vecUp );

	TQuaternion quatFrom;
	quatFrom.SetVectorDirection( pPickup->vDirection.AsVector3(), TVector3::VEC_POSZ );

	TQuaternion quatTo;
	quatTo.SetVectorDirection( vecTargetDirection.AsVector3(), TVector3::VEC_POSZ );

	pPickup->fTurnProgress = a_fUnk2 * m_fPickupTurnSpeed + pPickup->fTurnProgress;
	TMath::Clip( pPickup->fTurnProgress, 0.0f, 1.0f );

	TQuaternion::Slerp( quatFrom, quatFrom, quatTo, pPickup->fTurnProgress );
	RotateVector( vecUp, quatFrom, TVector4::VEC_POSZ );

	if ( fStep <= fDistance )
	{
		TVector4 vecMove;
		vecMove.Multiply( vecToTarget, fStep );

		TVector4 vecNewPosition( vecMove.x + vecPosition.x, vecMove.y + vecPosition.y, vecMove.z + vecPosition.z, vecMove.w );
		vecNewPosition.y = vecToTarget.y * fStep + vecNewPosition.y;

		pPickup->pObject->GetModelInstance()->SetTranslation( vecNewPosition.AsVector3() );
	}
	else
	{
		pPickup->pObject->GetModelInstance()->SetTranslation( vecTarget.AsVector3() );

		TVector4 vecDifference( vecUp.x - vecTargetDirection.x, vecUp.y - vecTargetDirection.y, vecUp.z - vecTargetDirection.z, vecUp.w );

		if ( vecDifference.MagnitudeSq() < 0.01f )
			return TTRUE;
	}

	return TFALSE;
}

// The cow a_iUnk3 got tipped by the enemy a_iUnk1 (helped by a_iUnk2, can be TNULL)
// $Barnyard: FUNCTION 004b4df0
void ACowTippingMiniGameState::OnCowTipped( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 )
{
	Enemy* pEnemy  = TREINTERPRETCAST( Enemy*, a_iUnk1 );
	Enemy* pHelper = TREINTERPRETCAST( Enemy*, a_iUnk2 );
	Cow*   pCow    = TREINTERPRETCAST( Cow*, a_iUnk3 );

	pCow->pObject->PlayAnimation( TPS8( tip_1rac_end ) );
	pCow->fTipProgress = 0.0f;
	m_bUnk_13409       = TTRUE;

	CT_ENEMY_ANIM( pEnemy )->PlayAnimation( TPS8( push_end ) );
	pEnemy->eState = Enemy::ENEMYSTATE_PUSH_END;

	if ( pHelper )
	{
		if ( pHelper->eState == Enemy::ENEMYSTATE_PUSHING )
		{
			CT_ENEMY_ANIM( pHelper )->PlayAnimation( TPS8( push_end ) );
			pHelper->eState = Enemy::ENEMYSTATE_PUSH_END;
		}
		else
		{
			SendUnitBackToStart( a_iUnk2 );
		}
	}

	AwardCowPoints( TREINTERPRETCAST( ACowTippingMiniGameState::Cow*, a_iUnk3 ), -m_iCowTippedPenalty );
	AddLives2( TREINTERPRETCAST( TINT, &m_aPlayers[ m_iCurrentPlayer ] ), -1 );

	TINT iStageType = CT_STAGE_TYPE( this );

	if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
	{
		AAnimatedUnit* pFarmer = TREINTERPRETCAST( AAnimatedUnit*, m_pOtis );

		if ( pFarmer->GetAnimatableObject()->IsAnimationPlanned( TPS8( gasp ) ) || pFarmer->GetAnimatableObject()->IsAnimPlaying( TPS8( gasp ), 0.0f ) )
			pFarmer->GetAnimatableObject()->FinishAnimation( TPS8( gasp ) );
		else
			ASoundManager::GetSingleton()->PlayCue( soundbank::DAGBOSS_OTIS_RESCUE2 );

		pFarmer->GetAnimatableObject()->PlayAnimation( TPS8( gasp ) );

		TVector4 vecPosition;
		TurnFarmerTowards( TREINTERPRETCAST( TINT, GetUnitTargetPos( &vecPosition, a_iUnk1 ) ) );
	}
	else if ( !m_oLives2Hud.IsPlayingFrameAnimation( TREINTERPRETCAST( T2GUITextureSection**, m_ppCowSpinSprites ) ) )
	{
		m_oCowShockMarks.SetShown( TFALSE );
		m_oLives2Hud.SetFrameAnimations(
		    TREINTERPRETCAST( T2GUITextureSection**, m_ppCowSpinSprites ),
		    TREINTERPRETCAST( T2GUITextureSection**, m_ppCowFallSprites ),
		    m_iNumCowSpinSprites,
		    m_iNumCowFallSprites,
		    1.0f,
		    5.0f
		);
	}
}

// Gives the player the power-up a_iUnk1 (5: extra life, a random one if the lives are full)
// $Barnyard: FUNCTION 004b4fb0
TBOOL ACowTippingMiniGameState::GivePowerUp( TINT a_iUnk1 )
{
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );

	TINT iPowerUp = a_iUnk1;

	if ( a_iUnk1 == 5 )
	{
		TINT iLives = m_aPlayers[ m_iCurrentPlayer ].iLives;

		if ( iLives < m_aStages[ m_iStage ].iMaxLives && iLives >= 0 )
		{
			TVector4 vecPosition;
			GetLauncherBonePosition1( &vecPosition );
			AddLifeAt( TREINTERPRETCAST( TINT, &m_aPlayers[ m_iCurrentPlayer ] ), TREINTERPRETCAST( TINT, &vecPosition ) );

			AGUI2TextBox* pTextBox = &m_aLivesTexts[ 1 ];
			TFLOAT        fScale   = pTextBox->GetScale();

			const TWCHAR* wszText = ALocaleManager::Interface()->GetString( 557 );
			m_aFlarePopups[ m_iNextFlarePopup ].Show( wszText, fScale, pTextBox, -1, -14336 );
			m_iNextFlarePopup = ( m_iNextFlarePopup + 1 ) % 3;

			m_oElementPopup.SetShownInstantly( TTRUE );
			return TFALSE;
		}

		iPowerUp = ARandom::GetSingleton()->m_oRandom.GetInt( 5 );
	}

	if ( iPowerUp != 3 )
		ResetOtherPowerUpTimers( m_iCurrentPlayer, iPowerUp );

	PowerUp& rPowerUp = m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ iPowerUp ];
	rPowerUp.fTime    = m_afPowerUpTimes[ iPowerUp ];
	rPowerUp.iCharges += m_aiPowerUpCharges[ iPowerUp ];

	if ( iPowerUp != 3 && iPowerUp != 5 )
	{
		TINT iUnk = m_aiPowerUpAmmo[ iPowerUp ];

		if ( iUnk == 0 )
			iUnk = m_iMaxAmmo;

		m_iMaxAmmo = iUnk;

		if ( iUnk != 0 && SetAmmoCount( 0, iUnk ) && m_iProjectileType < 0 )
			SelectAmmoList();
	}

	AGUI2TextBox* pTextBox = TREINTERPRETCAST( AGUI2TextBox*, TREINTERPRETCAST( TUINTPTR, &m_oAmmoCounter ) + 0x54 );
	const TWCHAR* wszText;

	switch ( iPowerUp )
	{
		case 0:
			wszText = ALocaleManager::Interface()->GetString( 573 );
			m_oAmmoCounter.SetMode( 2, m_iMaxAmmo );
			m_oAmmoCounter.SetText( wszText );
			break;
		case 1:
			wszText = ALocaleManager::Interface()->GetString( 574 );
			m_oAmmoCounter.SetMode( 1, m_iMaxAmmo );
			m_oAmmoCounter.SetText( wszText );
			break;
		case 2:
			wszText      = ALocaleManager::Interface()->GetString( 575 );
			m_bUnk_13408 = TFALSE;
			m_oAmmoCounter.SetMode( 3, 1 );
			m_oAmmoCounter.SetText( wszText );
			break;
		case 3:
			pTextBox = &m_aLivesTexts[ 0 ];
			wszText  = ALocaleManager::Interface()->GetString( 572 );
			break;
		case 4:
			wszText = ALocaleManager::Interface()->GetString( 576 );
			m_oAmmoCounter.SetMode( 0, m_iMaxAmmo );
			m_oAmmoCounter.SetText( wszText );
			break;
		default:
			return TFALSE;
	}

	if ( wszText && pTextBox )
	{
		TFLOAT fScale = pTextBox->GetScale();
		m_aFlarePopups[ m_iNextFlarePopup ].Show( wszText, fScale, pTextBox, -1, -14336 );
		m_iNextFlarePopup = ( m_iNextFlarePopup + 1 ) % 3;
	}

	return TFALSE;
}

// Picks a free cow (side) for the enemy a_iUnk1 (and outputs the side to a_pUnk2). Returns -1 if there's none.
// $Barnyard: FUNCTION 004b9120
TINT ACowTippingMiniGameState::PickFreeCow( TINT a_iUnk1, void* a_pUnk2 )
{
	struct WeightedChoice
	{
		TRandom* pRandom;
		TINT     iTotalWeight;
		TINT     iNumEntries;
		TINT     aEntries[ 30 ][ 2 ];
	};

	Enemy* pEnemy = TREINTERPRETCAST( Enemy*, a_iUnk1 );

	WeightedChoice oChoice;
	oChoice.pRandom      = &ARandom::GetSingleton()->m_oRandom;
	oChoice.iTotalWeight = 0;
	oChoice.iNumEntries  = 0;

	for ( TINT i = 0; i < 30; i++ )
	{
		oChoice.aEntries[ i ][ 0 ] = 0;
		oChoice.aEntries[ i ][ 1 ] = 0;
	}

	pEnemy->pUnit->GetSteer()->GetPosition();

	TINT iCurrentCow = pEnemy->iCow;
	TINT iStageType  = CT_STAGE_TYPE( this );
	TINT iNumFree    = 0;

	if ( iStageType != Stage::STAGETYPE_JUNKYARD_BOSS && iStageType != Stage::STAGETYPE_JUNKYARD )
	{
		if ( m_iNumTipSpots <= 0 )
			return -1;

		// Two sides per cow
		for ( TINT i = 0; i < m_iNumTipSpots; i++ )
		{
			if ( !IsBitSet( m_auiCowSideTakenBits, i ) && iCurrentCow != i / 2 )
			{
				oChoice.iTotalWeight++;
				oChoice.aEntries[ oChoice.iNumEntries ][ 0 ]   = i;
				oChoice.aEntries[ oChoice.iNumEntries++ ][ 1 ] = 1;
				iNumFree++;
			}
		}
	}
	else
	{
		if ( m_iNumTipSpots <= 0 )
			return -1;

		for ( TINT i = 0; i < m_iNumTipSpots; i++ )
		{
			if ( !IsBitSet( m_auiCowSideTakenBits, i ) && iCurrentCow != i )
			{
				oChoice.iTotalWeight++;
				oChoice.aEntries[ oChoice.iNumEntries ][ 0 ]   = i;
				oChoice.aEntries[ oChoice.iNumEntries++ ][ 1 ] = 1;
				iNumFree++;
			}
		}
	}

	if ( iNumFree <= 0 )
		return -1;

	TINT iResult = *TREINTERPRETCAST( TINT*, PickWeighted( &oChoice ) );

	if ( iStageType != Stage::STAGETYPE_JUNKYARD_BOSS && iStageType != Stage::STAGETYPE_JUNKYARD )
	{
		*TSTATICCAST( TINT, a_pUnk2 ) = iResult & 1;
		iResult /= 2;
	}

	return iResult;
}

// Brings the coyote (m_aEnemies[ 12 ]) back to the field
// $Barnyard: FUNCTION 004b9420
void ACowTippingMiniGameState::SpawnCoyote()
{
	Enemy* pCoyote = TREINTERPRETCAST( Enemy*, &m_aEnemies[ 12 ] );

	if ( m_eTurnEnd != TURNEND_NONE || pCoyote->eState != Enemy::ENEMYSTATE_INACTIVE )
		return;

	pCoyote->pUnit->ChangeSteer( &TGetClass( AInteriorSteer ) );
	AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pCoyote->pUnit->GetSteer() );

	pCoyote->eState = Enemy::ENEMYSTATE_ACTIVE;
	pCoyote->bUnk4C = !pCoyote->bUnk4C;

	// The coyote alternates between the two coyote goals
	TINT iGoal;

	if ( pCoyote->bUnk4C )
	{
		pCoyote->iCoyoteGoal = 1;
		iGoal                = 0;
	}
	else
	{
		pCoyote->iCoyoteGoal = 0;
		iGoal                = 1;
	}

	TString8 strGoalName;
	strGoalName.Format( "%s%d", "coyote", iGoal );
	TPString8 strGoal( strGoalName.GetString() );

	pSteer->SetInteracting( 0 );
	pSteer->SetNetwork( CT_NETWORK( this ), 0 );
	pSteer->Command_SnapToGoal( strGoal );

	pCoyote->pUnit->SetVisible( TTRUE );
	pCoyote->pUnit->SetHashed( TTRUE );
	pCoyote->pUnit->SetNoQuery( TFALSE );
	CT_ENEMY_ANIM( pCoyote )->SetAnimControllersDisabled( TFALSE );

	pCoyote->iHits                = 0;
	pCoyote->iProjectilesInFlight = 0;
	pCoyote->iCow                 = -1;
	pCoyote->iCowSide             = -1;
	pCoyote->bUnk4E               = TFALSE;

	TVector4 vecGoalPosition;
	GetGoalPosition( this, &vecGoalPosition, pCoyote->iCoyoteGoal, "coyote" );
	TINT iArea = CT_NETWORK( this )->FindContainingArea( vecGoalPosition );

	pSteer->Command_CancelCurrentCommand();
	pSteer->SeekPointInArea( vecGoalPosition, iArea );

	TRandom&                     rRandom   = ARandom::GetSingleton()->m_oRandom;
	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, pCoyote->pUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) ) );
	pMovement->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, ( rRandom.GetFloatMinMax( 0.0f, m_fRunSpeedRandom ) + 1.0f ) * m_fCoyoteRunSpeed + 1.0f );

	ANamedAnimationSetRef pAnimationSet = CT_ENEMY_ANIM( pCoyote )->GetObjectType()->FindAnimationSetOrDefault( TPS8( default ) );

	if ( CT_ENEMY_ANIM( pCoyote )->GetToshiAnimInterface().GetAnimationSet() != pAnimationSet )
		CT_ENEMY_ANIM( pCoyote )->GetToshiAnimInterface().SetAnimationSet( pAnimationSet );

	CT_ENEMY_ANIM( pCoyote )->KillAllAnimations();
	CT_ENEMY_ANIM( pCoyote )->PlayAnimation( TPS8( run_lp ) );

	pCoyote->fUnk24    = rRandom.GetFloatMinMax( m_fCoyoteWaitTimeMin, m_fCoyoteWaitTimeMax );
	pCoyote->fWaitTime = rRandom.GetFloatMinMax( m_fCoyoteWaitTimeMin, m_fCoyoteWaitTimeMax );

	CT_ENEMY_LIST( this ).PushBack( TREINTERPRETCAST( T2GenericSList::Node*, pCoyote ) );
}

// Coyote respawn timer
// $Barnyard: FUNCTION 004b9c90
void ACowTippingMiniGameState::UpdateCoyoteTimer( TFLOAT a_fUnk1 )
{
	TINT iStageType = CT_STAGE_TYPE( this );

	if ( iStageType != Stage::STAGETYPE_JUNKYARD_BOSS && iStageType != Stage::STAGETYPE_JUNKYARD )
	{
		m_fCoyoteTimer -= a_fUnk1;

		if ( m_fCoyoteTimer < 0.0f )
		{
			m_fCoyoteTimer = m_fCoyoteInterval;
			SpawnCoyote();
		}
	}
}

// Updates the pickups
// $Barnyard: FUNCTION 004b9ce0
TINT ACowTippingMiniGameState::UpdatePickups( TFLOAT a_fUnk1 )
{
	for ( TINT i = 0; i < NUM_PICKUP_GROUPS; i++ )
	{
		for ( TINT k = NUM_PICKUPS_PER_GROUP - 1; k >= 0; k-- )
		{
			Crate* pPickup = &m_aCrates[ i ][ k ];

			switch ( pPickup->iState )
			{
				case Crate::CRATESTATE_LOADING:
				{
					// Fading out while flying to the bone m_iLauncherCartridge2Bone
					TFLOAT fOpacity = pPickup->pObject->GetModelInstance()->GetOpacity();
					fOpacity        = fOpacity - a_fUnk1 * m_fFadeSpeed;

					if ( fOpacity >= 0.0f )
					{
						pPickup->pObject->GetModelInstance()->SetOpacity( fOpacity );
						MovePickupToBone2( TREINTERPRETCAST( TINT, pPickup ), a_fUnk1 );
					}
					else
					{
						HidePickup( pPickup );
					}

					break;
				}
				case Crate::CRATESTATE_FADING:
				{
					// Fading out
					TFLOAT fOpacity = pPickup->pObject->GetModelInstance()->GetOpacity();
					fOpacity        = fOpacity - a_fUnk1 * m_fFadeSpeed;

					if ( fOpacity >= 0.0f )
						pPickup->pObject->GetModelInstance()->SetOpacity( fOpacity );
					else
						PuffPickup( TREINTERPRETCAST( TINT, pPickup ) );

					break;
				}
				case Crate::CRATESTATE_CARRIED:
				{
					// Carried/thrown, lands once it hits the ground
					TVector4 vecPosition;
					pPickup->pObject->GetModelInstance()->GetPosition( vecPosition );

					TBOOL bMoved = TFALSE;

					if ( pPickup->vPosition.y != 0.0f && pPickup->vPosition.y < vecPosition.y )
					{
						AAnimatableObject* pParent = pPickup->pObject->GetParentObject();

						if ( !pParent || !pParent->IsAnimPlaying( TPS8( fire ), 0.0f ) || pParent->GetAnimationProgress( TPS8( fire ) ) > 0.5f )
						{
							if ( pPickup->pObject->GetParentObject() )
							{
								TSkeletonInstance* pSkeletonInstance = pPickup->pObject->GetParentObject()->GetModelInstance()->GetSkeletonInstance();
								pSkeletonInstance->UpdateTime( a_fUnk1 );
								pSkeletonInstance->UpdateState( TTRUE ); // NOTE: retail UpdateState has no frame check
								pPickup->pObject->UpdateAttachmentTransform();
							}

							TVector4 vecMove( vecPosition.x - pPickup->vPosition.x, vecPosition.y - pPickup->vPosition.y, vecPosition.z - pPickup->vPosition.z, vecPosition.w );
							TFLOAT   fMoveSq = vecMove.MagnitudeSq();

							if ( fMoveSq > 0.0001f )
							{
								bMoved = TTRUE;

								TFLOAT   fDistance = TMath::Sqrt( fMoveSq );
								TVector4 vecDirection;
								vecDirection.Divide( vecMove, fDistance );
								fDistance += 0.001f;

								TVector4 vecNormal;
								TFLOAT   fStepHeight;
								TFLOAT   fPenetration;

								if ( ATerrainInterface::GetSingleton()->CollideSweptSphere( pPickup->vPosition, 0.1f, vecDirection, fDistance, vecNormal, 0.0f, fStepHeight, fPenetration, 0 ) )
								{
									TVector4 vecGround = vecPosition + TVector4::VEC_POSY;
									vecGround.w        = vecPosition.w;

#ifdef BARNYARD_COMMUNITY_PATCH
									// NOTE: retail multiplies an uninitialised vector by the distance here, use the direction of the move
									TVector4 vecHit;
									vecHit.Multiply( vecDirection, fDistance );
#else
									TVector4 vecUninitialised;
									TVector4 vecHit;
									vecHit.Multiply( vecUninitialised, fDistance );
#endif
									vecPosition.Set( pPickup->vPosition.x + vecHit.x, pPickup->vPosition.y + vecHit.y, pPickup->vPosition.z + vecHit.z, pPickup->vPosition.w );

									TINT     iAlpha;
									TVector3 vecRotation;

									if ( ATerrainInterface::GetSingleton()->FindGroundPlacement( vecPosition, -0.1f, vecGround, iAlpha, vecRotation, pPickup->vNormal, 0.3f, 10.0f ) )
									{
										pPickup->pObject->GetModelInstance()->SetTranslation( vecGround.AsVector3() );
										ResetCrate( pPickup );
									}
								}
							}
						}
					}

					if ( !bMoved )
						pPickup->vPosition = vecPosition;

					break;
				}
				case Crate::CRATESTATE_FALLING:
				{
					// Falling down
					TVector4 vecPosition;
					pPickup->pObject->GetModelInstance()->GetPosition( vecPosition );

					pPickup->fFallTime += a_fUnk1;
					TFLOAT fFallDistance = pPickup->fFallTime * 4.2f;

					TVector4 vecNormal;
					TFLOAT   fStepHeight;
					TFLOAT   fPenetration;

					if ( ATerrainInterface::GetSingleton()->CollideSweptSphere( vecPosition, 0.1f, TVector4::VEC_POSY, fFallDistance, vecNormal, 0.0f, fStepHeight, fPenetration, 0 ) )
					{
						pPickup->iState = Crate::CRATESTATE_ON_GROUND;
						vecPosition.y   = ( fFallDistance - 1.5f ) + vecPosition.y;

						TVector4 vecGround( vecPosition.x + TVector4::VEC_POSY.x, vecPosition.y + TVector4::VEC_POSY.y, vecPosition.z + TVector4::VEC_POSY.z, vecPosition.w );

						TINT     iAlpha;
						TVector3 vecRotation;
						ATerrainInterface::GetSingleton()->FindGroundPlacement( vecPosition, -0.1f, vecGround, iAlpha, vecRotation, pPickup->vNormal, 0.3f, 10.0f );

						vecPosition = vecGround;
					}
					else
					{
						TVector4 vecMove;
						vecMove.Multiply( TVector4::VEC_POSY, fFallDistance );

						vecPosition.x = vecMove.x + vecPosition.x;
						vecPosition.y = vecMove.y + vecPosition.y;
						vecPosition.z = vecMove.z + vecPosition.z;
					}

					pPickup->pObject->GetModelInstance()->SetTranslation( vecPosition.AsVector3() );
					break;
				}
				case Crate::CRATESTATE_ON_GROUND:
				{
					// On the ground: align with the ground normal and blink before it disappears
					TMatrix44 matTransform;
					pPickup->pObject->GetModelInstance()->GetLocalTransform( matTransform );

					TVector4 vecForward = matTransform.AsBasisVector4( BASISVECTOR_FORWARD );

					if ( TVector4::DistanceSq( vecForward, pPickup->vNormal ) > 0.001f )
					{
						TVector4 vecToNormal( pPickup->vNormal.x - vecForward.x, pPickup->vNormal.y - vecForward.y, pPickup->vNormal.z - vecForward.z, pPickup->vNormal.w );
						TFLOAT   fDistance = vecToNormal.NormaliseMag();
						TFLOAT   fStep     = ( fDistance >= 0.1f ) ? 0.1f : fDistance;

						TVector4 vecMove;
						vecMove.Multiply( vecToNormal, fStep );

						vecForward.x = vecMove.x + vecForward.x;
						vecForward.y = vecMove.y + vecForward.y;
						vecForward.z = vecMove.z + vecForward.z;
						vecForward.Normalise();

						TVector4 vecUp       = matTransform.AsBasisVector4( BASISVECTOR_UP );
						TVector4 vecPosition = matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
						BuildTransform( &vecUp.x, TREINTERPRETCAST( TINT, &vecForward ), &vecPosition.x, TREINTERPRETCAST( TFLOAT*, &matTransform ) );

						pPickup->pObject->GetModelInstance()->SetTransform( matTransform );
					}

					pPickup->fTimeLeft -= a_fUnk1;
					TFLOAT fBlinkTime = m_fBounceTime - m_fBounceFallTime;

					if ( fBlinkTime > pPickup->fTimeLeft )
					{
						TFLOAT fBlinkProgress = fBlinkTime - pPickup->fTimeLeft;
						TFLOAT fSpeedFactor;

						if ( fBlinkProgress >= 0.0f )
						{
							TUINT uiStep = TUINT( fBlinkProgress / fBlinkTime * m_fBlinkSteps + 0.5f );
							fSpeedFactor = TFLOAT( uiStep ) / m_fBlinkSteps;
						}
						else
						{
							fSpeedFactor = 0.0f;
						}

						pPickup->fBlinkPhase = pPickup->fBlinkPhase - ( fSpeedFactor * ( m_fBlinkSpeedMax - m_fBlinkSpeedMin ) + m_fBlinkSpeedMin ) * a_fUnk1;

						TFLOAT fBlink = TMath::Cos( pPickup->fBlinkPhase * TMath::PI );
						TMath::Clip( fBlink, 0.0f, 1.0f );

						if ( fBlink <= m_fUnk_00D40 )
							pPickup->pObject->GetModelInstance()->SetOpacity( m_fPickupOpacityB );
						else
							pPickup->pObject->GetModelInstance()->SetOpacity( m_fPickupOpacityA );

						if ( pPickup->fTimeLeft < 0.0f )
							pPickup->iState = Crate::CRATESTATE_FADING;
					}

					break;
				}
				case Crate::CRATESTATE_COLLECTED:
					// Collected, flying to the bone m_iLauncherCartridge1Bone
					if ( MovePickupToLauncher( TREINTERPRETCAST( TFLOAT*, pPickup ), a_fUnk1 ) )
					{
						pPickup->iState = Crate::CRATESTATE_GIVEN;
						GivePowerUp( GetPickupGroup( TREINTERPRETCAST( TINT, pPickup->pObject ) ) );
					}

					break;
				case Crate::CRATESTATE_GIVEN:
					pPickup->iState = Crate::CRATESTATE_LOADING;
					break;
				default:
					break;
			}
		}
	}

	return 0;
}

// The enemy a_iUnk1 starts pushing the cow a_iUnk2
// $Barnyard: FUNCTION 004baad0
void ACowTippingMiniGameState::StartPushing( TINT a_iUnk1, TINT a_iUnk2 )
{
	Enemy* pEnemy = TREINTERPRETCAST( Enemy*, a_iUnk1 );
	Cow*   pCow   = TREINTERPRETCAST( Cow*, a_iUnk2 );

	if ( !pCow->pObject->IsAnimPlaying( TPS8( tip_1rac_loop ), 0.0f ) )
		pCow->pObject->PlayAnimation( TPS8( tip_1rac_start ) );

	CT_ENEMY_ANIM( pEnemy )->PlayAnimation( TPS8( push_start ) );
	pEnemy->eState = Enemy::ENEMYSTATE_PUSHING;

	AInteriorNetwork*       pNetwork     = CT_NETWORK( this );
	AKinematicsObstruction* pObstruction = CT_OBSTRUCTION( pEnemy->pUnit );

	if ( pNetwork->m_vecObstructions.Find( pObstruction ) != pNetwork->m_vecObstructions.End() )
		pNetwork->m_vecObstructions.FindAndErase( CT_OBSTRUCTION( pEnemy->pUnit ) );

	StopUnitSteer( pEnemy->pUnit );

	TINT iStageType = CT_STAGE_TYPE( this );

	if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
	{
		AAnimatableObjectManager::GetSingleton()->AttachObject( CT_ENEMY_ANIM( pEnemy ), pCow->pObject, 0, TINT8( m_iCowTipSpotBone ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

		AAnimatableObject* pFarmerObject = m_pHanna;

		if ( !pFarmerObject->IsAnimationPlanned( TPS8( distress_idle ) ) && !pFarmerObject->IsAnimPlaying( TPS8( distress_idle ), 0.0f ) )
			pFarmerObject->FinishAnimation( TPS8( distress_idle ) );

		pFarmerObject->PlayAnimation( TPS8( distress_idle ) );

		if ( !m_bShooterTurning )
		{
			AAnimatableObject* pFarmerAnim = TREINTERPRETCAST( AAnimatedUnit*, m_pOtis )->GetAnimatableObject();

			if ( !pFarmerAnim->IsAnimationPlanned( TPS8( distress_idle ) ) && !pFarmerAnim->IsAnimPlaying( TPS8( distress_idle ), 0.0f ) )
				pFarmerAnim->FinishAnimation( TPS8( distress_idle ) );

			pFarmerAnim->PlayAnimation( TPS8( distress_idle ) );

			TVector4 vecPosition;
			TurnFarmerTowards( TREINTERPRETCAST( TINT, GetUnitTargetPos( &vecPosition, a_iUnk1 ) ) );
		}
	}
}

// Position of the side of the cow a_pUnk2 the enemy a_iUnk3 goes to (see GetCowSidePosition)
// $Barnyard: FUNCTION 004bc270
void* ACowTippingMiniGameState::GetCowSideForEnemy( void* a_pUnk1, void* a_pUnk2, TINT a_iUnk3, TFLOAT* a_pUnk4, void* a_pUnk5 )
{
	TVector4 vecEnemyPosition;
	GetEnemyRetreatPosition( &vecEnemyPosition, TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, a_iUnk3 ), TREINTERPRETCAST( TVector4*, a_pUnk4 ) );

	TINT* pPosition = TREINTERPRETCAST( TINT*, &vecEnemyPosition );
	GetCowSidePosition( a_pUnk1, a_pUnk2, pPosition[ 0 ], pPosition[ 1 ], pPosition[ 2 ], pPosition[ 3 ], a_pUnk4, a_pUnk5 );

	return a_pUnk1;
}

// Same as GetCowSideForEnemy, a_pUnk5 gets the enemy at the other side of the cow (TNULL if none or a_pUnk3 itself)
// $Barnyard: FUNCTION 004bc2d0
void* ACowTippingMiniGameState::GetCowSideAndHelper( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, TFLOAT* a_pUnk4, void* a_pUnk5 )
{
	TVector4 vecOtherSide;
	TVector4 vecPosition;
	GetCowSideForEnemy( &vecPosition, a_pUnk2, TREINTERPRETCAST( TINT, a_pUnk3 ), a_pUnk4, &vecOtherSide );

	void* pOther                   = FindClosestEnemy( &vecOtherSide, m_fCowSearchRadius );
	*TSTATICCAST( void*, a_pUnk5 ) = pOther;

	if ( a_pUnk3 == pOther )
		*TSTATICCAST( void*, a_pUnk5 ) = TNULL;

	*TSTATICCAST( TVector4, a_pUnk1 ) = vecPosition;
	return a_pUnk1;
}

// Sends the enemy a_iUnk1 to a free cow (or home if there's none)
// $Barnyard: FUNCTION 004bfa20
void ACowTippingMiniGameState::SendEnemyToCow( TINT a_iUnk1 )
{
	Enemy* pEnemy = TREINTERPRETCAST( Enemy*, a_iUnk1 );

	if ( m_iCarryBone == -1 || !AAnimatableObjectManager::GetSingleton()->GetAttachedObject( CT_ENEMY_ANIM( pEnemy ), m_iCarryBone, 0 ) )
	{
		TINT iSide;
		TINT iCow = PickFreeCow( a_iUnk1, &iSide );

		// NOTE: retail treats the cow 0 as no cow
		if ( iCow <= 0 )
		{
			FlattenSteerDirection( pEnemy->pUnit->GetSteer() );
			SendUnitBackToStart( a_iUnk1 );
		}
		else
		{
			pEnemy->iCow     = iCow;
			pEnemy->iCowSide = iSide;
			MakeEnemyRetreat( TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, a_iUnk1 ) );
			ResetEnemySpeed( pEnemy );
		}

		AInteriorNetwork*       pNetwork     = CT_NETWORK( this );
		AKinematicsObstruction* pObstruction = CT_OBSTRUCTION( pEnemy->pUnit );

		if ( pNetwork->m_vecObstructions.Find( pObstruction ) == pNetwork->m_vecObstructions.End() )
			pNetwork->m_vecObstructions.PushBack( pObstruction );
	}
	else
	{
		FlattenSteerDirection( pEnemy->pUnit->GetSteer() );
		SendUnitBackToStart( a_iUnk1 );
	}
}

// Updates the enemies
// $Barnyard: FUNCTION 004bff10
TBOOL ACowTippingMiniGameState::UpdateEnemies( TFLOAT a_fUnk1 )
{
	T2GenericSList& rList = CT_ENEMY_LIST( this );

	for ( T2GenericSList::Node* pNode = rList.Begin(); pNode != rList.End(); )
	{
		Enemy* pEnemy = TREINTERPRETCAST( Enemy*, pNode );

		// The enemy can leave the list while it's updated
		pNode = pNode->Next();

		AInteriorSteer* pSteer      = TSTATICCAST( AInteriorSteer, pEnemy->pUnit->GetSteer() );
		TVector4        vecPosition = *pSteer->GetPosition();
		AAnimatedUnit*  pUnit       = pEnemy->pUnit;

		pEnemy->fUnk24 -= a_fUnk1;

		if ( pEnemy->eState == Enemy::ENEMYSTATE_TARGETED )
		{
			pEnemy->fStillTime -= a_fUnk1;

			if ( pEnemy->fStillTime > 0.0f )
				continue;

			pSteer->SetMode( AInteriorSteer::MODE_STILL );
		}

		if ( pEnemy->eType != Enemy::ENEMYTYPE_COYOTE )
		{
			if ( *pSteer->GetCurrentArea()->m_strName == TPS8( l ) && pEnemy->fWaitTime > 0.0f )
				MakeUnitRunAway( TREINTERPRETCAST( TINT, pEnemy ), 0.0f, 0.0f, 0.0f );

			if ( pEnemy->eType != Enemy::ENEMYTYPE_COYOTE && pEnemy->eState != Enemy::ENEMYSTATE_PUSHING && pEnemy->eState != Enemy::ENEMYSTATE_PUSH_END && !pSteer->IsFollowingPath() )
			{
				TVector4 vecTarget;
				TVector4 vecDirection;
				GetEnemyRetreatPosition( &vecTarget, TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pEnemy ), TREINTERPRETCAST( TVector4*, &vecDirection.x ) );

				Cow* pCow = TNULL;

				if ( !pEnemy->bUnk4F )
					pCow = TSTATICCAST( Cow, FindClosestCow( &vecPosition, m_fCowSearchRadius ) );

				if ( pEnemy->bUnk4F || !pCow || &CT_COWS( this )[ pEnemy->iCow ] != pCow )
					pSteer->SetMode( AInteriorSteer::MODE_STILL );
			}
		}

		switch ( pEnemy->eState )
		{
			case Enemy::ENEMYSTATE_FADING:
			{
				// Fading out
				TFLOAT fOpacity = CT_ENEMY_ANIM( pEnemy )->GetModelInstance()->GetOpacity();
				fOpacity        = fOpacity - a_fUnk1 * m_fFadeSpeed;

				if ( fOpacity >= 0.0f )
				{
					CT_ENEMY_ANIM( pEnemy )->GetModelInstance()->SetOpacity( fOpacity );
				}
				else
				{
					pEnemy->eState = Enemy::ENEMYSTATE_INACTIVE;
					pEnemy->pUnit->SetVisible( TFALSE );
					RemoveEnemy( pEnemy );

					CT_ENEMY_ANIM( pEnemy )->GetModelInstance()->SetOpacity( 1.0f );
					pEnemy->bUnk4C = !pEnemy->bUnk4C;
				}

				break;
			}
			case Enemy::ENEMYSTATE_PUSH_END:
			{
				// Finishing the push
				if ( CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( push_end ), 0.0f ) )
					break;

				if ( CT_ENEMY_ANIM( pEnemy )->IsAnimationPlanned( TPS8( push_end ) ) )
					break;

				TINT iStageType = CT_STAGE_TYPE( this );

				if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
				{
					StealUnitCarriedObject( TREINTERPRETCAST( TINT, pEnemy ) );
					SendUnitBackToStart( TREINTERPRETCAST( TINT, pEnemy ) );
				}
				else
				{
					TUINT uiBit = GetUnitStartBit( pEnemy, iStageType );
					SendEnemyToCow( TREINTERPRETCAST( TINT, pEnemy ) );

					if ( !pEnemy->bUnk4F && uiBit <= 29 )
						ClearBit( m_auiCowSideTakenBits, uiBit );
				}

				break;
			}
			case Enemy::ENEMYSTATE_PUSHING:
			{
				// Pushing the cow
				if ( !IsUnitNotYukking( TREINTERPRETCAST( TINT, pEnemy ) ) )
					break;

				TVector4 vecTarget;
				TVector4 vecTargetDirection;
				GetEnemyRetreatPosition( &vecTarget, TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pEnemy ), TREINTERPRETCAST( TVector4*, &vecTargetDirection.x ) );

				Cow* pTargetCow = TSTATICCAST( Cow, FindClosestCow( &vecTarget, m_fCowSearchRadius ) );
				Cow* pCow       = TSTATICCAST( Cow, FindClosestCow( &vecPosition, m_fCowSearchRadius ) );

				if ( !pCow || pTargetCow != pCow )
					break;

				// Move to the side of the cow and face it
				TVector4 vecSideDirection;
				void*    pHelper;
				TVector4 vecSidePosition;
				GetCowSideAndHelper( &vecSidePosition, pCow, pEnemy, &vecSideDirection.x, &pHelper );

				TVector4 vecSteerPosition  = *pSteer->GetPosition();
				TVector4 vecSteerDirection = *pSteer->GetDirection();

				TQuaternion quatFrom;
				quatFrom.SetVectorDirection( vecSteerDirection.AsVector3(), TVector3::VEC_POSZ );

				TQuaternion quatTo;
				quatTo.SetVectorDirection( vecSideDirection.AsVector3(), TVector3::VEC_POSZ );

				TQuaternion::Slerp( quatFrom, quatFrom, quatTo, a_fUnk1 * m_fCowSideTurnSpeed );

				TVector4 vecFacing;
				TQuaternion::RotateVector( vecFacing.AsVector3(), quatFrom, TVector3::VEC_POSZ );
				vecFacing.w = 1.0f;
				vecFacing.Normalise();

				TVector4 vecFacingXZ = vecFacing;
				vecFacingXZ.y        = 0.0f;
				vecFacingXZ.Normalise();

				pSteer->SetDirection( vecFacingXZ );
				CT_ENEMY_ANIM( pEnemy )->SetAnimControllersDisabled( TTRUE );

				TVector4 vecNewPosition = LerpClamped3( vecSteerPosition, vecSidePosition, a_fUnk1 * m_fCowSideMoveSpeed );
				pSteer->SetPosition( vecNewPosition );

				TMatrix44 matCow;
				pCow->pObject->GetModelInstance()->GetLocalTransform( matCow );
				TVector4 vecCowForward = matCow.AsBasisVector4( BASISVECTOR_FORWARD );

				TMatrix44 matEnemy;
				BuildTransform( &vecFacing.x, TREINTERPRETCAST( TINT, &vecCowForward ), &vecNewPosition.x, TREINTERPRETCAST( TFLOAT*, &matEnemy ) );
				pEnemy->pUnit->GetModelInstance()->SetTransform( matEnemy );

				if ( CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( push_lp ), 0.0f ) )
				{
					// Keep the push in sync with the cow
					if ( pCow->pObject->IsAnimPlaying( TPS8( tip_1rac_loop ), 0.0f ) )
					{
						TFLOAT fTime = pCow->pObject->GetAnimationTime( TPS8( tip_1rac_loop ) );

						if ( fTime != CT_ENEMY_ANIM( pEnemy )->GetAnimationTime( TPS8( push_lp ) ) )
							CT_ENEMY_ANIM( pEnemy )->SetAnimationTime( TPS8( push_lp ), fTime );
					}

					pCow->fTipProgress = a_fUnk1 * m_afUnitTipSpeed[ pEnemy->eType ] + pCow->fTipProgress;

					TVector4 vecUnused;
					void*    pOtherHelper;
					TVector4 vecOtherPosition;
					GetCowSideAndHelper( &vecOtherPosition, pCow, pEnemy, &vecUnused.x, &pOtherHelper );
					TVector4::DistanceXZ( vecOtherPosition, vecPosition );

					if ( pCow->fTipProgress <= m_fTipThreshold && m_eTurnEnd != TURNEND_OUT_OF_LIVES )
						break;

					OnCowTipped( TREINTERPRETCAST( TINT, pEnemy ), TREINTERPRETCAST( TINT, pOtherHelper ), TREINTERPRETCAST( TINT, pCow ) );
				}
				else
				{
					if ( CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( push_start ), 0.0f ) != TTRUE )
						break;

					if ( pCow->pObject->IsAnimationPlanned( TPS8( tip_1rac_end ) ) == TTRUE ||
					     pCow->pObject->IsAnimationPlanned( TPS8( spin_idle ) ) == TTRUE ||
					     pCow->pObject->IsAnimationPlanned( TPS8( spin ) ) == TTRUE ||
					     pCow->pObject->IsAnimationPlanned( TPS8( getup ) ) == TTRUE )
					{
						// The cow is already done
						CT_ENEMY_ANIM( pEnemy )->PlayAnimation( TPS8( idle_lp ) );
						SendUnitBackToStart( TREINTERPRETCAST( TINT, pEnemy ) );
					}
					else
					{
						if ( pCow->pObject->IsAnimPlaying( TPS8( tip_1rac_loop ), 0.0f ) != TTRUE )
							break;

						// Start the push at the time of the cow loop
						TFLOAT fCowTime     = pCow->pObject->GetAnimationTime( TPS8( tip_1rac_loop ) );
						TFLOAT fCowDuration = pCow->pObject->GetToshiAnimInterface().FindAnimationRef( TPS8( tip_1rac_loop ) )->GetNamedAnimation()->GetDuration();

						if ( CT_ENEMY_ANIM( pEnemy )->GetAnimationTime( TPS8( push_start ) ) >= fCowDuration )
							break;

						TFLOAT fStartDuration = CT_ENEMY_ANIM( pEnemy )->GetToshiAnimInterface().FindAnimationRef( TPS8( push_start ) )->GetNamedAnimation()->GetDuration();
						TFLOAT fTime          = fStartDuration - fCowDuration + fCowTime;

						if ( fTime > fCowDuration )
							fTime = fTime - fCowDuration;

						if ( fStartDuration == fTime )
							break;

						CT_ENEMY_ANIM( pEnemy )->SetAnimationTime( TPS8( push_start ), fTime );
					}
				}

				break;
			}
			default:
			{
				if ( pSteer->GetMode() == AInteriorSteer::MODE_STILL || pSteer->IsInteracting() )
				{
					if ( !IsUnitNotYukking( TREINTERPRETCAST( TINT, pEnemy ) ) )
						break;

					if ( pEnemy->fWaitTime >= 0.0f )
					{
						if ( pEnemy->eType == Enemy::ENEMYTYPE_COYOTE )
						{
							// The coyote disappears
							pEnemy->eState = Enemy::ENEMYSTATE_FADING;

							TVector4 vecEffectPosition = *pEnemy->pUnit->GetSteer()->GetPosition();
							vecEffectPosition.y        = vecEffectPosition.y - 0.5f;

							AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, vecEffectPosition, TVector4::VEC_NEGY );
						}
						else
						{
							pSteer->SetMode( AInteriorSteer::MODE_NORMAL );

							if ( pEnemy->eState == Enemy::ENEMYSTATE_RETURNING || pEnemy->iHits >= m_aiUnitMaxHits[ pEnemy->eType ] )
							{
								pEnemy->iRunning = 0;
								pEnemy->eState   = Enemy::ENEMYSTATE_ACTIVE;
								SendUnitBackToStart( TREINTERPRETCAST( TINT, pEnemy ) );
								break;
							}

							pEnemy->iRunning = 0;
							pEnemy->eState   = Enemy::ENEMYSTATE_ACTIVE;
							MakeEnemyRetreat( TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pEnemy ) );
						}
					}
					else
					{
						pEnemy->fWaitTime += a_fUnk1;

						if ( pEnemy->fWaitTime <= 0.0f )
						{
							TVector4 vecTarget;
							TVector4 vecTargetDirection;
							GetEnemyRetreatPosition( &vecTarget, TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pEnemy ), TREINTERPRETCAST( TVector4*, &vecTargetDirection.x ) );

							if ( !pEnemy->bUnk4F )
								break;

							if ( !CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( growl ), 0.0f ) )
							{
								if ( pEnemy->fWaitTime >= -600.0f )
									break;

								StartWaiting( TREINTERPRETCAST( TINT, pEnemy ), TFALSE );
								CT_ENEMY_ANIM( pEnemy )->PlayAnimation( TPS8( growl ) );
								pEnemy->fWaitTime = -1000.0f;

								CT_ENEMY_ANIM( pEnemy )->GetModelInstance()->GetUpVector( pEnemy->vTurnStartFacing );
								pEnemy->vTurnStartFacing.x = -pEnemy->vTurnStartFacing.x;
								pEnemy->vTurnStartFacing.y = -pEnemy->vTurnStartFacing.y;
								pEnemy->vTurnStartFacing.z = -pEnemy->vTurnStartFacing.z;
								pEnemy->fTurnProgress      = 0.0f;
							}
							else
							{
								if ( !pEnemy->bUnk4F )
									break;

								if ( pEnemy->fWaitTime >= -600.0f )
									break;

								// Turn to the target while growling
								TVector4 vecFacing;
								CT_ENEMY_ANIM( pEnemy )->GetModelInstance()->GetUpVector( vecFacing );
								vecFacing.x = -vecFacing.x;
								vecFacing.y = -vecFacing.y;
								vecFacing.z = -vecFacing.z;

								TVector4 vecDifference;
								vecDifference.Substract( vecFacing, vecTargetDirection );

								if ( vecDifference.MagnitudeSq() <= 0.01f )
									break;

								TQuaternion quatFrom;
								quatFrom.SetVectorDirection( pEnemy->vTurnStartFacing.AsVector3(), TVector3::VEC_POSZ );

								TQuaternion quatTo;
								quatTo.SetVectorDirection( vecTargetDirection.AsVector3(), TVector3::VEC_POSZ );

								pEnemy->fTurnProgress = a_fUnk1 * m_fEnemyTurnSpeed + pEnemy->fTurnProgress;
								TMath::Clip( pEnemy->fTurnProgress, 0.0f, 1.0f );

								TQuaternion::Slerp( quatFrom, quatFrom, quatTo, pEnemy->fTurnProgress );

								TVector4 vecNewFacing;
								RotateVector( vecNewFacing, quatFrom, TVector4::VEC_POSZ );

								pSteer->GetKinematics()->SetFacing( vecNewFacing );
								CT_ENEMY_ANIM( pEnemy )->GetModelInstance()->SetDirection( vecNewFacing );
							}
						}
						else
						{
							if ( CT_ENEMY_ANIM( pEnemy )->IsAnimationPlanned( TPS8( howl ) ) || CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( howl ), 0.0f ) )
							{
								CT_ENEMY_ANIM( pEnemy )->PlayAnimation( TPS8( run_lp ) );
								pEnemy->fWaitTime = -0.00001f;
							}
							else
							{
								pEnemy->fWaitTime = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( m_fEnemyWaitTimeMin, m_fEnemyWaitTimeMax );
								pSteer->SetMode( AInteriorSteer::MODE_NORMAL );

								if ( pEnemy->eType != Enemy::ENEMYTYPE_COYOTE )
									break;

								CT_ENEMY_ANIM( pEnemy )->PlayAnimation( TPS8( run_lp ) );
							}
						}
					}
				}
				else if ( pEnemy->eState == Enemy::ENEMYSTATE_RETURNING )
				{
					// Running home
					AAnimatableObject* pAnim = pUnit->GetAnimatableObject();

					if ( pAnim->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) || pAnim->IsAnimPlaying( TPS8( run_lp ), 0.0f ) || pAnim->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
					{
						if ( !pSteer->IsFollowingPath() || pEnemy->iRunning == 0 )
						{
							pEnemy->eState   = Enemy::ENEMYSTATE_ACTIVE;
							pEnemy->iRunning = 0;
							SendUnitBackToStart( TREINTERPRETCAST( TINT, pEnemy ) );
						}

						if ( pSteer->GetSpeed() < 2.0f )
						{
							pEnemy->iRunning = 0;
							MakeUnitRunAway( TREINTERPRETCAST( TINT, pEnemy ), 0.0f, 0.0f, 0.0f );
						}
					}

					TVector4 vecExit;
					GetEnemyExitPosition( &vecExit, pEnemy );

					if ( TVector4::DistanceXZ( vecExit, vecPosition ) < m_fExitRadius )
						RemoveEnemy( pEnemy );
				}
				else if ( pEnemy->eState == Enemy::ENEMYSTATE_INACTIVE )
				{
					break;
				}
				else if ( pEnemy->eType == Enemy::ENEMYTYPE_COYOTE )
				{
					if ( CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( howl ), 0.0f ) )
						break;

					TVector4 vecStart;
					GetEnemyStartPosition( &vecStart, pEnemy );

					if ( TVector4::DistanceXZ( vecStart, vecPosition ) < m_fExitRadius )
					{
						RemoveEnemy( pEnemy );
					}
					else
					{
						if ( pEnemy->fWaitTime <= 0.0f || pEnemy->fUnk24 >= 0.0f )
							break;

						pEnemy->fWaitTime -= a_fUnk1;

						if ( pEnemy->fWaitTime >= 0.0f )
						{
							if ( pSteer->IsFollowingPath() )
								break;

							MakeEnemyRetreat( TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pEnemy ) );
						}
						else
						{
							StartWaiting( TREINTERPRETCAST( TINT, pEnemy ), TTRUE );
						}
					}
				}
				else
				{
					if ( CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( growl ), 0.0f ) )
						break;

					TVector4 vecTarget;
					TVector4 vecTargetDirection;
					GetEnemyRetreatPosition( &vecTarget, TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pEnemy ), TREINTERPRETCAST( TVector4*, &vecTargetDirection.x ) );

					Cow* pTargetCow = TNULL;
					Cow* pCow       = TNULL;

					if ( !pEnemy->bUnk4F )
					{
						pTargetCow = &CT_COWS( this )[ pEnemy->iCow ];
						pCow       = TSTATICCAST( Cow, FindClosestCow( &vecPosition, m_fCowSearchRadius ) );
					}

					if ( !pEnemy->bUnk4F && pCow && pTargetCow == pCow )
					{
						// At its cow
						TVector4 vecSideDirection;
						void*    pHelper;
						TVector4 vecSidePosition;
						GetCowSideAndHelper( &vecSidePosition, pCow, pEnemy, &vecSideDirection.x, &pHelper );

						if ( TVector4::DistanceXZ( vecSidePosition, vecPosition ) >= m_fCowSideMaxDistance )
						{
							if ( !pTargetCow || pTargetCow->fTipProgress <= m_fTipThreshold )
								break;

							SendUnitBackToStart( TREINTERPRETCAST( TINT, pEnemy ) );
						}
						else
						{
							AAnimatableObject* pCowObject = pCow->pObject;

							if ( pCow->fTipProgress > m_fTipThreshold ||
							     pCowObject->IsAnimationPlanned( TPS8( spin_idle ) ) ||
							     pCowObject->IsAnimationPlanned( TPS8( spin ) ) ||
							     pCowObject->IsAnimationPlanned( TPS8( tip_1rac_end ) ) ||
							     pCowObject->IsAnimationPlanned( TPS8( push_start ) ) ||
							     pCowObject->IsAnimationPlanned( TPS8( getup ) ) ||
							     pCowObject->IsAnimPlaying( TPS8( getup ), 0.0f ) ||
							     pCowObject->IsAnimPlaying( TPS8( headhit ), 0.0f ) ||
							     pCowObject->IsAnimPlaying( TPS8( backhit ), 0.0f ) ||
							     pCowObject->IsAnimPlaying( TPS8( fronthit ), 0.0f ) ||
							     pCowObject->IsAnimationPlanned( TPS8( headhit ) ) ||
							     pCowObject->IsAnimationPlanned( TPS8( backhit ) ) ||
							     pCowObject->IsAnimationPlanned( TPS8( fronthit ) ) )
							{
								break;
							}

							if ( pEnemy->bUnk4D )
							{
								if ( !pEnemy->bUnk4F )
								{
									TUINT uiBit = GetUnitStartBit( pEnemy, CT_STAGE_TYPE( this ) );

									if ( uiBit <= 29 )
										ClearBit( m_auiCowSideReservedBits, uiBit );
								}

								pEnemy->bUnk4D = TFALSE;
							}

							if ( pCow->fTipProgress > 0.0f )
							{
								// Join the push of the other enemy
								if ( !CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
									break;

								if ( CT_ENEMY_ANIM( pEnemy )->IsAnimPlaying( TPS8( push_start ), 0.0f ) )
									break;

								if ( CT_ENEMY_ANIM( pEnemy )->IsAnimationPlanned( TPS8( push_start ) ) )
									break;

								if ( !pCowObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) &&
								     ( !pCowObject->IsAnimPlaying( TPS8( tip_1rac_loop ), 0.0f ) ||
								       pCowObject->IsAnimationPlanned( TPS8( idle_lp ) ) ||
								       pCowObject->GetAnimationTime( TPS8( tip_1rac_loop ) ) < 1.47f ) )
								{
									break;
								}
							}

							StartPushing( TREINTERPRETCAST( TINT, pEnemy ), TREINTERPRETCAST( TINT, pCow ) );
						}
					}
					else
					{
						// On the way
						if ( m_iCarryBone != -1 && pEnemy->eState == Enemy::ENEMYSTATE_ACTIVE && !CT_COWS( this )[ pEnemy->iCow ].bActive )
						{
							if ( !AAnimatableObjectManager::GetSingleton()->GetAttachedObject( CT_ENEMY_ANIM( pEnemy ), m_iCarryBone, 0 ) )
								SendEnemyToCow( TREINTERPRETCAST( TINT, pEnemy ) );
						}

						if ( pEnemy->iHits != 1 )
							pEnemy->fWaitTime -= a_fUnk1;

						TFLOAT fDistance = TVector4::DistanceXZ( vecTarget, vecPosition );

						if ( pEnemy->fWaitTime >= 0.0f )
						{
							if ( m_fUnk_00E2C + m_fUnk_00E30 <= fDistance )
							{
								MakeUnitRunAway( TREINTERPRETCAST( TINT, pEnemy ), 0.0f, 0.0f, 0.0f );
							}
							else
							{
								TFLOAT fRandom = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, m_fUnk_00E30 );

								if ( fRandom + m_fUnk_00E2C > fDistance && pEnemy->iHits == 0 && pEnemy->fUnk24 < 0.0f )
									ResetEnemySpeed( pEnemy );
							}
						}
						else if ( pEnemy->fUnk24 < 0.0f && pEnemy->eState != Enemy::ENEMYSTATE_RETURNING && fDistance > m_fUnk_00E34 )
						{
							StartWaiting( TREINTERPRETCAST( TINT, pEnemy ), TTRUE );
						}

						if ( fDistance >= 1.0f || !pEnemy->bUnk4F )
							break;

						// Reached the target, growl at it
						StartWaiting( TREINTERPRETCAST( TINT, pEnemy ), TFALSE );
						CT_ENEMY_ANIM( pEnemy )->PlayAnimation( TPS8( growl ) );
						pSteer->SetDesiredDirection( vecTargetDirection );
						pEnemy->fWaitTime = -1000.0f;

						CT_ENEMY_ANIM( pEnemy )->GetModelInstance()->GetUpVector( pEnemy->vTurnStartFacing );
						pEnemy->vTurnStartFacing.x = -pEnemy->vTurnStartFacing.x;
						pEnemy->vTurnStartFacing.y = -pEnemy->vTurnStartFacing.y;
						pEnemy->vTurnStartFacing.z = -pEnemy->vTurnStartFacing.z;
						pEnemy->fTurnProgress      = 0.0f;
					}
				}

				break;
			}
		}
	}

	return TFALSE;
}
