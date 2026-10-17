#include "pch.h"
#include "ACowTippingMiniGameState.h"
#include "Units/Projectiles/AProjectile.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Units/AAnimatedUnit.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "Sound/ASoundManager.h"
#include "FX/AParticleEffectManager.h"
#include "GUI/AFadeManager.h"
#include <Render/TCameraObject.h> // before the Physics headers (Opcode #defines PI)
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Terrain/ATerrainInterface.h"

#include <Math/TMath.h>
#include <Toshi/TStringManager.h>

#include <bit>
#include "SoundBank/cowtip.h"
#include "SoundBank/dagboss.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Defined in AGTAStyleCameraHelper.cpp (006186a0)
TBOOL SweepSphere( const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength, TFLOAT* a_pOutFraction, TVector4* a_pOutNormal, TINT a_iFlags, TUINT a_uiCollisionMask );

// NOTE: same as the static NormaliseXZ of AGTAStyleCameraHelper.cpp (TVector4 method 006c7540)
static TFLOAT NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return fMagSq;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;

	return fMag;
}

TPSTRING8_DECLARE( backhit );
TPSTRING8_DECLARE( default );
TPSTRING8_DECLARE( down );
TPSTRING8_DECLARE( duck );
TPSTRING8_DECLARE( duck_lp );
TPSTRING8_DECLARE( duck_up );
TPSTRING8_DECLARE( fronthit );
TPSTRING8_DECLARE( headhit );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( impact );
TPSTRING8_DECLARE( push_end );
TPSTRING8_DECLARE( push_lp );
TPSTRING8_DECLARE( push_start );
TPSTRING8_DECLARE( sl_idle_lp );
TPSTRING8_DECLARE( slingshot_miss );
TPSTRING8_DECLARE( spin );
TPSTRING8_DECLARE( spin_idle );
TPSTRING8_DECLARE( tawnt );
TPSTRING8_DECLARE( tip_1rac_end );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( yukobject );

// NOTE: ACowTippingMiniGameState is split over several .cpp files, this one has the [Proj] part:
// the fixed step of the slingshot projectiles (UpdateProjectiles) and the hit tests against the units.

using Projectile = ACowTippingMiniGameState::Projectile;
using Enemy      = ACowTippingMiniGameState::Enemy;
using Cow        = ACowTippingMiniGameState::Cow;

// NOTE: most fields of the provisional header are plain TINT/TFLOAT. As<T>() views a field
// (or a run of fields) as the type the [Proj] code uses, e.g. m_vPlayerPos.
template <class T, class F>
static TFORCEINLINE T& As( F& a_rField )
{
	return *TREINTERPRETCAST( T*, &a_rField );
}

// Pointers and floats passed to methods of the other parts whose provisional declarations take TINT
template <class F>
static TFORCEINLINE TINT AsInt( F a_Value )
{
	if constexpr ( std::is_pointer_v<F> )
		return TREINTERPRETCAST( TINT, a_Value );
	else
		return std::bit_cast<TINT>( a_Value );
}

using Raccoon = ACowTippingMiniGameState::Raccoon;

// Stage types 2 and 3 throw "cowtipping_largeobject" instead of walnuts and use their own tuning
static TFORCEINLINE TBOOL IsLargeObjectStage( ACowTippingMiniGameState* a_pThis )
{
	TINT iStageType = a_pThis->m_aStages[ a_pThis->m_iStage ].iStageType;
	return iStageType == ACowTippingMiniGameState::Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == ACowTippingMiniGameState::Stage::STAGETYPE_JUNKYARD;
}

static TFORCEINLINE TBOOL IsZero3( const TVector4& a_rcVec )
{
	return a_rcVec.x == 0.0f && a_rcVec.y == 0.0f && a_rcVec.z == 0.0f;
}

static TFORCEINLINE TVector4 GetModelPosition( AAnimatableObject* a_pObject )
{
	TVector4 vPosition;
	a_pObject->GetModelInstance()->GetPosition( vPosition );
	return vPosition;
}

//-----------------------------------------------------------------------------
// NOTE: 0066a920, 0066aa90 and 0066ab40 are projectile helpers of the engine area next to
// AProjectile (0066a0d0..), only used by this minigame.
//-----------------------------------------------------------------------------

// Where a_rcPosition gets moving with a_rcVelocity for a_fTime (w is kept)
// $Barnyard: FUNCTION 0066aa00
static void FUN_0066aa00( const TVector4& a_rcPosition, const TVector4& a_rcVelocity, TFLOAT a_fTime, TVector4& a_rOutPosition )
{
	TVector4 vOffset;
	vOffset.Multiply( a_rcVelocity, a_fTime );

	TFLOAT fW        = a_rcPosition.w;
	a_rOutPosition.x = vOffset.x + a_rcPosition.x;
	a_rOutPosition.y = vOffset.y + a_rcPosition.y;
	a_rOutPosition.z = vOffset.z + a_rcPosition.z;
	a_rOutPosition.w = fW;
}

// Velocity of a lob from a_rcFrom that lands at a_rcTarget, its horizontal speed is a_fSpeed * sqrt( distance ) / 3
// Where a target moving with a_rcTargetVelocity will be when a projectile thrown from a_rcFrom with a_rcVelocity reaches it
// $Barnyard: FUNCTION 0066aa90
static void FUN_0066aa90( const TVector4& a_rcTarget, const TVector4& a_rcTargetVelocity, const TVector4& a_rcFrom, const TVector4& a_rcVelocity, TVector4& a_rOutTarget )
{
	TFLOAT fTime = TVector4::DistanceXZ( a_rcTarget, a_rcFrom ) / a_rcVelocity.MagnitudeXZ();

	TVector4 vOffset;
	vOffset.Multiply( a_rcTargetVelocity, fTime );

	TFLOAT fW      = a_rcTarget.w;
	a_rOutTarget.x = vOffset.x + a_rcTarget.x;
	a_rOutTarget.y = vOffset.y + a_rcTarget.y;
	a_rOutTarget.z = vOffset.z + a_rcTarget.z;
	a_rOutTarget.w = fW;
}

// $Barnyard: FUNCTION 0066ab40
static void FUN_0066ab40( const TVector4& a_rcTarget, const TVector4& a_rcTargetVelocity, const TVector4& a_rcFrom, TFLOAT a_fSpeed, TVector4& a_rOutVelocity, TVector4& a_rOutTarget )
{
	a_rOutTarget = a_rcTarget;
	AProjectile::CalculateThrowVelocity( a_rOutTarget, a_rcFrom, a_fSpeed, a_rOutVelocity );
	FUN_0066aa90( a_rOutTarget, a_rcTargetVelocity, a_rcFrom, a_rOutVelocity, a_rOutTarget );
}

// Moves the start area of a_pPath to the first area of the path that contains its start position and drops the areas before it
// NOTE: copy of AInteriorNetwork::UpdateStartAreaFromPath (005517c0), the network isn't used
static void CT_UpdateStartAreaFromPath( AInteriorNetwork* a_pNetwork, AInteriorNetworkPath* a_pPath )
{
	TVector4 vecStart( a_pPath->m_vStartPosition.x, a_pPath->m_vStartPosition.z, 0.0f, 0.0f );
	TINT     iNumAreas = a_pPath->m_oPath.m_iNumPathAreas;

	for ( TINT i = 0; i < iNumAreas; i++ )
	{
		TINT iArea = a_pPath->m_oPath.GetPathAreaIndex( i );

		if ( a_pNetwork->GetArea( iArea )->CheckPointInsideArea( vecStart ) )
		{
			a_pPath->m_iStartArea = iArea;
			a_pPath->m_oPath.AdvancePath( i );
			return;
		}
	}
}

