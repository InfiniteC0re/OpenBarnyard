#include "pch.h"
#include "ABarnYardPoolAI.h"
#include "ABarnYardPoolMiniGameState.h"
#include "Units/AUnitProxy.h"
#include "Units/AUnitPoolBall.h"
#include "Terrain/ATerrainInterface.h"

#include <Math/TMath.h>
#include <Math/TRandom.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABarnYardPoolAI::PotEntry ) == 0x20 );
TSTATICASSERT( sizeof( ABarnYardPoolAI::ShotEntry ) == 0x40 );
TSTATICASSERT( sizeof( ABarnYardPoolAI ) == 0x1db5c );

// Types of AddApproachPoints
static constexpr TINT APPROACH_POT = 1; // PotEntry: start is a ball, the points are around a pocket
static constexpr TINT APPROACH_HIT = 2; // ShotEntry: start is the cue ball, the points are around a ball

// Minimum cosine between the cue ball and the object ball directions (a cut of about 87 degrees)
static constexpr TFLOAT MIN_CUT_COSINE = 1.0f / 18.0f;

static constexpr TFLOAT NO_SHOT_DISTANCE = 99999.99f;

// T2Vector<ABarnYardPoolAI::PotEntry, 3200>::Clear
// $Barnyard: FUNCTION 004ce310
static void ClearPotEntries( T2Vector<ABarnYardPoolAI::PotEntry, ABarnYardPoolAI::MAX_NUM_POT_ENTRIES>& a_rVector )
{
	a_rVector.Clear();
}

// T2Vector<ABarnYardPoolAI::ShotEntry, 300>::Clear
// $Barnyard: FUNCTION 004ce350
static void ClearShotEntries( T2Vector<ABarnYardPoolAI::ShotEntry, ABarnYardPoolAI::MAX_NUM_SHOT_ENTRIES>& a_rVector )
{
	a_rVector.Clear();
}

// T2Vector<ABarnYardPoolAI::PotEntry, 3200>::Erase (retail clears the slot that's freed at the end)
// $Barnyard: FUNCTION 004ce620
static void ErasePotEntry( T2Vector<ABarnYardPoolAI::PotEntry, ABarnYardPoolAI::MAX_NUM_POT_ENTRIES>& a_rVector, TINT a_iIndex )
{
	a_rVector.Erase( a_rVector.Begin() + a_iIndex );
}

// T2Vector<ABarnYardPoolAI::ShotEntry, 300>::Erase (retail clears the slot that's freed at the end)
// $Barnyard: FUNCTION 004ce680
static void EraseShotEntry( T2Vector<ABarnYardPoolAI::ShotEntry, ABarnYardPoolAI::MAX_NUM_SHOT_ENTRIES>& a_rVector, TINT a_iIndex )
{
	a_rVector.Erase( a_rVector.Begin() + a_iIndex );
}

// $Barnyard: FUNCTION 004ce6e0
ABarnYardPoolAI::ABarnYardPoolAI()
{
	Reset();
}

// $Barnyard: FUNCTION 004ce400
ABarnYardPoolAI::~ABarnYardPoolAI()
{
	Reset();
}

// $Barnyard: FUNCTION 004ce390
void ABarnYardPoolAI::Reset()
{
	m_eTargetBallType    = BALLTYPE_ANY_GROUP;
	m_iNumBalls          = 0;
	m_iNumPockets        = 0;
	m_iDifficulty        = 4;
	m_iNumApproachPoints = 8;
	m_fPocketRadius      = 0.0f;
	m_fBallRadius        = 0.0f;
	m_fMaxShotPower      = 0.0f;
	m_pBalls             = TNULL;
	m_pPockets           = TNULL;
	ClearShotEntries( m_vecShotEntries );
	ClearPotEntries( m_vecPotEntries );
	m_bBallInHand = TFALSE;
	m_vecForward  = TVector4( 0.0f, 0.0f, 1.0f, 1.0f );
}

AUnitPoolBall* ABarnYardPoolAI::GetBall( TINT a_iIndex ) const
{
	return TSTATICCAST( AUnitPoolBall, m_pBalls[ a_iIndex ].Get()->GetUnit() );
}

