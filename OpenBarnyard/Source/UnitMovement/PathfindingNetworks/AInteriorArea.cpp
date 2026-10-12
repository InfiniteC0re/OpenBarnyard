#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0054fbe0
void AInteriorArea::GetPosition( Toshi::TVector4& a_rOutPosition ) const
{
	GetPositionXZ( a_rOutPosition );
}

// $Barnyard: FUNCTION 0054fc40
void AInteriorArea::GetPositionXZ( Toshi::TVector4& a_rOutPosition ) const
{
	a_rOutPosition = Toshi::TVector4( m_fOriginX, 0.0f, m_fOriginZ, 1.0f );

	m_pNetwork->ResolvePointHeight( m_iIndex, a_rOutPosition, 0 );
}

// $Barnyard: FUNCTION 0054fc90
TBOOL AInteriorArea::CheckPointInsideArea( const Toshi::TVector4& a_rcPoint ) const
{
	// We ignore point height for this check and use Y as Z coordinate on purpose
	const TFLOAT fPointX = a_rcPoint.x;
	const TFLOAT fPointZ = a_rcPoint.y;

	// Fast AABB check
	if ( fPointX <= m_fMinX || fPointX >= m_fMaxX ||
	     fPointZ <= m_fMinZ || fPointZ >= m_fMaxZ )
	{
		return TFALSE;
	}

	if ( m_iNumArcBounds < 3 ) return TFALSE;

	// Detailed check if point is in polygon
	TBOOL                     bInside    = TFALSE;
	const AInteriorArcBounds* pPrevBound = m_pArcBounds + m_iNumArcBounds - 1;
	TBOOL                     bPrevAbove = fPointZ <= pPrevBound->m_fZ;

	for ( TINT i = 0; i < m_iNumArcBounds; i++ )
	{
		const AInteriorArcBounds* pCurrBound = &m_pArcBounds[ i ];
		const TBOOL               bCurrAbove = fPointZ <= pCurrBound->m_fZ;

		if ( bPrevAbove == bCurrAbove )
		{
			if ( pPrevBound->m_fZ == fPointZ && pCurrBound->m_fZ == fPointZ )
			{
				const AInteriorArcBounds* pCheckAgainst = pCurrBound;

				if ( pPrevBound->m_fX <= pCurrBound->m_fX )
				{
					pCheckAgainst = pPrevBound;
				}
				if ( pPrevBound->m_fX < pCurrBound->m_fX )
				{
					pPrevBound = pCurrBound;
				}

				if ( pCheckAgainst->m_fX <= fPointX && fPointX <= pPrevBound->m_fX )
				{
					return TTRUE;
				}
			}
		}
		else
		{
			TFLOAT fDot = ( ( fPointX - pPrevBound->m_fX ) * pPrevBound->m_fNormalX + ( fPointZ - pPrevBound->m_fZ ) * pPrevBound->m_fNormalZ );

			if ( ( fDot >= 0.0f ) != bCurrAbove )
			{
				bInside = !bInside;
			}
		}

		pPrevBound = pCurrBound;
		bPrevAbove = bCurrAbove;
	}

	return bInside;
}

// $Barnyard: FUNCTION 0054fde0
TBOOL AInteriorArea::CheckPositionInsideArea( const Toshi::TVector4& a_rcPosition ) const
{
	// CheckPointInsideArea takes the XZ position packed into x and y
	Toshi::TVector4 vecPointXZ;
	vecPointXZ.x = a_rcPosition.x;
	vecPointXZ.y = a_rcPosition.z;

	return CheckPointInsideArea( vecPointXZ );
}

// $Barnyard: FUNCTION 0054fb90
AInteriorAreaLink* AInteriorArea::FindLink( TINT a_iAreaIndex ) const
{
	for ( TINT i = 0; i < m_iNumLinks; i++ )
	{
		if ( m_pLinks[ i ].m_iAreaIndex == a_iAreaIndex )
			return &m_pLinks[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005507a0
TFLOAT AInteriorArea::GetSpecialHeight() const
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_SPECIAL_GROUP_AREA ) )
		return 0.0f;

	return m_pNetwork->GetSpecialAreaHeight( m_iIndex );
}