// Aims a projectile thrown from a_rcFrom at a unit following a path: walks the path of the steer, segment by
// segment, to find where the unit will be when the projectile lands (a_fLeadFactor scales the speed of the unit),
// then makes the unit seek that point. a_fHeightOffset is added to the height of the target.
// Units that don't follow a path are aimed at directly.
// $Barnyard: FUNCTION 0066ab90
static TBOOL FUN_0066ab90( AInteriorSteer* a_pSteer, const TVector4& a_rcFrom, TFLOAT a_fSpeed, TVector4& a_rOutVelocity, TVector4& a_rOutTarget, TFLOAT a_fHeightOffset, TFLOAT a_fLeadFactor )
{
	if ( a_pSteer->GetMode() == AInteriorSteer::MODE_STILL || a_pSteer->IsInteracting() || !a_pSteer->IsFollowingPath() )
	{
		a_pSteer->Command_CancelCurrentCommand();

		TVector4 vecTarget = *a_pSteer->GetPosition();
		vecTarget.y += a_fHeightOffset;

		AProjectile::CalculateThrowVelocity( vecTarget, a_rcFrom, a_fSpeed, a_rOutVelocity );
		a_rOutTarget = vecTarget;

		// NOTE: retail returns the low byte of the a_rOutTarget pointer here (the result is never used)
		return TTRUE;
	}

	AInteriorNetwork*     pNetwork = TSTATICCAST( AInteriorNetwork, a_pSteer->GetCurrentNetwork() );
	AInteriorNetworkPath& rPath    = a_pSteer->GetPath();

	a_rOutTarget = rPath.m_vStartPosition;

	// The path is restored when done
	TINT     iSavedStartArea = rPath.m_iStartArea;
	TVector4 vecSavedStart   = rPath.m_vStartPosition;

	TFLOAT fUnitSpeed = ( a_pSteer->GetDesiredSpeed() * 0.2f + a_pSteer->GetTraits()->m_fMaxSpeed * 0.8f ) * a_fLeadFactor;

	TVector2 vecStart( rPath.m_vStartPosition.x, rPath.m_vStartPosition.z );
	TVector2 vecDestination( rPath.m_oPath.m_vDestination.x, rPath.m_oPath.m_vDestination.z );

	CT_UpdateStartAreaFromPath( pNetwork, &rPath );

	TVector2 vecNext;
	TINT     iNextArea;
	TFLOAT   fUnitTime = 0.0f;

	if ( pNetwork->FindNextPathPoint( &rPath.m_oPath, rPath.m_fRadius, vecStart, vecDestination, vecNext, &iNextArea ) )
	{
		do
		{
			TVector4 vecSegmentStart( vecStart.x, 0.0f, vecStart.y, 1.0f );
			TVector4 vecSegmentEnd( vecNext.x, 0.0f, vecNext.y, 1.0f );

			// Velocity of the unit along the segment
			TVector4 vecUnitVelocity( vecNext.x - vecStart.x, 0.0f, vecNext.y - vecStart.y, 1.0f );
			vecUnitVelocity.NormaliseXZ();
			vecUnitVelocity.Multiply( fUnitSpeed );

			TFLOAT fSegmentTime = TVector4::DistanceXZ( vecSegmentEnd, vecSegmentStart ) / vecUnitVelocity.MagnitudeXZ();

			a_rOutTarget = vecSegmentStart;
			fUnitTime += fSegmentTime;

			TVector4 vecVelocity;
			AProjectile::CalculateThrowVelocity( a_rOutTarget, a_rcFrom, a_fSpeed, vecVelocity );
			FUN_0066aa90( a_rOutTarget, vecUnitVelocity, a_rcFrom, vecVelocity, a_rOutTarget );

			if ( TVector4::DistanceSq( vecSegmentStart, vecSegmentEnd ) < 0.001f )
				break;

			TFLOAT fFlightTime = TVector4::DistanceXZ( a_rcFrom, a_rOutTarget ) / vecVelocity.MagnitudeXZ();

			if ( !( fUnitTime < fFlightTime ) )
				break;

			// Where the unit is on the segment when the projectile lands
			FUN_0066aa00( vecSegmentEnd, vecUnitVelocity, fFlightTime - fUnitTime, a_rOutTarget );

			if ( pNetwork->FindAreaContainingPointXZ( TVector2( a_rOutTarget.x, a_rOutTarget.z ) ) != -1 )
				break;

			// The point is past the end of the segment (off the network), continue with the next one
			vecStart = vecNext;

			// NOTE: retail moves the path start to the start of the segment that was just checked, not to its end
			rPath.m_vStartPosition = vecSegmentStart;
			CT_UpdateStartAreaFromPath( pNetwork, &rPath );

		} while ( pNetwork->FindNextPathPoint( &rPath.m_oPath, rPath.m_fRadius, vecStart, vecDestination, vecNext, &iNextArea ) );
	}

	TINT iArea = pNetwork->FindAreaContainingPointXZ( TVector2( a_rOutTarget.x, a_rOutTarget.z ) );
	pNetwork->ResolvePointHeight( iArea, a_rOutTarget, 0 );
	a_rOutTarget.y += a_fHeightOffset;

	AProjectile::CalculateThrowVelocity( a_rOutTarget, a_rcFrom, a_fSpeed, a_rOutVelocity );

	rPath.SetStart( vecSavedStart, iSavedStartArea );
	a_pSteer->Command_CancelCurrentCommandNoStop();

	return a_pSteer->SeekPoint( a_rOutTarget );
}

// XZ distance between the segments a_rcStart1-a_rcEnd1 (extended by 0.25 at a_rcStart1) and a_rcStart2-a_rcEnd2, minus 0.05
// NOTE: a game helper outside of the minigame's range, only used here
// NOTE: same as 004de880 (ATapperMiniGameState::GetGlassToPlayerDistance)
static TFLOAT CT_GetSegmentDistanceXZ( const TVector4& a_rcStart1, const TVector4& a_rcEnd1, const TVector4& a_rcStart2, const TVector4& a_rcEnd2 )
{
	TVector2 vStart1( a_rcStart1.x, a_rcStart1.z );
	TVector2 vEnd1( a_rcEnd1.x, a_rcEnd1.z );
	TVector2 vDirection( vStart1.x - vEnd1.x, vStart1.y - vEnd1.y );

	if ( vDirection.y * vDirection.y + vDirection.x * vDirection.x > 0.0f )
	{
		vDirection.Normalize();
		vStart1.x = vDirection.x * 0.25f + vStart1.x;
		vStart1.y = vDirection.y * 0.25f + vStart1.y;
	}

	TVector2 vStart2( a_rcStart2.x, a_rcStart2.z );
	TVector2 vEnd2( a_rcEnd2.x, a_rcEnd2.z );

	return TMath::DistanceSegmentToSegment2D( vStart1, vEnd1, vStart2, vEnd2 ) - 0.050000001f;
}

// Copies the vectors at 0x38 and 0x58 of a cow
// $Barnyard: FUNCTION 004a7510
static void FUN_004a7510( Cow* a_pCow, TVector4* a_pOut1, TVector4* a_pOut2 )
{
	*a_pOut1 = a_pCow->vTipSpot2;
	*a_pOut2 = a_pCow->vTipDirection2;
}

// Whether the player character is ducking
// $Barnyard: FUNCTION 004a7590
TBOOL ACowTippingMiniGameState::IsPlayerDucking()
{
	AAnimatableObject* pObject = m_pLauncher;

	return pObject->IsAnimPlaying( TPS8( duck ), 0.0f ) ||
	    pObject->IsAnimPlaying( TPS8( duck_lp ), 0.0f ) ||
	    pObject->IsAnimPlaying( TPS8( duck_up ), 0.0f );
}

// Plays one of three sounds at the enemy, depending on its type (eType)
// $Barnyard: FUNCTION 004a8590
void ACowTippingMiniGameState::PlayEnemySound( Enemy* a_pEnemy, ASoundId a_eSound0, ASoundId a_eSound1, ASoundId a_eSound2 )
{
	AAnimatedUnit* pUnit = a_pEnemy->pUnit;

	switch ( a_pEnemy->eType )
	{
		case Enemy::ENEMYTYPE_KID_BOY:
			ASoundManager::GetSingleton()->PlayCueAtPosition( a_eSound0, *pUnit->GetSteer()->GetPosition() );
			break;
		case Enemy::ENEMYTYPE_KID_GIRL:
			ASoundManager::GetSingleton()->PlayCueAtPosition( a_eSound1, *pUnit->GetSteer()->GetPosition() );
			break;
		case Enemy::ENEMYTYPE_KID_BOY_FAT:
			ASoundManager::GetSingleton()->PlayCueAtPosition( a_eSound2, *pUnit->GetSteer()->GetPosition() );
			break;
	}
}

// Throw speed of the current stage
// NOTE: retail is a static function that reads the singleton (00782efc), which is this
// $Barnyard: FUNCTION 004a8fd0
TFLOAT ACowTippingMiniGameState::GetThrowSpeed()
{
	return IsLargeObjectStage( this ) ? m_fThrowSpeedLarge : m_fThrowSpeed;
}

// Velocity to throw from a_pFrom to land at a_pTo, under the gravity of the minigame (m_fTrajectoryGravity)
// NOTE: retail is a static function that reads the singleton (00782efc), which is this
// $Barnyard: FUNCTION 004a9000
TVector4* ACowTippingMiniGameState::CalculateThrowVelocity( TVector4* a_pOutVelocity, const TVector4* a_pFrom, const TVector4* a_pTo, TFLOAT a_fSpeed )
{
	TVector4 vDirection;
	vDirection.x = a_pTo->x - a_pFrom->x;
	vDirection.y = 0.0f;
	vDirection.z = a_pTo->z - a_pFrom->z;
	vDirection.w = a_pTo->w;

	TFLOAT fSpeed = TMath::Sqrt( vDirection.NormaliseMag() ) * a_fSpeed * 0.33333334f;

	TVector4 vVelocity;
	vVelocity.Multiply( vDirection, fSpeed );

	TFLOAT fTime = TVector4::DistanceXZ( *a_pFrom, *a_pTo ) / fSpeed;

	a_pOutVelocity->x = vVelocity.x;
	a_pOutVelocity->y = -( ( fTime * m_fTrajectoryGravity * fTime * 0.5f - ( a_pTo->y - a_pFrom->y ) ) / fTime );
	a_pOutVelocity->z = vVelocity.z;
	a_pOutVelocity->w = vVelocity.w;

	return a_pOutVelocity;
}

// Velocity to throw from a_pPosition to land at m_vAimTarget
// $Barnyard: FUNCTION 004a90f0
TVector4* ACowTippingMiniGameState::CalculateThrowVelocityToTarget( TVector4* a_pOutVelocity, const TVector4* a_pPosition )
{
	// NOTE: GetThrowSpeed is inlined in retail
	TVector4 vVelocity;
	AProjectile::CalculateThrowVelocity( m_vAimTarget, *a_pPosition, GetThrowSpeed(), vVelocity );

	*a_pOutVelocity = vVelocity;
	return a_pOutVelocity;
}

// Velocity to throw from a_pPosition to land at m_vReticleTarget
// $Barnyard: FUNCTION 004a9170
TVector4* ACowTippingMiniGameState::CalculateThrowVelocityToTarget2( ACowTippingMiniGameState* a_pThis, TVector4* a_pOutVelocity, const TVector4* a_pPosition )
{
	// NOTE: GetThrowSpeed is inlined in retail (it reads the singleton)
	a_pThis->CalculateThrowVelocity( a_pOutVelocity, a_pPosition, &a_pThis->m_vReticleTarget, a_pThis->GetThrowSpeed() );
	return a_pOutVelocity;
}

// Position of the interior goal "<a_szGoalName><a_iIndex>" closest to a_pPosition
// $Barnyard: FUNCTION 004a93d0
TVector4* ACowTippingMiniGameState::GetClosestGoalPosition( TVector4* a_pOutPosition, TINT a_iIndex, const TCHAR* a_szGoalName, const TVector4* a_pPosition )
{
	TCHAR szGoalName[ 100 ];
	TStringManager::String8Format( szGoalName, sizeof( szGoalName ), "%s%d", a_szGoalName, a_iIndex );

	AInteriorGoal* pGoal = m_pNetwork->FindClosestGoalOrFirst( szGoalName, *a_pPosition );

	TVector4 vPosition;
	pGoal->GetPosition( vPosition );

	*a_pOutPosition = vPosition;
	return a_pOutPosition;
}