// $Barnyard: FUNCTION 004d04a0
TVector4 ABarnYardPoolAI::GetShot( TBOOL a_bNoAimError )
{
	TVector4 vecShot = TVector4::VEC_ZERO;
	vecShot          = FindPotShot();

	if ( TVector4::VEC_ZERO.IsEqual( vecShot ) )
	{
		vecShot = FindHitShot();

		if ( TVector4::VEC_ZERO.IsEqual( vecShot ) )
			vecShot = GetFallbackShot();
	}

	if ( !a_bNoAimError )
	{
		static TRandom s_oAimRandom; // 0x00783138

		TFLOAT fAimError = 0.0f;

		switch ( m_iDifficulty )
		{
			case 0:
				fAimError = s_oAimRandom.GetFloatMinMax( -5.0f, 5.0f );
				break;
			case 1:
				fAimError = s_oAimRandom.GetFloatMinMax( -3.0f, 3.0f );
				break;
			case 2:
				fAimError = s_oAimRandom.GetFloatMinMax( -2.0f, 2.0f );
				break;
			case 3:
				fAimError = s_oAimRandom.GetFloatMinMax( -1.0f, 1.0f );
				break;
			case 4:
				fAimError = 0.0f;
				break;
		}

		TFLOAT fAngle = fAimError * 0.017453292f;

		if ( m_bBallInHand )
		{
			// Only keep the error if the shot still goes forward
			TVector4 vecRotated = vecShot;
			vecRotated.AsVector3().RotateY( fAngle );

			if ( TVector4::DotProduct3( vecRotated, m_vecForward ) >= 0.0f )
				vecShot = vecRotated;
		}
		else
		{
			vecShot.AsVector3().RotateY( fAngle );
		}
	}

	m_bBallInHand = TFALSE;
	return vecShot;
}

// $Barnyard: FUNCTION 004ce410
void ABarnYardPoolAI::AddApproachPoints( TINT a_eType, TVector4 a_vStart, TFLOAT a_fUnused, TVector4 a_vCenter, TFLOAT a_fRadius )
{
	TVector4 vecDirection( 1.0f, 0.0f, 0.0f, 1.0f );

	for ( TINT i = 0; i < m_iNumApproachPoints; i++ )
	{
		TVector4 vecOffset;
		vecOffset.Multiply( vecDirection, a_fRadius );

		TVector4 vecPoint( vecOffset.x + a_vCenter.x, vecOffset.y + a_vCenter.y, vecOffset.z + a_vCenter.z, a_vCenter.w );

		if ( a_eType == APPROACH_POT )
		{
			PotEntry oEntry;
			oEntry.vecBall   = a_vStart;
			oEntry.vecTarget = vecPoint;
			m_vecPotEntries.PushBack( oEntry );
		}
		else
		{
			// NOTE: retail leaves vecBall uninitialised here, it's never read for hit shots
			ShotEntry oEntry;
			oEntry.vecStart  = a_vStart;
			oEntry.vecGhost  = vecPoint;
			oEntry.vecTarget = a_vCenter;
			m_vecShotEntries.PushBack( oEntry );
		}

		vecDirection.AsVector3().RotateY( ( TMath::PI + TMath::PI ) / TFLOAT( m_iNumApproachPoints ) );
	}
}