// $Barnyard: FUNCTION 0054fc70
const Toshi::TVector4& AInteriorArea::GetSpecialPoint( TINT a_iPoint ) const
{
	return m_pNetwork->GetSpecialAreaPoint( m_iIndex, a_iPoint );
}

// $Barnyard: FUNCTION 0054fe10
TBOOL AInteriorArea::IsCircleInsideArea( const Toshi::TVector4& a_rcPoint, TFLOAT a_fRadius, TBOOL a_bOnlyOuterEdges ) const
{
	if ( !CheckPointInsideArea( a_rcPoint ) )
		return TFALSE;

	Toshi::TVector2 vecCenter( a_rcPoint.x, a_rcPoint.y );

	for ( TINT i = 0; i < m_iNumArcBounds; i++ )
	{
		if ( a_bOnlyOuterEdges && m_pArcBounds[ i ].m_iNeighborArea != -1 )
			continue;

		const AInteriorArcBounds& rcStart = m_pArcBounds[ i ];
		const AInteriorArcBounds& rcEnd   = m_pArcBounds[ ( i + 1 ) % m_iNumArcBounds ];

		// Touches the edge
		if ( Toshi::TMath::CircleVsSegment2D( Toshi::TVector2( rcStart.m_fX, rcStart.m_fZ ), Toshi::TVector2( rcEnd.m_fX, rcEnd.m_fZ ), a_fRadius, vecCenter, TNULL ) )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0054fea0
TBOOL AInteriorArea::IsCircleAtPositionInsideArea( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TBOOL a_bOnlyOuterEdges ) const
{
	Toshi::TVector4 vecPointXZ;
	vecPointXZ.x = a_rcPosition.x;
	vecPointXZ.y = a_rcPosition.z;

	return IsCircleInsideArea( vecPointXZ, a_fRadius, a_bOnlyOuterEdges );
}

// $Barnyard: FUNCTION 0054fbc0
TBOOL AInteriorArea::IsSpecialArea( TINT a_iUnused ) const
{
	return m_pNetwork->m_pINetwork->m_iNumNormalAreas <= m_iIndex;
}

// $Barnyard: FUNCTION 0054fed0
void AInteriorArea::GetRandomPointXZ( Toshi::TVector2& a_rOutPoint, TFLOAT a_fUnused ) const
{
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	// Start from a random edge
	TINT   iNext      = rRandom.GetInt( m_iNumArcBounds ) + 1;
	TFLOAT fAlongEdge = rRandom.GetFloat();

	const TVector2 vecOrigin( m_fOriginX, m_fOriginZ );

	TFLOAT fDistToPrevEdge = 0.0f;
	TFLOAT fDistToNextEdge = 0.0f;
	TINT   iNumChecked     = 0;

	while ( TTRUE )
	{
		TINT iCurrent   = iNext % m_iNumArcBounds;
		iNext           = iCurrent + 1;
		TINT iNextBound = iNext % m_iNumArcBounds;

		if ( iNumChecked == m_iNumArcBounds )
			break;

		iNumChecked++;

		const AInteriorArcBounds& rcBound   = m_pArcBounds[ iCurrent ];
		TFLOAT                    fDistance = ( m_fOriginX - rcBound.m_fX ) * rcBound.m_fNormalX + ( m_fOriginZ - rcBound.m_fZ ) * rcBound.m_fNormalZ;

		// The edge has to be far enough from the origin
		if ( fDistance >= 0.5f )
		{
			TFLOAT fMinT = 0.5f / fDistance;

			// A random point of the edge moved towards the origin, at least 0.5 units away from the edge
			a_rOutPoint.Lerp( rcBound.GetPoint(), m_pArcBounds[ iNextBound ].GetPoint(), fAlongEdge );
			a_rOutPoint.Lerp( a_rOutPoint, vecOrigin, TFLOAT( rRandom.RandRaw() >> 1 ) * ( 1.0f - fMinT ) * 4.6566129E-10f + fMinT );

			// It also has to be 0.5 units away from both of the neighbour edges
			TINT                      iPrev       = ( iCurrent == 0 ? m_iNumArcBounds : iCurrent ) - 1;
			const AInteriorArcBounds& rcPrevBound = m_pArcBounds[ iPrev ];
			const AInteriorArcBounds& rcNextBound = m_pArcBounds[ iNextBound ];

			fDistToPrevEdge = ( a_rOutPoint.y - rcBound.m_fZ ) * rcPrevBound.m_fNormalZ + ( a_rOutPoint.x - rcBound.m_fX ) * rcPrevBound.m_fNormalX;
			fDistToNextEdge = ( a_rOutPoint.y - rcNextBound.m_fZ ) * rcNextBound.m_fNormalZ + ( a_rOutPoint.x - rcNextBound.m_fX ) * rcNextBound.m_fNormalX;
		}

		if ( fDistToPrevEdge >= 0.5f && fDistToNextEdge >= 0.5f )
			return;
	}

	// Every edge was checked, use the origin
	a_rOutPoint = vecOrigin;
}

// $Barnyard: FUNCTION 005500e0
void AInteriorArea::GetRandomPosition( Toshi::TVector4& a_rOutPosition, TFLOAT a_fUnused ) const
{
	TVector2 vecPoint;
	GetRandomPointXZ( vecPoint, a_fUnused );

	a_rOutPosition.x = vecPoint.x;
	a_rOutPosition.y = 0.0f;
	a_rOutPosition.z = vecPoint.y;
	a_rOutPosition.w = 1.0f;

	m_pNetwork->ResolvePointHeight( m_iIndex, a_rOutPosition, 0 );
}

// $Barnyard: FUNCTION 00550130
void AInteriorArea::CollideMove( AInteriorMoveQuery& a_rQuery, TUINT32* a_pVisitedAreas ) const
{
	a_pVisitedAreas[ TUINT32( m_iIndex ) >> 5 ] |= 1 << ( m_iIndex & 31 );

	for ( TINT i = 0; i < m_iNumArcBounds; i++ )
	{
		const AInteriorArcBounds& rcBound     = m_pArcBounds[ i ];
		const AInteriorArcBounds& rcNextBound = m_pArcBounds[ ( i + 1 ) % m_iNumArcBounds ];

		// NOTE: retail calculates the direction of the edge but never uses it
		TVector2 vecEdgeDir( rcNextBound.m_fX - rcBound.m_fX, rcNextBound.m_fZ - rcBound.m_fZ );
		vecEdgeDir.Multiply( 1.0f / sqrtf( vecEdgeDir.MagnitudeSq() ) );

		TVector2 vecMove( a_rQuery.m_vTarget.x - a_rQuery.m_vStart.x, a_rQuery.m_vTarget.y - a_rQuery.m_vStart.y );

		// Does the circle get close to the edge?
		if ( !TMath::IsSegmentNearSegment2D( rcBound.GetPoint(), rcNextBound.GetPoint(), a_rQuery.m_vStart, a_rQuery.m_vTarget, a_rQuery.m_fRadius ) )
			continue;

		const TVector2 vecStart  = a_rQuery.m_vStart;
		const TINT     iNeighbor = rcBound.m_iNeighborArea;

		if ( iNeighbor == -1 || HASANYFLAG( m_pNetwork->m_pINetwork->m_pAreas[ iNeighbor ].m_eFlags, FLAGS_NON_ROUTABLE ) )
		{
			// An outer edge, stop the circle if it moves against it
			TFLOAT fDot = vecMove.y * rcBound.m_fNormalZ + vecMove.x * rcBound.m_fNormalX;

			if ( fDot >= 0.0f )
				continue;

			// Only if no edge was hit yet or this one stops the movement more than the one that was hit
			TBOOL bNotHitYet = 0.0f < a_rQuery.m_fHitDistance;

			if ( !( a_rQuery.m_fHitDistance == 0.0f && fDot < vecMove.y * a_rQuery.m_vHitNormal.y + vecMove.x * a_rQuery.m_vHitNormal.x ) && !bNotHitYet )
				continue;

			a_rQuery.m_vHitPosition = a_rQuery.m_vStart;
			a_rQuery.m_bHit         = TTRUE;
			a_rQuery.m_bSlide       = TTRUE;
			a_rQuery.m_fHitDistance = 0.0f;
			a_rQuery.m_vHitNormal   = rcBound.GetNormal();
			a_rQuery.m_vHitNormal.Normalize();
			a_rQuery.m_vSlideNormal = a_rQuery.m_vHitNormal;

			// 0x007840c8
			static TFLOAT s_fCornerDistanceScale = 4.0f;

			// Bend the slide normal towards the normal of a corner the circle is close to
			TFLOAT fDistanceSq1 = ( rcBound.m_fX - vecStart.x ) * ( rcBound.m_fX - vecStart.x ) + ( rcBound.m_fZ - vecStart.y ) * ( rcBound.m_fZ - vecStart.y );
			TFLOAT fDistanceSq2 = ( rcNextBound.m_fX - vecStart.x ) * ( rcNextBound.m_fX - vecStart.x ) + ( rcNextBound.m_fZ - vecStart.y ) * ( rcNextBound.m_fZ - vecStart.y );
			TFLOAT fMaxDistance = a_rQuery.m_fRadius * a_rQuery.m_fRadius * s_fCornerDistanceScale;

			const AInteriorArcBounds* pCorner;
			TFLOAT                    fCornerDistanceSq;

			if ( fDistanceSq1 < fMaxDistance && fDistanceSq1 < fDistanceSq2 )
			{
				pCorner           = &rcBound;
				fCornerDistanceSq = fDistanceSq1;
			}
			else if ( fDistanceSq2 < fMaxDistance && fDistanceSq2 < fDistanceSq1 )
			{
				pCorner           = &rcNextBound;
				fCornerDistanceSq = fDistanceSq2;
			}
			else
			{
				continue;
			}

			TFLOAT fT = sqrtf( fCornerDistanceSq ) / a_rQuery.m_fRadius - 1.0f;

			if ( fT < 0.0f )
				fT = 0.0f;

			a_rQuery.m_vSlideNormal.Lerp( rcBound.GetNormal(), pCorner->GetCornerNormal(), 1.0f - fT );
			a_rQuery.m_vSlideNormal.Normalize();
		}
		else
		{
			// A linked edge, record the crossing and continue in the neighbour
			const AInteriorArea& rcNeighbor = m_pNetwork->m_pINetwork->m_pAreas[ iNeighbor ];
			TBOOL                bVisited   = ( a_pVisitedAreas[ TUINT32( rcNeighbor.m_iIndex ) >> 5 ] & ( 1 << ( rcNeighbor.m_iIndex & 31 ) ) ) != 0;

			if ( vecMove.y * rcBound.m_fNormalZ + vecMove.x * rcBound.m_fNormalX < 0.0f && iNeighbor != a_rQuery.m_iArea )
			{
				TVector2 vecCrossing;

				if ( TMath::SegmentVsSegment2D( rcBound.GetPoint(), rcNextBound.GetPoint(), a_rQuery.m_vStart, a_rQuery.m_vTarget, vecCrossing ) )
					a_rQuery.AddCrossing( sqrtf( TVector2::DistanceSq( a_rQuery.m_vStart, vecCrossing ) ), iNeighbor );
			}

			if ( !bVisited )
				rcNeighbor.CollideMove( a_rQuery, a_pVisitedAreas );
		}
	}
}

// $Barnyard: FUNCTION 005504f0
TBOOL AInteriorArea::ClipLineToCorners( const Toshi::TVector2& a_rcFrom, const Toshi::TVector2& a_rcTo, TFLOAT a_fRadius, Toshi::TVector2& a_rOutEnd, AInteriorPathFunnel* a_pFunnel ) const
{
	a_rOutEnd = a_rcTo;

	TBOOL bClear = TTRUE;

	if ( !HASANYFLAG( m_eFlags, FLAGS_CHECK_CORNERS ) )
		return bClear;

	TVector2 vecDir( a_rcTo.x - a_rcFrom.x, a_rcTo.y - a_rcFrom.y );

	// Offset to the side of the line
	TVector2 vecSide( vecDir.y, -vecDir.x );
	vecSide.Normalize();
	vecSide.x *= a_fRadius;
	vecSide.y *= a_fRadius;

	TFLOAT         fLengthSq = vecDir.x * vecDir.x + vecDir.y * vecDir.y;
	const TVector2 vecOrigin( m_fOriginX, m_fOriginZ );
	TBOOL          bOriginOnRight = TVector2::IsOnRight( a_rcFrom, a_rcTo, vecOrigin );

	TINT iPrev = m_iNumArcBounds - 1;

	for ( TINT i = 0; i < m_iNumArcBounds; i++ )
	{
		const AInteriorArcBounds& rcCorner = m_pArcBounds[ i ];

		// Only the corners of the outer edges
		if ( rcCorner.m_iNeighborArea != -1 && m_pArcBounds[ iPrev ].m_iNeighborArea != -1 )
		{
			iPrev = i;
			continue;
		}

		// NOTE: retail doesn't update iPrev for the corners that aren't along the line
		TFLOAT fToCornerX = rcCorner.m_fX - a_rcFrom.x;
		TFLOAT fToCornerY = rcCorner.m_fZ - a_rcFrom.y;

		if ( fToCornerX * fToCornerX + fToCornerY * fToCornerY > fLengthSq )
			continue;

		if ( fToCornerY * vecDir.y + fToCornerX * vecDir.x < 0.0f )
			continue;

		TBOOL bOnRight1 = TVector2::IsOnRight( a_rcFrom, vecOrigin, rcCorner.GetPoint() );
		TBOOL bOnRight2 = TVector2::IsOnRight( vecOrigin, a_rcTo, rcCorner.GetPoint() );
		TBOOL bCornerOnRight;

		if ( bOriginOnRight )
			bCornerOnRight = bOnRight1 || bOnRight2;
		else
			bCornerOnRight = bOnRight1 && bOnRight2;

		// The point a_fRadius away from the corner
		TVector2 vecOffset = vecSide;

		if ( bCornerOnRight )
		{
			vecOffset.x = -vecOffset.x;
			vecOffset.y = -vecOffset.y;
		}

		TVector2 vecPoint( vecOffset.x + rcCorner.m_fX, vecOffset.y + rcCorner.m_fZ );

		if ( !CheckPointInsideArea( TVector4( vecPoint.x, vecPoint.y, 0.0f, 0.0f ) ) )
		{
			// Move it towards the origin instead
			vecOffset.x = m_fOriginX - rcCorner.m_fX;
			vecOffset.y = m_fOriginZ - rcCorner.m_fZ;
			vecOffset.Normalize();
			vecOffset.x *= a_fRadius;
			vecOffset.y *= a_fRadius;

			vecPoint.x = vecOffset.x + rcCorner.m_fX;
			vecPoint.y = vecOffset.y + rcCorner.m_fZ;
		}

		if ( bOriginOnRight != bCornerOnRight && bOriginOnRight == TVector2::IsOnRight( a_rcFrom, a_rcTo, vecPoint ) )
		{
			// The corner is in the way
			bClear = TFALSE;

			if ( bOriginOnRight == TVector2::IsOnRight( a_rcFrom, a_rOutEnd, vecPoint ) )
				a_rOutEnd = vecPoint;
		}

		if ( a_pFunnel )
		{
			if ( bCornerOnRight )
				a_pFunnel->NarrowEdgeBToPoint( vecPoint );
			else
				a_pFunnel->NarrowEdgeAToPoint( vecPoint );
		}

		iPrev = i;
	}

	return bClear;
}

//-----------------------------------------------------------------------------
// AInteriorPathFunnel
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0054f980
AInteriorPathFunnel& AInteriorPathFunnel::operator=( const AInteriorPathFunnel& a_rcOther )
{
	m_eState = a_rcOther.m_eState;
	m_vApex  = a_rcOther.m_vApex;
	m_vEdgeA = a_rcOther.m_vEdgeA;
	m_vEdgeB = a_rcOther.m_vEdgeB;
	return *this;
}

// $Barnyard: FUNCTION 0054f680
TBOOL AInteriorPathFunnel::IsDirectionInside( const Toshi::TVector2& a_rcDirection ) const
{
	if ( m_eState == STATE_OPEN )
		return TTRUE;

	if ( m_eState == STATE_CLOSED )
		return TFALSE;

	return m_vEdgeB.x * a_rcDirection.y - m_vEdgeB.y * a_rcDirection.x >= 0.0f &&
	    m_vEdgeA.x * a_rcDirection.y - m_vEdgeA.y * a_rcDirection.x <= 0.0f;
}

// $Barnyard: FUNCTION 0054f6e0
TBOOL AInteriorPathFunnel::IsPointInside( const Toshi::TVector2& a_rcPoint ) const
{
	TVector2 vecDirection( a_rcPoint.x - m_vApex.x, a_rcPoint.y - m_vApex.y );
	return IsDirectionInside( vecDirection );
}

// $Barnyard: FUNCTION 0054f720
void AInteriorPathFunnel::NarrowEdgeA( const Toshi::TVector2& a_rcDirection )
{
	if ( m_eState == STATE_OPEN )
	{
		m_vEdgeA = a_rcDirection;
		m_vEdgeA.Normalize();
		m_vEdgeB   = m_vEdgeA;
		m_vEdgeB.x = -m_vEdgeB.x;
		m_vEdgeB.y = -m_vEdgeB.y;
		m_eState   = STATE_NARROW;
	}
	else if ( m_eState != STATE_CLOSED )
	{
		if ( m_vEdgeA.x * a_rcDirection.y - m_vEdgeA.y * a_rcDirection.x < 0.0f )
		{
			if ( a_rcDirection.y * m_vEdgeB.x - m_vEdgeB.y * a_rcDirection.x >= 0.0f )
			{
				m_vEdgeA = a_rcDirection;
				m_vEdgeA.Normalize();
			}
			else
			{
				// The edges crossed
				m_eState = STATE_CLOSED;
			}
		}
	}
}

// $Barnyard: FUNCTION 0054f7d0
void AInteriorPathFunnel::NarrowEdgeB( const Toshi::TVector2& a_rcDirection )
{
	if ( m_eState == STATE_OPEN )
	{
		m_vEdgeB = a_rcDirection;
		m_vEdgeB.Normalize();
		m_vEdgeA   = m_vEdgeB;
		m_vEdgeA.x = -m_vEdgeA.x;
		m_vEdgeA.y = -m_vEdgeA.y;
		m_eState   = STATE_NARROW;
	}
	else if ( m_eState != STATE_CLOSED )
	{
		if ( m_vEdgeB.x * a_rcDirection.y - m_vEdgeB.y * a_rcDirection.x > 0.0f )
		{
			if ( a_rcDirection.y * m_vEdgeA.x - m_vEdgeA.y * a_rcDirection.x <= 0.0f )
			{
				m_vEdgeB = a_rcDirection;
				m_vEdgeB.Normalize();
			}
			else
			{
				// The edges crossed
				m_eState = STATE_CLOSED;
			}
		}
	}
}

// $Barnyard: FUNCTION 0054f880
void AInteriorPathFunnel::NarrowEdgeAToPoint( const Toshi::TVector2& a_rcPoint )
{
	TVector2 vecDirection( a_rcPoint.x - m_vApex.x, a_rcPoint.y - m_vApex.y );
	NarrowEdgeA( vecDirection );
}

// $Barnyard: FUNCTION 0054f8c0
void AInteriorPathFunnel::NarrowEdgeBToPoint( const Toshi::TVector2& a_rcPoint )
{
	TVector2 vecDirection( a_rcPoint.x - m_vApex.x, a_rcPoint.y - m_vApex.y );
	NarrowEdgeB( vecDirection );
}

// $Barnyard: FUNCTION 0054f900
void AInteriorPathFunnel::NarrowToPortal( const Toshi::TVector2& a_rcEdgeA, const Toshi::TVector2& a_rcEdgeB )
{
	TVector2 vecDirection( a_rcEdgeA.x - m_vApex.x, a_rcEdgeA.y - m_vApex.y );
	NarrowEdgeA( vecDirection );

	vecDirection.Set( a_rcEdgeB.x - m_vApex.x, a_rcEdgeB.y - m_vApex.y );
	NarrowEdgeB( vecDirection );
}

//-----------------------------------------------------------------------------
// AInteriorMoveQuery
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00550cf0
// $Barnyard: FUNCTION 00550c10
void AInteriorMoveQuery::Reset()
{
	m_bHit         = TFALSE;
	m_bSlide       = TTRUE;
	m_fHitDistance = TMath::MAXFLOAT;
	m_vHitPosition = m_vTarget;

	// NOTE: retail overwrites the entries with uninitialised values
	for ( TINT i = 0; i < m_iNumCrossings; i++ )
	{
		m_aCrossings[ i ].fDistance = 0.0f;
		m_aCrossings[ i ].iArea     = 0;
	}

	m_iNumCrossings = 0;
}

// $Barnyard: FUNCTION 00550d20
TINT AInteriorMoveQuery::GetEndArea() const
{
	for ( TINT i = m_iNumCrossings - 1; i >= 0; i-- )
	{
		if ( m_aCrossings[ i ].fDistance < m_fHitDistance )
			return m_aCrossings[ i ].iArea;
	}

	return m_iArea;
}

// NOTE: the crossings are a sorted array template in retail, its binary search (00550d90) and insertion (00550e40, 00550d50, 00550c50) are inlined here
// $Barnyard: FUNCTION 00550e80
// $Barnyard: FUNCTION 00550e40
// $Barnyard: FUNCTION 00550d90
// $Barnyard: FUNCTION 00550d50
// $Barnyard: FUNCTION 00550c50
void AInteriorMoveQuery::AddCrossing( TFLOAT a_fDistance, TINT a_iArea )
{
	// Find where to insert it (after the crossings at the same distance)
	TINT iLow  = 0;
	TINT iHigh = m_iNumCrossings - 1;
	TINT iInsertAt;

	while ( TTRUE )
	{
		if ( iLow > iHigh )
		{
			iInsertAt = iLow;
			break;
		}

		TINT iMiddle = ( iHigh + iLow ) / 2;
		TINT iSign   = TINT( TMath::Sign( a_fDistance - m_aCrossings[ iMiddle ].fDistance ) );

		if ( iSign < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iSign > 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			iInsertAt = iMiddle + 1;

			while ( iInsertAt <= iHigh && TINT( TMath::Sign( a_fDistance - m_aCrossings[ iInsertAt ].fDistance ) ) == 0 )
				iInsertAt++;

			break;
		}
	}

	TASSERT( m_iNumCrossings < MAX_NUM_CROSSINGS );

	// Make room for it
	for ( TINT i = m_iNumCrossings - 1; i >= iInsertAt; i-- )
		m_aCrossings[ i + 1 ] = m_aCrossings[ i ];

	m_iNumCrossings += 1;

	m_aCrossings[ iInsertAt ].fDistance = a_fDistance;
	m_aCrossings[ iInsertAt ].iArea     = a_iArea;
}