// Hit points of a cow: out 1/3 get 0x28/0x48, out 2/4 get 0x38/0x58 (or the same as 1/3 in stages of type 2/3)
// $Barnyard: FUNCTION 004a9b40
TVector4* ACowTippingMiniGameState::GetCowHitPoints( Cow* a_pCow, TVector4* a_pOut1, TVector4* a_pOut2, TVector4* a_pOut3, TVector4* a_pOut4 )
{
	*a_pOut1 = a_pCow->vTipSpot1;
	*a_pOut3 = a_pCow->vTipDirection1;

	if ( IsLargeObjectStage( this ) )
	{
		*a_pOut2 = *a_pOut1;
		*a_pOut4 = *a_pOut3;
	}
	else
	{
		*a_pOut2 = a_pCow->vTipSpot2;
		*a_pOut4 = a_pCow->vTipDirection2;
	}

	return a_pOut4;
}

// The player got hit: white flash, vibration and a sound
// $Barnyard: FUNCTION 004aaa10
TINT ACowTippingMiniGameState::OnPlayerHit()
{
	// NOTE: retail passes a 4th argument (TTRUE) that StartFade doesn't use
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 191, 255, 255, 255 ), AFade::Color( 0, 255, 255, 255 ), m_fHitFadeTime );

	// NOTE: retail builds a vibration { m_fHitRumbleStrength, 0, 0, m_fHitRumbleUnk, 0, 1.0f, 1.0f, 1 } and passes it
	// to FUN_005cc080 of the input device at AInputHandler + 0x4c, which is an empty stub on PC (returns 0).

	TINT iResult = ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );
	m_fShakeTime = m_fHitShakeTime;

	return iResult;
}

// Clears FLAGS_MILK_TARGET of the enemies, of the units of the m_llActiveRaccoons list and of the raccoons
// $Barnyard: FUNCTION 004ab270
void ACowTippingMiniGameState::ClearUnitsFlag9()
{
	T2GenericSList& rEnemies = As<T2GenericSList>( m_llActiveEnemies );

	for ( auto pNode = rEnemies.Begin(); pNode != rEnemies.End(); pNode = pNode->Next() )
		As<AAnimatedUnit*>( TREINTERPRETCAST( Enemy*, pNode )->pUnit )->RemoveFlags( AUnit::FLAGS_MILK_TARGET );

	T2GenericSList& rList13268 = As<T2GenericSList>( m_llActiveRaccoons );

	for ( auto pNode = rList13268.Begin(); pNode != rList13268.End(); pNode = pNode->Next() )
		TSTATICCAST( Raccoon, pNode )->pUnit->RemoveFlags( AUnit::FLAGS_MILK_TARGET );

	T2GenericSList& rRaccoons = As<T2GenericSList>( m_llFreeRaccoons );

	for ( auto pNode = rRaccoons.Begin(); pNode != rRaccoons.End(); pNode = pNode->Next() )
		As<AAnimatedUnit*>( TREINTERPRETCAST( Raccoon*, pNode )->pUnit )->RemoveFlags( AUnit::FLAGS_MILK_TARGET );
}

// Aims at an enemy that follows a path through the interior network
// NOTE: retail is a static function that reads the singleton (00782efc), which is this
// $Barnyard: FUNCTION 004ab3f0
TBOOL ACowTippingMiniGameState::AimAtPathFollower( AInteriorSteer* a_pSteer, const TVector4* a_pFrom, TVector4* a_pOutVelocity, TVector4* a_pOutTarget )
{
	TFLOAT fSpeed = GetThrowSpeed(); // NOTE: inlined in retail

	return FUN_0066ab90( a_pSteer, *a_pFrom, fSpeed, *a_pOutVelocity, *a_pOutTarget, -0.6f, 0.75f );
}

// Aims at a target moving with a_pTargetVelocity
// NOTE: retail is a static function that reads the singleton (00782efc), which is this
// $Barnyard: FUNCTION 004ab450
void ACowTippingMiniGameState::AimAtMovingTarget( const TVector4* a_pTarget, const TVector4* a_pTargetVelocity, const TVector4* a_pFrom, TVector4* a_pOutVelocity, TVector4* a_pOutTarget )
{
	// NOTE: GetThrowSpeed is inlined in retail
	FUN_0066ab40( *a_pTarget, *a_pTargetVelocity, *a_pFrom, GetThrowSpeed(), *a_pOutVelocity, *a_pOutTarget );
}

// The enemy closest (XZ) to a_pPosition and closer than a_fMaxDistance
// $Barnyard: FUNCTION 004ab5f0
Enemy* ACowTippingMiniGameState::FindClosestEnemy( const TVector4* a_pPosition, TFLOAT a_fMaxDistance )
{
	T2GenericSList& rEnemies = As<T2GenericSList>( m_llActiveEnemies );
	Enemy*          pClosest = TNULL;

	for ( auto pNode = rEnemies.Begin(); pNode != rEnemies.End(); pNode = pNode->Next() )
	{
		Enemy*   pEnemy    = TREINTERPRETCAST( Enemy*, pNode );
		TVector4 vPosition = *pEnemy->pUnit->GetSteer()->GetPosition();
		TFLOAT   fDistance = TVector4::DistanceXZ( *a_pPosition, vPosition );

		if ( fDistance < a_fMaxDistance )
		{
			a_fMaxDistance = fDistance;
			pClosest       = pEnemy;
		}
	}

	return pClosest;
}

// The cow hit by a projectile at a_pPosition flying along a_pDirection (not in stages of type 2/3)
// $Barnyard: FUNCTION 004ac900
Cow* ACowTippingMiniGameState::FindHitCow( const TVector4* a_pDirection, const TVector4* a_pPosition, TFLOAT* a_pOutDistance, TFLOAT a_fRadius, TFLOAT a_fMaxDistance, COWHIT* a_pOutHitType, TBOOL a_bUnk )
{
	if ( IsLargeObjectStage( this ) )
		return TNULL;

	Cow* pHitCow = TNULL;

	for ( TINT i = 0; i < m_iNumCows; i++ )
	{
		Cow*               pCow    = &m_aCows[ i ];
		AAnimatableObject* pObject = pCow->pObject;

		if ( pObject->IsAnimationPlanned( TPS8( spin_idle ) ) ||
		     pObject->IsAnimationPlanned( TPS8( spin ) ) ||
		     pObject->IsAnimationPlanned( TPS8( tip_1rac_end ) ) )
			continue;

		TFLOAT fDistance;

		// The head: half the radius and a quarter of the distance
		TVector4 vHead = pCow->vHeadPosition;
		a_fMaxDistance *= 0.25f;

		TFLOAT fNextMaxDistance;
		if ( IsTargetInAim( const_cast<TVector4*>( a_pDirection ), const_cast<TFLOAT*>( &a_pPosition->x ), &vHead.x, AsInt( a_fRadius * 0.5f ), &fDistance, a_fMaxDistance, std::bit_cast<TFLOAT>( TINT( a_bUnk ) ) ) )
		{
			fNextMaxDistance = fDistance;
			pHitCow          = pCow;
			*a_pOutDistance  = fDistance;
			*a_pOutHitType   = COWHIT_BACK;
		}
		else
		{
			fNextMaxDistance = a_fMaxDistance;
		}

		a_fMaxDistance = fNextMaxDistance * 4.0f;

		TVector4 vPoint1 = pCow->vSpinePosition;

		if ( IsTargetInAim( const_cast<TVector4*>( a_pDirection ), const_cast<TFLOAT*>( &a_pPosition->x ), &vPoint1.x, AsInt( a_fRadius ), &fDistance, a_fMaxDistance, std::bit_cast<TFLOAT>( TINT( a_bUnk ) ) ) )
		{
			pHitCow         = pCow;
			a_fMaxDistance  = fDistance;
			*a_pOutDistance = fDistance;
			*a_pOutHitType  = COWHIT_BACK;
		}

		TVector4 vPoint2 = pCow->vSpine1Position;

		if ( IsTargetInAim( const_cast<TVector4*>( a_pDirection ), const_cast<TFLOAT*>( &a_pPosition->x ), &vPoint2.x, AsInt( a_fRadius ), &fDistance, a_fMaxDistance, std::bit_cast<TFLOAT>( TINT( a_bUnk ) ) ) )
		{
			pHitCow         = pCow;
			a_fMaxDistance  = fDistance;
			*a_pOutDistance = fDistance;
			*a_pOutHitType  = COWHIT_BACK;
		}
	}

	return pHitCow;
}

// A projectile of the player hit its target (a_pProjectile is in list 1 and close to the player's hand)
// $Barnyard: FUNCTION 004acc00
void ACowTippingMiniGameState::OnProjectileReachedPlayer( Projectile* a_pProjectile )
{
	TBOOL bLargeObjects = IsLargeObjectStage( this );

	if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 3 ].fTime > 0.0f ||
	     ( m_iReloadState == RELOADSTATE_RELOADING && !bLargeObjects ) ||
	     IsPlayerDucking() )
	{
		// Missed (the player is protected or ducking): the thrower taunts
		AAnimatedUnit* pThrower = a_pProjectile->pThrower;

		if ( pThrower->IsVisible() &&
		     !pThrower->GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) &&
		     !pThrower->GetAnimatableObject()->IsAnimPlaying( TPS8( down ), 0.0f ) &&
		     !pThrower->GetAnimatableObject()->IsAnimationPlanned( TPS8( down ) ) )
		{
			if ( bLargeObjects )
				pThrower->GetAnimatableObject()->PlayAnimation( TPS8( sl_idle_lp ) );
			else
				pThrower->GetAnimatableObject()->PlayAnimation( TPS8( slingshot_miss ) );
		}
	}
	else
	{
		if ( m_fThrowCooldown <= 0.0f )
		{
			AAnimatableObject* pPlayerObject = m_pLauncher;

			m_fThrowCooldown = m_fThrowDelay;
			m_fUnk_132D0     = 0.0f;
			pPlayerObject->GetToshiAnimInterface().KillBaseAnimations();
			pPlayerObject->PlayAnimation( TPS8( impact ) );
			m_iReloadState = RELOADSTATE_HIT;
			m_oTargetReticle.SetActive( TFALSE, TTRUE, 50.0f );
			ClearUnitsFlag9();

			AAnimatedUnit* pThrower = a_pProjectile->pThrower;

			if ( pThrower->IsVisible() &&
			     !pThrower->GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) &&
			     !pThrower->GetAnimatableObject()->IsAnimPlaying( TPS8( down ), 0.0f ) &&
			     !pThrower->GetAnimatableObject()->IsAnimationPlanned( TPS8( down ) ) )
			{
				pThrower->GetAnimatableObject()->KillAllAnimations();
				pThrower->GetAnimatableObject()->PlayAnimation( TPS8( tawnt ) );
			}

			AddLives1( AsInt( &m_aPlayers[ m_iCurrentPlayer ] ), -1 );
			m_pCameraTarget     = a_pProjectile->pThrower;
			m_fCameraTargetTime = 0.0f;
		}

		OnPlayerHit();
	}
}