void ABarnYardPoolAI::FilterShotEntries( TBOOL a_bPotShot, TFLOAT a_fBallSweepEpsilon )
{
	TVector4 vecNormal;
	TFLOAT   fStepHeight;
	TFLOAT   fPenetration;

	// Other balls in the way of the cue ball
	for ( TINT i = 0; i < m_vecShotEntries.Size(); )
	{
		ShotEntry       oEntry       = m_vecShotEntries[ i ];
		const TVector4& rcObjectBall = a_bPotShot ? oEntry.vecBall : oEntry.vecTarget;

		TVector4 vecDirection( oEntry.vecGhost.x - oEntry.vecStart.x, oEntry.vecGhost.y - oEntry.vecStart.y, oEntry.vecGhost.z - oEntry.vecStart.z, oEntry.vecGhost.w );
		TFLOAT   fDistance = vecDirection.NormaliseMag();
		TBOOL    bBlocked  = TFALSE;

		for ( TINT k = 15; k >= 0; k-- )
		{
			AUnitPoolBall* pBall = GetBall( k );

			if ( !pBall->GetPosition().IsEqual( oEntry.vecStart ) && !pBall->GetPosition().IsEqual( rcObjectBall ) &&
			     TMath::SweptSphereVsSphere( oEntry.vecStart, m_fBallRadius, vecDirection, pBall->GetPosition(), m_fBallRadius, fDistance, vecNormal, a_fBallSweepEpsilon ) )
			{
				bBlocked = TTRUE;
				break;
			}
		}

		if ( bBlocked )
			EraseShotEntry( m_vecShotEntries, i );
		else
			i++;
	}

	// The table in the way of the cue ball
	for ( TINT i = 0; i < m_vecShotEntries.Size(); )
	{
		ShotEntry oEntry = m_vecShotEntries[ i ];

		TVector4 vecDirection( oEntry.vecGhost.x - oEntry.vecStart.x, oEntry.vecGhost.y - oEntry.vecStart.y, oEntry.vecGhost.z - oEntry.vecStart.z, oEntry.vecGhost.w );
		TFLOAT   fDistance = vecDirection.NormaliseMag();

		if ( ATerrainInterface::GetSingleton()->CollideSweptSphere( oEntry.vecStart, m_fBallRadius, vecDirection, fDistance, vecNormal, 0.01f, fStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE ) )
			EraseShotEntry( m_vecShotEntries, i );
		else
			i++;
	}

	// Cuts that are too thin
	for ( TINT i = 0; i < m_vecShotEntries.Size(); )
	{
		ShotEntry oEntry = m_vecShotEntries[ i ];

		TVector4 vecDirection( oEntry.vecGhost.x - oEntry.vecStart.x, oEntry.vecGhost.y - oEntry.vecStart.y, oEntry.vecGhost.z - oEntry.vecStart.z, oEntry.vecGhost.w );
		TVector4 vecObjectDirection( oEntry.vecTarget.x - oEntry.vecGhost.x, oEntry.vecTarget.y - oEntry.vecGhost.y, oEntry.vecTarget.z - oEntry.vecGhost.z, oEntry.vecTarget.w );
		vecDirection.Normalise();
		vecObjectDirection.Normalise();

		if ( TVector4::DotProduct3( vecObjectDirection, vecDirection ) < MIN_CUT_COSINE )
			EraseShotEntry( m_vecShotEntries, i );
		else
			i++;
	}

	// Ball in hand: the shot has to go forward
	if ( m_bBallInHand == TTRUE )
	{
		for ( TINT i = 0; i < m_vecShotEntries.Size(); )
		{
			ShotEntry oEntry = m_vecShotEntries[ i ];

			TVector4 vecDirection( oEntry.vecGhost.x - oEntry.vecStart.x, oEntry.vecGhost.y - oEntry.vecStart.y, oEntry.vecGhost.z - oEntry.vecStart.z, oEntry.vecGhost.w );
			vecDirection.Normalise();

			if ( TVector4::DotProduct3( vecDirection, m_vecForward ) < 0.0f )
				EraseShotEntry( m_vecShotEntries, i );
			else
				i++;
		}
	}
}

TBOOL ABarnYardPoolAI::FindBestShotEntry( ShotEntry& a_rOutEntry )
{
	TFLOAT fBestDistance = NO_SHOT_DISTANCE;

	for ( TINT i = 0; i < m_vecShotEntries.Size(); i++ )
	{
		ShotEntry oEntry    = m_vecShotEntries[ i ];
		TFLOAT    fDistance = TVector4::Distance( oEntry.vecGhost, oEntry.vecTarget );

		if ( fDistance < fBestDistance )
		{
			fBestDistance = fDistance;
			a_rOutEntry   = oEntry;
		}
	}

	return fBestDistance < NO_SHOT_DISTANCE;
}