// Normalises the axes of the model transform (removes the scale)
// $Barnyard: FUNCTION 004af450
void ACowTippingMiniGameState::ResetProjectileScale( Projectile* a_pProjectile )
{
	TMatrix44 mTransform;
	a_pProjectile->pObject->GetModelInstance()->GetLocalTransform( mTransform );

	mTransform.AsBasisVector4( BASISVECTOR_RIGHT ).Normalise();
	mTransform.AsBasisVector4( BASISVECTOR_UP ).Normalise();
	mTransform.AsBasisVector4( BASISVECTOR_FORWARD ).Normalise();

	a_pProjectile->pObject->GetModelInstance()->SetTransform( mTransform );
}

// Scales the model transform by a_fScale
// $Barnyard: FUNCTION 004af5c0
void ACowTippingMiniGameState::SetProjectileScale( Projectile* a_pProjectile, TFLOAT a_fScale )
{
	// NOTE: retail gets the local transform twice into the same matrix
	TMatrix44 mTransform;
	a_pProjectile->pObject->GetModelInstance()->GetLocalTransform( mTransform );
	a_pProjectile->pObject->GetModelInstance()->GetLocalTransform( mTransform );

	mTransform.Scale( a_fScale, a_fScale, a_fScale );
	a_pProjectile->pObject->GetModelInstance()->SetTransform( mTransform );
}

// A random interior goal called "<a_szGoalName><a_iIndex>"
// $Barnyard: FUNCTION 004b0780
AInteriorGoal* ACowTippingMiniGameState::FindRandomGoal( ACowTippingMiniGameState* a_pThis, TINT a_iIndex, const TCHAR* a_szGoalName )
{
	TCHAR szGoalName[ 100 ];
	TStringManager::String8Format( szGoalName, sizeof( szGoalName ), "%s%d", a_szGoalName, a_iIndex );

	return a_pThis->m_pNetwork->FindRandomGoal( TPString8( szGoalName ) );
}

// Position of the bone m_iLauncherBrowBone (the hand) of the player character
// $Barnyard: FUNCTION 004b11d0
TVector4* ACowTippingMiniGameState::GetPlayerHandPosition( TVector4* a_pOutPosition )
{
	TMatrix44 mBone;
	m_pLauncher->GetModelInstance()->GetBoneTransform( m_iLauncherBrowBone, mBone );

	*a_pOutPosition = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	return a_pOutPosition;
}

// The enemy a projectile at a_pPosition flying along a_pDirection hits (or, while m_aPlayers[].aPowerUps[ 2 ].fTime > 0,
// the enemy with the lowest iNumTargeting)
// $Barnyard: FUNCTION 004b1270
Enemy* ACowTippingMiniGameState::FindHitEnemy( const TVector4* a_pDirection, const TVector4* a_pPosition, TFLOAT* a_pOutDistance, TFLOAT a_fRadius, TFLOAT a_fMaxDistance, TBOOL a_bUnk )
{
	T2GenericSList& rEnemies     = As<T2GenericSList>( m_llActiveEnemies );
	Enemy*          pResult      = TNULL;
	TFLOAT          fMaxDistance = a_fMaxDistance;
	TINT            iLowestUnk40 = 100;

	for ( auto pNode = rEnemies.Begin(); pNode != rEnemies.End(); pNode = pNode->Next() )
	{
		Enemy* pEnemy = TREINTERPRETCAST( Enemy*, pNode );
		TINT   iType  = pEnemy->eType;
		TINT   iLimit = ( iType >= Enemy::ENEMYTYPE_COYOTE ) ? 1 : m_aiUnitMaxHits[ iType ];
		TBOOL  bFull  = iLimit <= pEnemy->iHits;
		TFLOAT fUnk10 = m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime;

		if ( ( !a_bUnk && pEnemy->iNumTargeting > m_iAmmoPerTarget && fUnk10 > 0.0f ) ||
		     ( iType == Enemy::ENEMYTYPE_COYOTE && bFull ) ||
		     ( !a_bUnk && fUnk10 <= 0.0f && ( !IsUnitNotYukking( AsInt( pEnemy ) ) || bFull ) ) )
			continue;

		TFLOAT fDistance = fMaxDistance;

		if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime <= 0.0f )
		{
			TVector4 vEnemy;
			GetUnitTargetPos( &vEnemy, AsInt( pEnemy ) );

			if ( IsTargetInAim( const_cast<TVector4*>( a_pDirection ), const_cast<TFLOAT*>( &a_pPosition->x ), &vEnemy.x, AsInt( a_fRadius ), &fDistance, fMaxDistance, std::bit_cast<TFLOAT>( TINT( a_bUnk ) ) ) )
			{
				if ( IsUnitNotYukking( AsInt( pEnemy ) ) && pEnemy->iProjectilesInFlight <= 0 && bFull )
				{
					fMaxDistance    = fDistance;
					*a_pOutDistance = fDistance;
				}
				else
				{
					if ( fMaxDistance != a_fMaxDistance || pResult )
						continue;

					*a_pOutDistance = fDistance;
				}

				pResult = pEnemy;
			}
		}
		else if ( iLowestUnk40 > pEnemy->iNumTargeting )
		{
			*a_pOutDistance = fMaxDistance;
			iLowestUnk40    = pEnemy->iNumTargeting;
			pResult         = pEnemy;
		}
	}

	return pResult;
}

// The unit of the m_llActiveRaccoons list a projectile at a_pPosition flying along a_pDirection hits
// $Barnyard: FUNCTION 004b1440
void* ACowTippingMiniGameState::FindHitUnk13268( const TVector4* a_pDirection, const TVector4* a_pPosition, TFLOAT* a_pOutDistance, TFLOAT a_fRadius, TFLOAT a_fMaxDistance, TBOOL a_bUnk )
{
	T2GenericSList& rList   = As<T2GenericSList>( m_llActiveRaccoons );
	Raccoon*        pResult = TNULL;

	for ( auto pIt = rList.Begin(); pIt != rList.End(); pIt = pIt->Next() )
	{
		Raccoon* pNode = TSTATICCAST( Raccoon, pIt );

		TVector4 vPosition;
		GetUnit13268TargetPos( AsInt( &vPosition ), TREINTERPRETCAST( void**, pNode ) );

		if ( pNode->iState != PopUpUnit::POPUPSTATE_DRAWING && pNode->iState != PopUpUnit::POPUPSTATE_ACTIVE && pNode->iState != PopUpUnit::POPUPSTATE_TAUNTING )
			continue;

		if ( !a_bUnk && pNode->bTargeted && m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime > 0.0f )
			continue;

		TFLOAT fDistance = 10.0f;

		if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime <= 0.0f &&
		     !IsTargetInAim( const_cast<TVector4*>( a_pDirection ), const_cast<TFLOAT*>( &a_pPosition->x ), &vPosition.x, AsInt( a_fRadius ), &fDistance, a_fMaxDistance, std::bit_cast<TFLOAT>( TINT( a_bUnk ) ) ) )
			continue;

		AAnimatableObject* pObject = pNode->pUnit->GetAnimatableObject();

		if ( IsLargeObjectStage( this ) )
		{
			if ( pObject->IsAnimPlaying( TPS8( yuk ), 0.0f ) ||
			     pObject->IsAnimationPlanned( TPS8( yuk ) ) ||
			     pObject->IsAnimPlaying( TPS8( yukobject ), 0.0f ) ||
			     pObject->IsAnimationPlanned( TPS8( yukobject ) ) )
				continue;
		}
		else
		{
			if ( pObject->IsAnimPlaying( TPS8( yuk ), 0.0f ) ||
			     pObject->IsAnimationPlanned( TPS8( yuk ) ) )
				continue;
		}

		a_fMaxDistance  = fDistance * 1.1f;
		*a_pOutDistance = a_fMaxDistance;
		pResult         = pNode;
	}

	return pResult;
}

// Near miss of the units of the m_llActiveRaccoons list: sets *a_pOutDistance to a_fMaxDistance when one is close to the path
// $Barnyard: FUNCTION 004b2d10
void ACowTippingMiniGameState::CheckNearMissUnk13268( const TVector4* a_pPosition, TFLOAT* a_pOutDistance, TFLOAT a_fMaxDistanceXZ, TFLOAT a_fMaxDistance )
{
	T2GenericSList& rList = As<T2GenericSList>( m_llActiveRaccoons );

	for ( auto pIt = rList.Begin(); pIt != rList.End(); pIt = pIt->Next() )
	{
		Raccoon* pNode = TSTATICCAST( Raccoon, pIt );

		TVector4 vPosition;
		GetUnit13268TargetPos( AsInt( &vPosition ), TREINTERPRETCAST( void**, pNode ) );

		const TVector4& rcOrigin = m_vCameraPos;

		TVector4 vDirection;
		vDirection.x = a_pPosition->x - rcOrigin.x;
		vDirection.y = a_pPosition->y - rcOrigin.y;
		vDirection.z = a_pPosition->z - rcOrigin.z;
		vDirection.w = a_pPosition->w;
		vDirection.Normalise();

		TVector4 vOffset;
		vOffset.Multiply( vDirection, m_fRaccoonSplatOffset );

		TVector4 vEnd;
		vEnd.x = vOffset.x + a_pPosition->x;
		vEnd.y = vOffset.y + a_pPosition->y;
		vEnd.z = vOffset.z + a_pPosition->z;
		vEnd.w = a_pPosition->w;

		TVector4 vBack;
		vBack.Multiply( vDirection, 2.0f );

		TVector4 vStart;
		vStart.x = a_pPosition->x - vBack.x;
		vStart.y = a_pPosition->y - vBack.y;
		vStart.z = a_pPosition->z - vBack.z;
		vStart.w = a_pPosition->w;

		TFLOAT fT;
		if ( TMath::Sqrt( TMath::DistancePointToSegmentSq( vPosition, vStart, vEnd, fT ) ) < a_fMaxDistance )
		{
			TVector4 vHalf;
			vHalf.Multiply( vDirection, 0.5f );

			TVector4 vMiddle = *a_pPosition;
			vMiddle.x += vHalf.x;
			vMiddle.y += vHalf.y;
			vMiddle.z += vHalf.z;

			if ( TVector4::DistanceXZ( vMiddle, vPosition ) < a_fMaxDistanceXZ )
			{
				AAnimatableObject* pObject = pNode->pUnit->GetAnimatableObject();

				if ( IsLargeObjectStage( this ) )
				{
					if ( !pObject->IsAnimPlaying( TPS8( yuk ), 0.0f ) &&
					     !pObject->IsAnimationPlanned( TPS8( yuk ) ) &&
					     !pObject->IsAnimPlaying( TPS8( yukobject ), 0.0f ) &&
					     !pObject->IsAnimationPlanned( TPS8( yukobject ) ) )
						*a_pOutDistance = a_fMaxDistance;
				}
				else
				{
					if ( !pObject->IsAnimPlaying( TPS8( yuk ), 0.0f ) &&
					     !pObject->IsAnimationPlanned( TPS8( yuk ) ) )
						*a_pOutDistance = a_fMaxDistance;
				}
			}
		}
	}
}