// $Barnyard: FUNCTION 004ce700
TVector4 ABarnYardPoolAI::FindPotShot()
{
	TVector4 vecShot = TVector4::VEC_ZERO;

	ClearShotEntries( m_vecShotEntries );
	ClearPotEntries( m_vecPotEntries );

	// Approach points around every pocket for every ball of the target group
	for ( TINT iPocket = m_iNumPockets - 1; iPocket >= 0; iPocket-- )
	{
		for ( TINT i = 15; i >= 0; i-- )
		{
			AUnitPoolBall* pBall = GetBall( i );

			if ( pBall->GetSteerState() != BALLSTATE_POCKETED )
			{
				TINT eTargetBallType = m_eTargetBallType;

				if ( pBall->GetBallType() == eTargetBallType ||
				     ( eTargetBallType == BALLTYPE_ANY_GROUP && ( pBall->GetBallType() == ABarnYardPoolMiniGameState::BALLTYPE_GROUP_B || pBall->GetBallType() == ABarnYardPoolMiniGameState::BALLTYPE_GROUP_A ) ) )
				{
					AddApproachPoints( APPROACH_POT, pBall->GetPosition(), m_fBallRadius, m_pPockets[ iPocket ], m_fPocketRadius );
				}
			}
		}
	}

	TVector4 vecNormal;
	TFLOAT   fStepHeight;
	TFLOAT   fPenetration;

	// The table in the way of the ball
	for ( TINT i = 0; i < m_vecPotEntries.Size(); )
	{
		PotEntry oEntry = m_vecPotEntries[ i ];

		TVector4 vecDirection( oEntry.vecTarget.x - oEntry.vecBall.x, oEntry.vecTarget.y - oEntry.vecBall.y, oEntry.vecTarget.z - oEntry.vecBall.z, oEntry.vecTarget.w );
		TFLOAT   fDistance = vecDirection.NormaliseMag();

		if ( ATerrainInterface::GetSingleton()->CollideSweptSphere( oEntry.vecBall, m_fBallRadius, vecDirection, fDistance, vecNormal, 0.01f, fStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE ) )
			ErasePotEntry( m_vecPotEntries, i );
		else
			i++;
	}

	// Other balls in the way of the ball
	for ( TINT i = 0; i < m_vecPotEntries.Size(); )
	{
		PotEntry oEntry = m_vecPotEntries[ i ];

		TVector4 vecDirection( oEntry.vecTarget.x - oEntry.vecBall.x, oEntry.vecTarget.y - oEntry.vecBall.y, oEntry.vecTarget.z - oEntry.vecBall.z, oEntry.vecTarget.w );
		TFLOAT   fDistance = vecDirection.NormaliseMag();
		TBOOL    bBlocked  = TFALSE;

		for ( TINT k = 15; k >= 0; k-- )
		{
			AUnitPoolBall* pBall = GetBall( k );

			if ( !pBall->GetPosition().IsEqual( oEntry.vecBall ) &&
			     TMath::SweptSphereVsSphere( oEntry.vecBall, m_fBallRadius, vecDirection, pBall->GetPosition(), m_fBallRadius, fDistance, vecNormal, 0.001f ) )
			{
				bBlocked = TTRUE;
				break;
			}
		}

		if ( bBlocked )
			ErasePotEntry( m_vecPotEntries, i );
		else
			i++;
	}

	// Turn the pots into cue ball shots at the ghost ball positions
	ShotEntry oShot;
	oShot.vecStart = m_pCueBall->GetPosition();

	for ( TINT i = 0; i < m_vecPotEntries.Size(); i++ )
	{
		PotEntry oEntry = m_vecPotEntries[ i ];

		TVector4 vecDirection( oEntry.vecBall.x - oEntry.vecTarget.x, oEntry.vecBall.y - oEntry.vecTarget.y, oEntry.vecBall.z - oEntry.vecTarget.z, oEntry.vecBall.w );
		vecDirection.Normalise();

		TVector4 vecOffset;
		vecOffset.Multiply( vecDirection, m_fBallRadius );

		TVector4 vecDiameter;
		vecDiameter.Multiply( vecOffset, 2.0f );

		oShot.vecGhost.Set( vecDiameter.x + oEntry.vecBall.x, vecDiameter.y + oEntry.vecBall.y, vecDiameter.z + oEntry.vecBall.z, oEntry.vecBall.w );
		oShot.vecTarget = oEntry.vecTarget;
		oShot.vecBall   = oEntry.vecBall;

#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: retail pushes without a capacity check, more than 300 pots overflow into the state's next member
		if ( m_vecShotEntries.Size() >= MAX_NUM_SHOT_ENTRIES )
			break;
#endif

		m_vecShotEntries.PushBack( oShot );
	}

	FilterShotEntries( TTRUE, 0.001f );

	ShotEntry oBest;
	TBOOL     bFound = FindBestShotEntry( oBest );

	vecShot       = TVector4::VEC_ZERO;
	m_vecAimPoint = m_pCueBall->GetPosition();

	if ( bFound )
	{
		vecShot.Set( oBest.vecGhost.x - oBest.vecStart.x, oBest.vecGhost.y - oBest.vecStart.y, oBest.vecGhost.z - oBest.vecStart.z, oBest.vecGhost.w );
		TVector4 vecObjectDirection( oBest.vecTarget.x - oBest.vecGhost.x, oBest.vecTarget.y - oBest.vecGhost.y, oBest.vecTarget.z - oBest.vecGhost.z, oBest.vecTarget.w );

		TFLOAT fPower = vecShot.NormaliseMag();

		TVector4 vecOffset;
		vecOffset.Multiply( vecShot, m_fBallRadius );

		TVector4 vecDiameter;
		vecDiameter.Multiply( vecOffset, 2.0f );

		m_vecAimPoint.x += vecDiameter.x;
		m_vecAimPoint.y += vecDiameter.y;
		m_vecAimPoint.z += vecDiameter.z;

		// NOTE: retail treats the angle (radians) as degrees, so the cosine is always about 1
		// and the cut barely adds any power
		TFLOAT fCutCosine = TMath::Cos( vecObjectDirection.GetAngleBetweenVector( vecShot ) * 1.5707964f * 0.011111111f );
		fPower            = TVector4::Distance( oBest.vecGhost, oBest.vecTarget ) / fCutCosine + fPower;

		TFLOAT fReference = TVector4::Distance( m_pCueBall->GetStartPosition(), m_pBlackBall->GetStartPosition() );
		fReference        = fReference + fReference;

		if ( fPower <= fReference )
			fPower = m_fMaxShotPower / fReference * fPower;
		else
			fPower = m_fMaxShotPower;

		vecShot.Multiply( fPower );
	}

	return vecShot;
}