// Gives a_iPoints (doubled while m_aPlayers[].aPowerUps[ 4 ].fTime > 0) to the current player, shown above the cow
// $Barnyard: FUNCTION 004b4690
TINT ACowTippingMiniGameState::AwardCowPoints( Cow* a_pCow, TINT a_iPoints )
{
	// NOTE: retail reads the singleton (00782efc) here, which is this
	if ( a_iPoints > 0 && m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 4 ].fTime > 0.0f )
		a_iPoints *= 2;

	TVector4 vPosition = GetModelPosition( a_pCow->pObject );
	vPosition.y -= 4.0f;

	TVector4 vOffset;
	vOffset.Multiply( m_vRailDirection, 0.6f );

	vPosition.x -= vOffset.x;
	vPosition.y -= vOffset.y;
	vPosition.z -= vOffset.z;

	TINT iScore = m_aPlayers[ m_iCurrentPlayer ].iScore;

	if ( iScore + a_iPoints < 0 )
		a_iPoints = -iScore;

	ShowScoreFloat( &vPosition, a_iPoints, 0 );

	PlayerData* pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
	pPlayer->iScore += a_iPoints;

	if ( pPlayer->iScore < 0 )
		pPlayer->iScore = 0;

	SetScore( pPlayer->iScore );

	m_iScoreTotal += a_iPoints;
	return m_iScoreTotal;
}

// A projectile hit the world: splat
// $Barnyard: FUNCTION 004b4810
TINT ACowTippingMiniGameState::SplatOnWorld( Projectile* a_pProjectile, const TVector4* a_pNormal )
{
	TVector4 vPosition = GetModelPosition( a_pProjectile->pObject );

	if ( TVector4::DistanceSq( vPosition, m_vCameraBasePos ) > 0.5f )
		vPosition.y -= 0.1f;

	PlaySplatEffect( &vPosition, AsInt( a_pNormal ) );
	return ASoundManager::GetSingleton()->PlayCueAtPosition( m_iSplatSound, vPosition );
}

// Two projectiles collided in the air (stages of type 2/3)
// $Barnyard: FUNCTION 004b48e0
void ACowTippingMiniGameState::SplatProjectilesCollided( Projectile* a_pProjectile, Projectile* a_pOther )
{
	TVector4 vPosition      = GetModelPosition( a_pProjectile->pObject );
	TVector4 vOtherPosition = GetModelPosition( a_pOther->pObject );

	const TVector4& rcCamera = m_vCameraBasePos;

	TVector4 vDirection;
	vDirection.x = vPosition.x - rcCamera.x;
	vDirection.y = vPosition.y - rcCamera.y;
	vDirection.z = vPosition.z - rcCamera.z;
	vDirection.w = vPosition.w;

	if ( vDirection.NormaliseMag() > 0.5f )
		vPosition.y -= 0.1f;

	PlaySplatEffect( &vPosition, AsInt( &vDirection ) );
	ASoundManager::GetSingleton()->PlayCueAtPosition( m_iSplatSound, vPosition );
	ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::DAGBOSS_BARREL_BANG, vPosition );
	AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, vOtherPosition, TVector4::VEC_NEGY );
	AParticleEffectManager::GetSingleton()->PlayEffect( m_iStompCrateEffect, vOtherPosition, TVector4::VEC_NEGY );
}

// A projectile hit an enemy: splat and (every 2-4 seconds) a sound of the enemy
// $Barnyard: FUNCTION 004b4a90
void ACowTippingMiniGameState::SplatOnEnemy( Projectile* a_pProjectile, Enemy* a_pEnemy )
{
	TVector4 vPosition = GetModelPosition( a_pProjectile->pObject );

	const TVector4& rcCamera = m_vCameraBasePos;

	TVector4 vDirection;
	vDirection.x = vPosition.x - rcCamera.x;
	vDirection.y = vPosition.y - rcCamera.y;
	vDirection.z = vPosition.z - rcCamera.z;
	vDirection.w = vPosition.w;

	if ( vDirection.NormaliseMag() > 0.5f )
		vPosition.y -= 0.1f;

	PlaySplatEffect( &vPosition, AsInt( &vDirection ) );

	if ( a_pEnemy->eState != Enemy::ENEMYSTATE_RETURNING && m_fUnk_13340 < 0.0f )
	{
		PlayEnemySound( a_pEnemy, 360, 364, 376 );
		m_fUnk_13340 = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 2.0f, 4.0f );
	}
}

// A projectile hit a cow
// $Barnyard: FUNCTION 004bb220
void ACowTippingMiniGameState::OnCowHit( Projectile* a_pProjectile, Cow* a_pCow, TFLOAT a_fDistance, TINT a_iList, COWHIT a_iHitType )
{
	a_pProjectile->bHit = TTRUE;

	TVector4 vPoint1;
	TVector4 vPoint2;
	TVector4 vPoint3;
	TVector4 vPoint4;
	GetCowHitPoints( a_pCow, &vPoint1, &vPoint2, &vPoint3, &vPoint4 );

	AAnimatableObject* pObject = a_pCow->pObject;

	if ( !pObject->IsAnimPlaying( TPS8( backhit ), 0.0f ) &&
	     !pObject->IsAnimPlaying( TPS8( fronthit ), 0.0f ) &&
	     !pObject->IsAnimPlaying( TPS8( headhit ), 0.0f ) )
	{
		ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::COWTIP_MOO, vPoint1 );
	}

	pObject->KillAllAnimations();

	if ( a_iHitType == COWHIT_BACK )
		pObject->PlayAnimation( TPS8( backhit ) );
	else if ( a_iHitType == COWHIT_FRONT )
		pObject->PlayAnimation( TPS8( fronthit ) );
	else if ( a_iHitType == COWHIT_HEAD )
		pObject->PlayAnimation( TPS8( headhit ) );

	// Enemies pushing the cow stop
	Enemy* apEnemies[ 2 ];
	apEnemies[ 0 ] = FindClosestEnemy( &vPoint1, m_fCowSearchRadius );
	apEnemies[ 1 ] = FindClosestEnemy( &vPoint2, m_fCowSearchRadius );

	TBOOL bStoppedPushing = TFALSE;

	for ( TINT i = 0; i < 2; i++ )
	{
		Enemy* pEnemy = apEnemies[ i ];

		if ( pEnemy )
		{
			AAnimatedUnit* pUnit = pEnemy->pUnit;

			if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( push_start ), 0.0f ) ||
			     pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( push_start ) ) ||
			     pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( push_lp ), 0.0f ) ||
			     pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( push_lp ) ) ||
			     pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( push_end ), 0.0f ) ||
			     pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( push_end ) ) )
			{
				pUnit->GetAnimatableObject()->KillAllAnimations();
				pUnit->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );

				apEnemies[ i ]->eState    = Enemy::ENEMYSTATE_ACTIVE;
				apEnemies[ i ]->fWaitTime = 0;
				bStoppedPushing           = TTRUE;
			}
		}
	}

	if ( bStoppedPushing )
		AwardCowPoints( a_pCow, -m_iCowHitPenaltyPushing );
	else
		AwardCowPoints( a_pCow, -m_iCowHitPenalty );

	a_pCow->fHitTime = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( m_fCowTimerMin, m_fCowTimerMax );
}

// $Barnyard: FUNCTION 004bb4b0
TBOOL ACowTippingMiniGameState::TryHitCow( TINT a_iList, const TVector4* a_pPosition, Projectile* a_pProjectile, Cow* a_pCow, TFLOAT a_fDistance, COWHIT a_iHitType )
{
	if ( a_fDistance >= m_fCowHitDistance )
		return TFALSE;

	OnCowHit( a_pProjectile, a_pCow, a_fDistance, a_iList, a_iHitType );
	PlayProjectileSplat( TREINTERPRETCAST( void**, a_pProjectile ), AsInt( a_pCow ) );
	FreeProjectile( a_pProjectile, a_iList );
	return TTRUE;
}

// Velocity to throw from a_pFrom to land at the enemy
// NOTE: retail is a static function that reads the singleton (00782efc), which is this
// $Barnyard: FUNCTION 004bbfb0
TVector4* ACowTippingMiniGameState::CalculateThrowVelocityToEnemy( TVector4* a_pOutVelocity, const TVector4* a_pFrom, Enemy* a_pEnemy )
{
	TVector4 vEnemy;
	GetUnitTargetPos( &vEnemy, AsInt( a_pEnemy ) );

	// NOTE: GetThrowSpeed is inlined in retail
	TVector4 vVelocity;
	AProjectile::CalculateThrowVelocity( vEnemy, *a_pFrom, GetThrowSpeed(), vVelocity );

	*a_pOutVelocity = vVelocity;
	return a_pOutVelocity;
}