// $Barnyard: FUNCTION 004cf8d0
TVector4 ABarnYardPoolAI::FindHitShot()
{
	TVector4 vecShot = TVector4::VEC_ZERO;

	ClearShotEntries( m_vecShotEntries );
	ClearPotEntries( m_vecPotEntries );

	// Approach points around every ball of the target group
	for ( TINT i = 15; i >= 0; i-- )
	{
		AUnitPoolBall* pBall = GetBall( i );

		if ( pBall->GetSteerState() != BALLSTATE_POCKETED && !pBall->GetPosition().IsEqual( m_pCueBall->GetPosition() ) )
		{
			TINT eTargetBallType = m_eTargetBallType;

			if ( pBall->GetBallType() == eTargetBallType ||
			     ( eTargetBallType == BALLTYPE_ANY_GROUP && ( pBall->GetBallType() == ABarnYardPoolMiniGameState::BALLTYPE_GROUP_B || pBall->GetBallType() == ABarnYardPoolMiniGameState::BALLTYPE_GROUP_A ) ) )
			{
				AddApproachPoints( APPROACH_HIT, m_pCueBall->GetPosition(), m_fBallRadius, pBall->GetPosition(), m_fBallRadius );
			}
		}
	}

	FilterShotEntries( TFALSE, 0.000001f );

	ShotEntry oBest;
	TBOOL     bFound = FindBestShotEntry( oBest );

	vecShot       = TVector4::VEC_ZERO;
	m_vecAimPoint = m_pCueBall->GetPosition();

	if ( bFound )
	{
		vecShot.Set( oBest.vecGhost.x - oBest.vecStart.x, oBest.vecGhost.y - oBest.vecStart.y, oBest.vecGhost.z - oBest.vecStart.z, oBest.vecGhost.w );
		TVector4 vecObjectDirection( oBest.vecTarget.x - oBest.vecGhost.x, oBest.vecTarget.y - oBest.vecGhost.y, oBest.vecTarget.z - oBest.vecGhost.z, oBest.vecTarget.w );

		TFLOAT fPower = vecShot.NormaliseMag();

		TVector4 vecOffset;
		vecOffset.Multiply( vecShot, m_fBallRadius );

		TVector4 vecDiameter;
		vecDiameter.Multiply( vecOffset, 2.0f );

		m_vecAimPoint.x += vecDiameter.x;
		m_vecAimPoint.y += vecDiameter.y;
		m_vecAimPoint.z += vecDiameter.z;

		// NOTE: retail computes the cut angle here too but never uses it
		vecObjectDirection.GetAngleBetweenVector( vecShot );

		TFLOAT fReference = TVector4::Distance( m_pCueBall->GetStartPosition(), m_pBlackBall->GetStartPosition() );
		fReference        = fReference + fReference;

		if ( fPower <= fReference )
			fPower = m_fMaxShotPower / fReference * fPower;
		else
			fPower = m_fMaxShotPower;

		vecShot.Multiply( fPower );
	}

	// Don't play too softly
	if ( !vecShot.IsEqual( TVector4::VEC_ZERO ) && m_fMaxShotPower * 0.05f > vecShot.Magnitude() )
	{
		vecShot.Normalise();
		vecShot.Multiply( m_fMaxShotPower * 0.5f );
	}

	return vecShot;
}

// $Barnyard: FUNCTION 004ce150
TVector4 ABarnYardPoolAI::GetFallbackShot()
{
	TVector4 vecShot = TVector4::VEC_ZERO;

	// NOTE: the cue ball is shot straight away from the black, not at it
	TVector4 vecBlackBall = m_pBlackBall->GetPosition();
	TVector4 vecCueBall   = m_pCueBall->GetPosition();
	vecShot.Set( vecCueBall.x - vecBlackBall.x, vecCueBall.y - vecBlackBall.y, vecCueBall.z - vecBlackBall.z, vecCueBall.w );
	vecShot.Normalise();

	m_vecAimPoint = m_pCueBall->GetPosition();

	TVector4 vecOffset;
	vecOffset.Multiply( vecShot, m_fBallRadius );

	TVector4 vecDiameter;
	vecDiameter.Multiply( vecOffset, 2.0f );

	m_vecAimPoint.x += vecDiameter.x;
	m_vecAimPoint.y += vecDiameter.y;
	m_vecAimPoint.z += vecDiameter.z;

	vecShot.Multiply( m_fMaxShotPower * 0.5f );

	if ( m_bBallInHand == TTRUE )
	{
		vecShot = m_vecForward;
		vecShot.Normalise();
		vecShot.Multiply( m_fMaxShotPower * 0.5f );
	}

	return vecShot;
}