// Where the enemy goes after it was hit (a goal or a point of its cow) and the direction it faces there
// $Barnyard: FUNCTION 004bc090
TVector4* ACowTippingMiniGameState::GetEnemyRetreatPosition( TVector4* a_pOutPosition, Enemy* a_pEnemy, TVector4* a_pOutDirection )
{
	AAnimatedUnit* pUnit = a_pEnemy->pUnit;

	if ( a_pEnemy->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		GetClosestGoalPosition( a_pOutPosition, a_pEnemy->iCoyoteGoal, "coyote", pUnit->GetSteer()->GetPosition() );
		return a_pOutPosition;
	}

	Cow* pCow = &m_aCows[ a_pEnemy->iCow ];

	// NOTE: stays uninitialised in retail when iCowSide isn't 0 or 1 (outside of stages of type 2/3)
	TVector4 vPosition;

	if ( a_pEnemy->iCowSide == 0 || IsLargeObjectStage( this ) )
	{
		if ( a_pEnemy->bUnk4F )
		{
			AInteriorGoal* pGoal = FindRandomGoal( this, a_pEnemy->iCow % 3 + 1, "coyoteattack" );
			pGoal->GetPosition( vPosition );

			TVector4 vPlayer = GetModelPosition( As<AAnimatedUnit*>( m_pOtis )->GetAnimatableObject() );

			*a_pOutDirection   = vPlayer - vPosition;
			a_pOutDirection->y = 0.0f;
			a_pOutDirection->Normalise();
		}
		else
		{
			vPosition        = pCow->vTipSpot1;
			*a_pOutDirection = pCow->vTipDirection1;
		}
	}
	else if ( a_pEnemy->iCowSide == 1 )
	{
		FUN_004a7510( pCow, &vPosition, a_pOutDirection );
	}

	*a_pOutPosition = vPosition;
	return a_pOutPosition;
}

// Moves a projectile by one step: homing on its target (list 0) or on m_vPlayerCentre, gravity, spin and the ground shadow
// NOTE: retail is a method of Projectile that reads the singleton (00782efc), which is this
// $Barnyard: FUNCTION 004bcf20
TBOOL ACowTippingMiniGameState::UpdateProjectile( Projectile* a_pProjectile, TFLOAT a_fDeltaTime, TINT a_iList )
{
	TVector4 vPosition = GetModelPosition( a_pProjectile->pObject );

	if ( a_iList != 1 )
	{
		TVector4 vDestination = m_vPlayerCentre;

		if ( a_pProjectile->pTarget && !a_pProjectile->pTarget->IsVisible() )
			a_pProjectile->pTarget = TNULL;

		if ( TVector4::Distance( vDestination, vPosition ) >= 1.5f )
		{
			AAnimatedUnit* pTarget = a_pProjectile->pTarget;

			if ( pTarget )
			{
				TVector4 vOldVelocity = a_pProjectile->vVelocity;
				TVector4 vVelocity    = a_pProjectile->vVelocity;

				TVector4 vTarget;

				if ( a_pProjectile->iTargetBone < 0 )
				{
					vTarget = GetModelPosition( pTarget->GetAnimatableObject() );
				}
				else
				{
					TMatrix44 mBone;
					pTarget->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( a_pProjectile->iTargetBone, mBone );

					vTarget = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
					vTarget.y -= 0.3f;
				}

				// NOTE: in retail vSteerVelocity stays uninitialised when the steer is an AInteriorSteer in MODE_STILL,
				// but it's still passed to AimAtMovingTarget below
				TVector4 vSteerVelocity;
				TVector4 vTargetVelocity;
				ASteer*  pSteer = pTarget->GetSteer();

				if ( !pSteer->IsA( &TGetClass( AInteriorSteer ) ) || TSTATICCAST( AInteriorSteer, pSteer )->GetMode() == AInteriorSteer::MODE_STILL )
				{
					vTargetVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
				}
				else
				{
					vSteerVelocity  = pSteer->GetVelocity();
					vTargetVelocity = vSteerVelocity;
				}

				if ( a_pProjectile->bRecalcVelocity )
				{
					TVector4 vTemp;
					vVelocity = *CalculateThrowVelocityToTarget( &vTemp, &vPosition );
					vVelocity.x *= 0.75f;
					vVelocity.z *= 0.75f;
					vVelocity.y -= 0.1f;

					a_pProjectile->vVelocity       = vVelocity;
					a_pProjectile->bRecalcVelocity = TFALSE;
				}
				else if ( pTarget->GetSteer()->IsA( &TGetClass( AInteriorSteer ) ) )
				{
					// Leads the target
					AimAtMovingTarget( &vTarget, &vSteerVelocity, &vPosition, &vSteerVelocity, &vTarget );
				}

				TVector4 vToTarget = vTarget - vPosition;

				TFLOAT fMinSpeed = vTargetVelocity.AsVector3().Magnitude();

				if ( GetThrowSpeed() * 0.7f > fMinSpeed )
					fMinSpeed = GetThrowSpeed() * 0.7f;

				if ( a_pProjectile->vVelocity.y > 0.0f )
				{
					TFLOAT fSpeed = vVelocity.AsVector3().Magnitude() + ( m_fTrajectoryGravity + 0.3f ) * a_fDeltaTime;

					if ( fSpeed < fMinSpeed )
						fSpeed = ( m_fTrajectoryGravity + 0.3f ) * a_fDeltaTime + fMinSpeed;

					TFLOAT fDistance = vToTarget.NormaliseMag();

					vVelocity = vToTarget;
					vVelocity.Multiply( fSpeed );

					fSpeed = vVelocity.AsVector3().Magnitude();
					fDistance *= 1.1f;

					// Don't overshoot the target
					if ( fSpeed * a_fDeltaTime > fDistance )
					{
						if ( fSpeed > 0.0f )
							vVelocity.Divide( fSpeed );

						vVelocity.Multiply( fDistance );
					}
				}
				else
				{
					TFLOAT fSpeed = vVelocity.MagnitudeXZ() + a_fDeltaTime * 0.3f;

					if ( fSpeed < fMinSpeed )
						fSpeed = ( m_fTrajectoryGravity + 0.3f ) * a_fDeltaTime + fMinSpeed;

					vToTarget.Normalise();
					vToTarget.Multiply( fSpeed );

					vVelocity.x = vToTarget.x;
					vVelocity.z = vToTarget.z;
					vVelocity.y = a_fDeltaTime * m_fTrajectoryGravity + vVelocity.y;
				}

				// Turn faster when close to the target
				TFLOAT fDistance = TVector4::Distance( vTarget, vPosition );

				if ( fDistance == 0.0f )
					fDistance = 0.001f;

				TFLOAT fTurn = 1.0f / fDistance;
				TMath::Clip( fTurn, 0.2f, 1.0f );

				a_pProjectile->vVelocity.x = TMath::LERP( vOldVelocity.x, vVelocity.x, fTurn );
				a_pProjectile->vVelocity.y = vVelocity.y;
				a_pProjectile->vVelocity.z = TMath::LERP( vOldVelocity.z, vVelocity.z, fTurn );

				TFLOAT fSpeedXZ = a_pProjectile->vVelocity.MagnitudeXZ();

				if ( fSpeedXZ < fMinSpeed )
				{
					TFLOAT fNewSpeed = ( m_fTrajectoryGravity + 0.3f ) * a_fDeltaTime + fMinSpeed;
					TFLOAT fScale    = 1.0f / fSpeedXZ;

					a_pProjectile->vVelocity.x = fScale * fNewSpeed * a_pProjectile->vVelocity.x;
					a_pProjectile->vVelocity.z = fScale * a_pProjectile->vVelocity.z * fNewSpeed;
				}
			}
			else
			{
				if ( a_pProjectile->bRecalcVelocity )
				{
					Enemy*  pEnemy = a_pProjectile->pEnemy;
					ASteer* pSteer;

					if ( pEnemy &&
					     pEnemy->pUnit->IsVisible() &&
					     ( pSteer = pEnemy->pUnit->GetSteer(), pSteer->IsA( &TGetClass( AInteriorSteer ) ) ) )
					{
						AInteriorSteer* pInteriorSteer = TSTATICCAST( AInteriorSteer, pSteer );

						if ( pInteriorSteer->GetMode() == AInteriorSteer::MODE_STILL )
						{
							TVector4 vTemp;
							a_pProjectile->vVelocity = *CalculateThrowVelocityToEnemy( &vTemp, &vPosition, pEnemy );
						}
						else
						{
							TVector4 vTarget;
							AimAtPathFollower( pInteriorSteer, &vPosition, &a_pProjectile->vVelocity, &vTarget );
						}
					}
					else
					{
						TVector4 vTemp;
						a_pProjectile->vVelocity = *CalculateThrowVelocityToTarget( &vTemp, &vPosition );
					}

					a_pProjectile->bRecalcVelocity = TFALSE;
				}

				a_pProjectile->vVelocity.y = a_fDeltaTime * m_fTrajectoryGravity + a_pProjectile->vVelocity.y;
			}
		}
		else
		{
			// Arrived at m_vPlayerCentre: thrown again towards m_vReticleTarget with the speed it would have to reach m_vAimTarget
			TVector4 vTemp;
			a_pProjectile->vVelocity = *CalculateThrowVelocityToTarget( &vTemp, &vPosition );

			TFLOAT fSpeed = a_pProjectile->vVelocity.AsVector3().Magnitude();

			a_pProjectile->vVelocity = *CalculateThrowVelocityToTarget2( this, &vTemp, &vPosition );
			a_pProjectile->vVelocity.Normalise();
			a_pProjectile->vVelocity.Multiply( fSpeed );
			a_pProjectile->bRecalcVelocity = TTRUE;
		}
	}

	TVector4 vMove;
	vMove.Multiply( a_pProjectile->vVelocity, a_fDeltaTime );

	TVector4 vNewPosition;
	vNewPosition.x = vMove.x + vPosition.x;
	vNewPosition.y = vMove.y + vPosition.y;
	vNewPosition.z = vMove.z + vPosition.z;
	vNewPosition.w = vMove.w; // NOTE: an uninitialised local in retail, unused

	if ( a_pProjectile->pTarget )
	{
		// Keep homing projectiles below the ceiling
		TFLOAT fHeight   = m_fProjectileCeilingHeight;
		TFLOAT fDistance = fHeight;

		if ( ATerrainInterface::GetSingleton()->CollideRay( vNewPosition, TVector4::VEC_POSY, fDistance, TCollisionCommon::TOSHICGROUPFLAG( 0 ), TFALSE ) )
			vNewPosition.y -= fHeight - fDistance;
	}

	a_pProjectile->pObject->GetModelInstance()->SetTranslation( vNewPosition.AsVector3() );

	// Spin
	TVector4 vUp;
	a_pProjectile->pObject->GetModelInstance()->GetUpVector( vUp );
	vUp.AsVector3().RotateZ( a_fDeltaTime * 2.3f );
	vUp.Normalise();
	a_pProjectile->pObject->GetModelInstance()->SetDirection( vUp );

	TVector4 vShadowPosition;
	TINT     iShadowAlpha;
	TVector3 vShadowRotation;
	TVector4 vShadowNormal;

	return ATerrainInterface::GetSingleton()->GetGroundShadowInfo( vNewPosition, -0.1f, vShadowPosition, iShadowAlpha, vShadowRotation, vShadowNormal, 0.5f, 100.0f, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );
}

// The enemy was hit: it stops and walks away (to the goal from GetEnemyRetreatPosition), or SendUnitBackToStart if it's carrying something
// $Barnyard: FUNCTION 004bf770
void ACowTippingMiniGameState::MakeEnemyRetreat( Enemy* a_pEnemy )
{
	AAnimatedUnit* pUnit = a_pEnemy->pUnit;

	if ( m_iCarryBone == -1 || !AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pUnit->GetAnimatableObject(), m_iCarryBone, 0 ) )
	{
		ASteer* pSteer = pUnit->GetSteer();
		pSteer->SetInteracting( 0 );

		TVector4 vDirection = *pSteer->GetDirection();
		vDirection.y        = 0.0f;
		vDirection.Normalise();
		pSteer->SetDirection( vDirection );
		pSteer->Command_CancelCurrentCommand();

		TVector4 vGoal;
		TVector4 vGoalDirection;
		GetEnemyRetreatPosition( &vGoal, a_pEnemy, &vGoalDirection );

		TINT iArea = m_pNetwork->FindContainingArea( vGoal );
		pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

		if ( iArea >= 0 )
		{
			a_pEnemy->eState = Enemy::ENEMYSTATE_ACTIVE;
			TSTATICCAST( AInteriorSteer, pSteer )->SeekPointInArea( vGoal, iArea );
			MakeUnitRunAway( AsInt( a_pEnemy ), 0.0f, 0.0f, 0.0f );
			a_pEnemy->fUnk24 = m_fRetreatUnk24;

			ANamedAnimationSetRef pDefaultSet = pUnit->GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( default ) );

			if ( pUnit->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet().Get() != pDefaultSet.Get() )
				pUnit->GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( pDefaultSet );

			if ( !a_pEnemy->bUnk4F )
			{
				TUINT uiBit = IsLargeObjectStage( this ) ? a_pEnemy->iCow : a_pEnemy->iCowSide + 2 * a_pEnemy->iCow;

				if ( uiBit <= 29 )
					TREINTERPRETCAST( TUINT8*, m_auiCowSideTakenBits )
				[uiBit >> 3] |= 1 << ( uiBit & 7 );
			}
		}
	}
	else
	{
		ASteer* pSteer = pUnit->GetSteer();

		TVector4 vDirection = *pSteer->GetDirection();
		vDirection.y        = 0.0f;
		vDirection.Normalise();
		pSteer->SetDirection( vDirection );

		SendUnitBackToStart( AsInt( a_pEnemy ) );
	}
}

// Hit test against the cows
// $Barnyard: FUNCTION 004c1410
TBOOL ACowTippingMiniGameState::CheckHitCows( TINT a_iList, const TVector4* a_pPosition, Projectile* a_pProjectile )
{
	const TVector4& rcOrigin = m_vCameraPos;

	TVector4 vDirection;
	vDirection.x = a_pPosition->x - rcOrigin.x;
	vDirection.y = a_pPosition->y - rcOrigin.y;
	vDirection.z = a_pPosition->z - rcOrigin.z;
	vDirection.w = a_pPosition->w;
	vDirection.Normalise();

	TFLOAT fDistance;
	COWHIT iHitType = COWHIT_BACK; // NOTE: uninitialised in retail, only read after FindHitCow found a cow (and set it)
	Cow*   pCow     = FindHitCow( &vDirection, a_pPosition, &fDistance, m_fCowHitRadius, m_fCowHitDistance, &iHitType, TTRUE );

	return pCow && TryHitCow( a_iList, a_pPosition, a_pProjectile, pCow, fDistance, iHitType );
}

// The projectile hit the enemy a_pEnemy at a_fDistance
// $Barnyard: FUNCTION 004c14e0
TBOOL ACowTippingMiniGameState::TryHitEnemy( TINT a_iList, const TVector4* a_pPosition, Projectile* a_pProjectile, Enemy* a_pEnemy, TFLOAT a_fDistance )
{
	TVector4 vEnemy;
	GetUnitTargetPos( &vEnemy, AsInt( a_pEnemy ) );

	if ( a_pProjectile->pEnemy != a_pEnemy && !IsUnitNotYukking( AsInt( a_pEnemy ) ) )
		return TFALSE;

	if ( a_pProjectile->pEnemy == a_pEnemy )
	{
		if ( --a_pEnemy->iProjectilesInFlight <= 0 )
			a_pEnemy->eState = Enemy::ENEMYSTATE_ACTIVE;

		a_pProjectile->pEnemy = TNULL;
	}

	TVector4 vCenter = vEnemy;
	vCenter.y        = vEnemy.y - m_fEnemyHeight * 0.5f;

	if ( a_fDistance >= m_fEnemyHitDistance )
	{
		// Near miss: the enemy may leave
		TVector4 vDirection = a_pProjectile->vVelocity;
		vDirection.Normalise();

		if ( a_fDistance < m_fEnemyNearMissDistance )
		{
			TVector4 vOffset;
			vOffset.Multiply( vDirection, m_fEnemyNearMissDistance );

			TVector4 vHalfOffset;
			vHalfOffset.Multiply( vOffset, 0.5f );

			TVector4 vMiddle = vCenter + vHalfOffset;

			if ( TVector4::DistanceXZ( *a_pPosition, vMiddle ) < m_fEnemyNearMissDistance )
			{
				TVector4 vGoal;
				GetEnemyRetreatPosition( &vGoal, a_pEnemy, &vMiddle );

				if ( TVector4::DistanceXZ( vCenter, vGoal ) > m_fEnemyGoalDistanceXZ )
					MakeUnitRunAway( AsInt( a_pEnemy ), 0.0f, 0.0f, 0.0f );
			}
		}

		return TFALSE;
	}

	OnProjectileHitUnit13120( AsInt( a_pProjectile ), a_pEnemy, AsInt( a_fDistance ), a_iList );
	SplatOnEnemy( a_pProjectile, a_pEnemy );
	FreeProjectile( a_pProjectile, a_iList );
	return TTRUE;
}

// The projectile hit the unit a_pUnk (of the m_llActiveRaccoons list) at a_fDistance
// $Barnyard: FUNCTION 004c18c0
TBOOL ACowTippingMiniGameState::TryHitUnk13268( TINT a_iList, const TVector4* a_pPosition, Projectile* a_pProjectile, void* a_pUnk, TFLOAT a_fDistance )
{
	TVector4 vPosition;
	GetUnit13268TargetPos( AsInt( &vPosition ), TREINTERPRETCAST( void**, a_pUnk ) );

	TVector4 vCenter = vPosition;
	vCenter.y        = vPosition.y - m_fEnemyHeight * 0.5f;

	if ( a_fDistance >= m_fRaccoonHitDistance )
	{
		TVector4 vDirection = a_pProjectile->vVelocity;
		vDirection.Normalise();

		if ( a_fDistance < m_fRaccoonNearMissDistance )
		{
			TVector4 vOffset;
			vOffset.Multiply( vDirection, m_fRaccoonNearMissDistance );

			TVector4 vHalfOffset;
			vHalfOffset.Multiply( vOffset, 0.5f );

			TVector4 vMiddle = vCenter + vHalfOffset;

			// NOTE: retail computes this distance and ignores it
			TVector4::DistanceXZ( *a_pPosition, vMiddle );
		}

		return TFALSE;
	}

	OnProjectileHitUnit13268( AsInt( a_pProjectile ), a_pUnk, AsInt( a_fDistance ), a_iList );
	PlayProjectileSplat( TREINTERPRETCAST( void**, a_pProjectile ), AsInt( a_pUnk ) );
	FreeProjectile( a_pProjectile, a_iList );
	return TTRUE;
}

// Hit test against the enemies
// $Barnyard: FUNCTION 004c30a0
TBOOL ACowTippingMiniGameState::CheckHitEnemies( TINT a_iList, const TVector4* a_pPosition, Projectile* a_pProjectile )
{
	const TVector4& rcOrigin = m_vCameraPos;

	TVector4 vDirection;
	vDirection.x = a_pPosition->x - rcOrigin.x;
	vDirection.y = a_pPosition->y - rcOrigin.y;
	vDirection.z = a_pPosition->z - rcOrigin.z;
	vDirection.w = a_pPosition->w;
	vDirection.Normalise();

	TFLOAT fDistance;
	Enemy* pEnemy = FindHitEnemy( &vDirection, a_pPosition, &fDistance, m_fEnemyHitRadius, m_fEnemyHitDistance, TTRUE );

	return pEnemy && TryHitEnemy( a_iList, a_pPosition, a_pProjectile, pEnemy, fDistance );
}

// Hit test against the units of the m_llActiveRaccoons list
// $Barnyard: FUNCTION 004c3160
TBOOL ACowTippingMiniGameState::CheckHitUnk13268( TINT a_iList, const TVector4* a_pPosition, Projectile* a_pProjectile )
{
	const TVector4& rcOrigin = m_vCameraPos;

	TVector4 vDirection;
	vDirection.x = a_pPosition->x - rcOrigin.x;
	vDirection.y = a_pPosition->y - rcOrigin.y;
	vDirection.z = a_pPosition->z - rcOrigin.z;
	vDirection.w = a_pPosition->w;
	vDirection.Normalise();

	TFLOAT fDistance;
	void*  pHit = FindHitUnk13268( &vDirection, a_pPosition, &fDistance, m_fRaccoonHitRadius, m_fRaccoonHitDistance, TTRUE );

	if ( pHit && TryHitUnk13268( a_iList, a_pPosition, a_pProjectile, pHit, fDistance ) )
		return TTRUE;

	CheckNearMissUnk13268( a_pPosition, &fDistance, m_fRaccoonNearMissDistanceXZ, m_fRaccoonNearMissDistance );
	return TFALSE;
}