// $Barnyard: FUNCTION 004d0430
TBOOL ABarnYardPoolAI::IsBallOverlapping( AUnitPoolBall* a_pBall )
{
	TVector4 vecPosition = a_pBall->GetPosition();

	for ( TINT i = 15; i >= 0; i-- )
	{
		AUnitPoolBall* pBall = GetBall( i );

		if ( pBall != a_pBall && TMath::SphereVsSphere( pBall->GetPosition(), m_fBallRadius, vecPosition, m_fBallRadius ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004d0720
void ABarnYardPoolAI::PlaceCueBall( TVector4 a_vCenter, TFLOAT a_fCueBallSize, TFLOAT a_fAreaRadius, TVector4 a_vForward )
{
	TVector4 vecSide = a_vForward;
	vecSide.AsVector3().RotateY( TMath::PI * 0.5f );

	TVector4 vecPlacement = TVector4::VEC_ZERO;

	TVector4 vecForwardOffset;
	vecForwardOffset.Multiply( a_vForward, 0.0001f );

	TVector4 vecSideOffset;
	vecSideOffset.Multiply( vecSide, -( a_fAreaRadius - 0.0001f ) );

	// Grid corner: one side of the area, just behind the center
	TVector4 vecGridStart(
	    vecSideOffset.x + a_vCenter.x - vecForwardOffset.x,
	    vecSideOffset.y + a_vCenter.y - vecForwardOffset.y,
	    vecSideOffset.z + a_vCenter.z - vecForwardOffset.z,
	    a_vCenter.w
	);

	TFLOAT fStep       = a_fCueBallSize * 0.5f;
	TINT   iNumRows    = TINT( a_fAreaRadius / fStep + 2.0f );
	TINT   iNumColumns = iNumRows / 2 + 2;

	m_bBallInHand = TTRUE;

	for ( TINT i = 0; i < iNumRows; i++ )
	{
		for ( TINT j = 0; j < iNumColumns; j++ )
		{
			TVector4 vecForwardSteps;
			vecForwardSteps.Multiply( a_vForward, TFLOAT( j ) + TFLOAT( j ) );

			TVector4 vecForwardStep;
			vecForwardStep.Multiply( vecForwardSteps, fStep );

			TVector4 vecSideSteps;
			vecSideSteps.Multiply( vecSide, TFLOAT( i ) + TFLOAT( i ) );

			TVector4 vecSideStep;
			vecSideStep.Multiply( vecSideSteps, fStep );

			TVector4 vecPoint(
			    vecSideStep.x + vecGridStart.x - vecForwardStep.x,
			    vecSideStep.y + vecGridStart.y - vecForwardStep.y,
			    vecSideStep.z + vecGridStart.z - vecForwardStep.z,
			    vecGridStart.w
			);

			// NOTE: retail only uses the result, the clamped point is ignored
			TVector4 vecClamped;
			if ( ClampToBallInHandArea( vecPoint, a_vCenter, a_fAreaRadius, a_vForward, &vecClamped ) )
			{
				TVector4 vecCueBall( vecPoint.x, m_pCueBall->GetStartPosition().y, vecPoint.z, vecPoint.w );
				m_pCueBall->SetPosition( vecCueBall );

				if ( !IsBallOverlapping( m_pCueBall ) )
				{
					vecPlacement = vecCueBall;

					// Done once there's something to hit from here
					if ( !FindHitShot().IsEqual( TVector4::VEC_ZERO ) )
						return;
				}
			}
		}
	}

	// NOTE: with no free spot at all, retail puts the cue ball on the black ball's spawn position
	if ( vecPlacement.IsEqual( TVector4::VEC_ZERO ) )
		vecPlacement = m_pBlackBall->GetStartPosition();

	m_pCueBall->SetPosition( vecPlacement );
}

// $Barnyard: FUNCTION 004d1ca0
TBOOL ABarnYardPoolAI::ClampToBallInHandArea( TVector4 a_vPoint, TVector4 a_vCenter, TFLOAT a_fRadius, TVector4 a_vForward, TVector4* a_pOutPoint )
{
	TVector4 vecDirection( a_vPoint.x - a_vCenter.x, 0.0f, a_vPoint.z - a_vCenter.z, a_vPoint.w );
	TBOOL    bInside = TTRUE;

	TFLOAT fDistance    = vecDirection.NormaliseMag();
	TFLOAT fDot         = TVector4::DotProduct3( vecDirection, a_vForward );
	TFLOAT fMaxDistance = ( TMath::Abs( fDot ) * -0.35000002f + 1.0f ) * a_fRadius;

	if ( fDistance > fMaxDistance || fDot > 0.0f )
	{
		bInside = TFALSE;

		if ( fDot > 0.0f )
		{
			// In front of the center: project onto the side line
			TVector4 vecSide = a_vForward;

			if ( vecDirection.z * a_vForward.x - a_vForward.z * vecDirection.x <= 0.0f )
				vecSide.AsVector3().RotateY( TMath::PI * 0.5f );
			else
				vecSide.AsVector3().RotateY( -( TMath::PI * 0.5f ) );

			fDistance    = TVector4::DotProduct3( vecSide, vecDirection ) * fDistance;
			vecDirection = vecSide;
		}
	}

	TMath::Clip( fDistance, 0.0f, fMaxDistance );

	TVector4 vecOffset;
	vecOffset.Multiply( vecDirection, fDistance );

	a_pOutPoint->Set( vecOffset.x + a_vCenter.x, vecOffset.y + a_vPoint.y, vecOffset.z + a_vCenter.z, a_vCenter.w );
	return bInside;
}