// Fixed step (0.02 s) of the projectiles. List 0: thrown by the player, list 1: thrown at the player.
// $Barnyard: FUNCTION 004c39e0
void ACowTippingMiniGameState::UpdateProjectiles( TFLOAT a_fDeltaTime )
{
	for ( TINT iList = 0; iList < 2; iList++ )
	{
		T2SList<Projectile>& rList = m_aProjectileLists[ iList ].llHeld;

		for ( auto it = rList.Begin(); it != rList.End(); )
		{
			Projectile* pProjectile = it;
			it                      = it->Next();

			if ( iList == 1 )
			{
				if ( IsZero3( pProjectile->vVelocity ) )
					continue;

				pProjectile->pObject->GetModelInstance()->GetPosition( pProjectile->vLastPosition );

				// Home on the player's hand
				TVector4 vHand;
				GetPlayerHandPosition( &vHand );

				TFLOAT fSpeedXZ = pProjectile->vVelocity.MagnitudeXZ();

				TVector4 vDirection;
				vDirection.x = vHand.x - pProjectile->vLastPosition.x;
				vDirection.z = vHand.z - pProjectile->vLastPosition.z;
				NormaliseXZ( vDirection );
				vDirection.Multiply( fSpeedXZ );

				pProjectile->vVelocity.x = vDirection.x;
				pProjectile->vVelocity.z = vDirection.z;
				pProjectile->vVelocity.y = a_fDeltaTime * m_fTrajectoryGravity + pProjectile->vVelocity.y;

				UpdateProjectile( pProjectile, a_fDeltaTime, 1 );

				TVector4 vNewPosition = GetModelPosition( pProjectile->pObject );
				TFLOAT   fDistanceSq  = TMath::DistancePointToSegmentSq( vHand, pProjectile->vLastPosition.AsVector3(), vNewPosition.AsVector3() );

				if ( IsLargeObjectStage( this ) )
				{
					// Projectiles of the player can hit this one
					T2SList<Projectile>& rPlayerList = m_aProjectileLists[ 0 ].llHeld;
					TBOOL                bCollided   = TFALSE;

					for ( auto itOther = rPlayerList.Begin(); itOther != rPlayerList.End(); itOther = itOther->Next() )
					{
						Projectile* pOther = itOther;

						// NOTE: retail requires all three components of the velocity to be non-zero here
						if ( pOther == pProjectile || pOther->vVelocity.x == 0.0f || pOther->vVelocity.y == 0.0f || pOther->vVelocity.z == 0.0f )
							continue;

						TVector4 vOtherPosition = GetModelPosition( pOther->pObject );

						if ( CT_GetSegmentDistanceXZ( pProjectile->vLastPosition, vNewPosition, vOtherPosition, pOther->vLastPosition ) < m_fProjectileCollideDistance )
						{
							SplatProjectilesCollided( pOther, pProjectile );

							rList.Transfer( pProjectile, m_aProjectileLists[ 1 ].llFree );
							pProjectile->vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
							pProjectile->pObject->SetSkeletonUpdating( TFALSE, TTRUE );

							rPlayerList.Transfer( pOther, m_aProjectileLists[ 0 ].llFree );
							pOther->vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
							pOther->pObject->SetSkeletonUpdating( TFALSE, TTRUE );

							bCollided = TTRUE;
							break;
						}
					}

					if ( bCollided )
						continue;
				}

				TFLOAT fHitDistanceSq = IsLargeObjectStage( this ) ? m_fHitPlayerDistanceSqLarge : m_fHitPlayerDistanceSq;

				if ( fDistanceSq < fHitDistanceSq )
				{
					// Hit the player
					OnProjectileReachedPlayer( pProjectile );
					rList.Transfer( pProjectile, m_aProjectileLists[ 1 ].llFree );
				}
				else if ( TVector4::DistanceXZ( vNewPosition, m_vPlayerPos ) > 100.0f )
				{
					rList.Transfer( pProjectile, m_aProjectileLists[ 1 ].llFree );
				}
				else
				{
					continue;
				}

				pProjectile->vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
				pProjectile->pObject->SetSkeletonUpdating( TFALSE, TTRUE );
			}
			else if ( IsZero3( pProjectile->vVelocity ) )
			{
				// Held by the player: grows to its full size
				ResetProjectileScale( pProjectile );

				if ( pProjectile->iGrowType == 2 && pProjectile->fScale < m_fGrowType2MaxScale )
				{
					pProjectile->fScale = a_fDeltaTime * m_fGrowType2MaxScale + pProjectile->fScale;
					SetProjectileScale( pProjectile, pProjectile->fScale );
				}
				else if ( pProjectile->iGrowType == 1 && pProjectile->fScale < m_fGrowType1MaxScale )
				{
					pProjectile->fScale = a_fDeltaTime * m_fGrowType1MaxScale + pProjectile->fScale;
					SetProjectileScale( pProjectile, pProjectile->fScale );
				}
				else
				{
					pProjectile->fScale = m_fGrowType1MaxScale;
					SetProjectileScale( pProjectile, m_fGrowType1MaxScale );
				}
			}
			else
			{
				pProjectile->pObject->GetModelInstance()->GetPosition( pProjectile->vLastPosition );

				if ( pProjectile->iGrowType == 1 )
					ResetProjectileScale( pProjectile );

				UpdateProjectile( pProjectile, a_fDeltaTime, iList );

				if ( pProjectile->iGrowType == 1 )
				{
					if ( pProjectile->fScale >= m_fGrowType1MaxScale )
						pProjectile->fScale = m_fGrowType1MaxScale;
					else
						pProjectile->fScale = a_fDeltaTime * m_fGrowType1ShrinkSpeed + pProjectile->fScale;

					SetProjectileScale( pProjectile, pProjectile->fScale );
				}

				if ( pProjectile->vVelocity.y < 0.0f )
					continue;

				TVector4 vNewPosition = GetModelPosition( pProjectile->pObject );

				if ( TVector4::DistanceXZ( vNewPosition, m_vPlayerPos ) <= m_fProjectileMaxRange )
				{
					TINT iPosition = AsInt( &vNewPosition );
					TINT iProj     = AsInt( pProjectile );

					TBOOL bTestWorld = pProjectile->bHit ||
					    ( !TestProjectileVsAll( iList, &vNewPosition.x, iProj ) &&
					      ( pProjectile->iGrowType == 1 ||
					        ( !CheckHitUnk13268( iList, &vNewPosition, pProjectile ) &&
					          !CheckHitEnemies( iList, &vNewPosition, pProjectile ) &&
					          !TestProjectileVsCrates( TREINTERPRETCAST( void*, iList ), iPosition, TREINTERPRETCAST( void**, pProjectile ) ) &&
					          !TestProjectileVsCoins( iList, iPosition, iProj ) &&
					          !TestProjectileVs13270( iList, iPosition, iProj ) &&
					          !CheckHitCows( iList, &vNewPosition, pProjectile ) ) ) );

					if ( bTestWorld && !vNewPosition.IsEqual( pProjectile->vLastPosition ) )
					{
						TVector4 vDirection;
						vDirection.x = vNewPosition.x - pProjectile->vLastPosition.x;
						vDirection.y = vNewPosition.y - pProjectile->vLastPosition.y;
						vDirection.z = vNewPosition.z - pProjectile->vLastPosition.z;
						vDirection.w = vNewPosition.w;

						TFLOAT   fLength = vDirection.NormaliseMag();
						TVector4 vNormal;

						if ( SweepSphere( pProjectile->vLastPosition, vDirection, 0.2f, fLength, &fLength, &vNormal, 2048, 0 ) )
						{
							TVector4 vSplatNormal;
							vSplatNormal.x = vNormal.x + TVector4::VEC_NEGY.x;
							vSplatNormal.y = vNormal.y + TVector4::VEC_NEGY.y;
							vSplatNormal.z = vNormal.z + TVector4::VEC_NEGY.z;
							vSplatNormal.w = vNormal.w;
							vSplatNormal.Normalise();

							SplatOnWorld( pProjectile, &vSplatNormal );
						}
					}

					continue;
				}

				// Out of the area: the enemy it was thrown at (if any) gives up
				Enemy* pEnemy = pProjectile->pEnemy;

				if ( pEnemy )
				{
					AAnimatedUnit* pUnit = pEnemy->pUnit;

					if ( pUnit->IsVisible() && pEnemy->eState != Enemy::ENEMYSTATE_FADING )
					{
						if ( pEnemy->iCow == -1 )
						{
							if ( pEnemy->eType != Enemy::ENEMYTYPE_COYOTE )
							{
								pEnemy->iRunning = 0;
								pEnemy->eState   = Enemy::ENEMYSTATE_ACTIVE;
								SendUnitBackToStart( AsInt( pEnemy ) );
							}
						}
						else
						{
							pProjectile->pEnemy->iProjectilesInFlight--;
							pEnemy->iRunning = 0;
							pEnemy->eState   = Enemy::ENEMYSTATE_ACTIVE;

							if ( pUnit->GetAnimatableObject()->GetParentObject() )
								DetachUnit( AsInt( pEnemy ) );

							if ( pEnemy->iHits < m_aiUnitMaxHits[ pEnemy->eType ] )
							{
								MakeUnitRunAway( AsInt( pEnemy ), 0.0f, 0.0f, 0.0f );
								MakeEnemyRetreat( pEnemy );
							}
							else
							{
								SendUnitBackToStart( AsInt( pEnemy ) );
							}
						}
					}

					pProjectile->pEnemy = TNULL;
				}

				rList.Transfer( pProjectile, m_aProjectileLists[ iList ].llFree );
				pProjectile->vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
				pProjectile->pObject->SetSkeletonUpdating( TFALSE, TTRUE );
			}
		}
	}
}
