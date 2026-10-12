#include "pch.h"
#include "ASpline.h"
#include "Assets/AAssetLoader.h"

#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: inlined into both loaders
void ASpline::SetData( const Data* a_pData )
{
	m_iNumSegments = a_pData->iNumSegments;
	m_pSegments    = a_pData->pSegments;

	TFree( m_pfSegmentEnds );
	m_pfSegmentEnds = TSTATICCAST( TFLOAT, TMalloc( sizeof( TFLOAT ) * m_iNumSegments ) );

	CalculateLengths();
}

// $Barnyard: FUNCTION 0066d9d0
void ASpline::Load( TTRB& a_rTRB, const TCHAR* a_szSymbol )
{
	SetData( TSTATICCAST( Data, a_rTRB.GetSymbolAddress( a_szSymbol ) ) );
}

// $Barnyard: FUNCTION 0066d980
void ASpline::Load( const TCHAR* a_szFileName, const TCHAR* a_szSymbol, TUINT a_eAssetType )
{
	SetData( TSTATICCAST( Data, AAssetLoader::GetSymbolAddress( a_szFileName, a_szSymbol, a_eAssetType ) ) );
}

// $Barnyard: FUNCTION 0066d810
void ASpline::CalculateLengths()
{
	constexpr TINT NUM_STEPS = 12;

	TFLOAT fTotalLength = 0.0f;

	for ( TINT i = 0; i < m_iNumSegments; i++ )
	{
		TFLOAT fT            = 0.0f;
		m_pfSegmentEnds[ i ] = 0.0f;

		for ( TINT k = NUM_STEPS; k != 0; k-- )
		{
			TVector4 vStart = m_pSegments[ i ].GetPoint( fT );
			fT += 1.0f / NUM_STEPS;
			TVector4 vEnd = m_pSegments[ i ].GetPoint( fT );

			m_pfSegmentEnds[ i ] += TVector4::Distance( vStart, vEnd );
		}

		fTotalLength += m_pfSegmentEnds[ i ];
	}

	// Normalise and accumulate
	TFLOAT fInvTotalLength = 1.0f / fTotalLength;

	for ( TINT i = 0; i < m_iNumSegments; i++ )
	{
		m_pfSegmentEnds[ i ] = fInvTotalLength * m_pfSegmentEnds[ i ];

		if ( i > 0 )
			m_pfSegmentEnds[ i ] = m_pfSegmentEnds[ i - 1 ] + m_pfSegmentEnds[ i ];
	}

	m_fLength = 0.0f;

	for ( TINT i = m_iNumSegments; i != 0; i-- )
		m_fLength += m_pSegments[ i - 1 ].m_fLength;
}

// $Barnyard: FUNCTION 0066d760
void ASpline::GetSegment( TFLOAT a_fProgress, TINT& a_rOutSegment, TFLOAT& a_rOutT ) const
{
	a_rOutSegment = 0;
	a_rOutT       = 0.0f;

	if ( a_fProgress < 1.0f )
	{
		if ( a_fProgress > 0.0f )
		{
			for ( TINT i = 0; i < m_iNumSegments; i++ )
			{
				if ( a_fProgress <= m_pfSegmentEnds[ i ] )
				{
					TFLOAT fStart = GetSegmentStart( i );

					a_rOutSegment = i;
					a_rOutT       = ( a_fProgress - fStart ) / ( m_pfSegmentEnds[ i ] - fStart );
					return;
				}
			}
		}
	}
	else
	{
		a_rOutSegment = m_iNumSegments - 1;
		a_rOutT       = 1.0f;
	}
}

// $Barnyard: FUNCTION 0066da20
void ASpline::GetPosition( TFLOAT a_fProgress, TVector4& a_rOutPosition ) const
{
	TINT   iSegment;
	TFLOAT fT;
	GetSegment( a_fProgress, iSegment, fT );

	a_rOutPosition = m_pSegments[ iSegment ].GetPoint( fT );
}

// $Barnyard: FUNCTION 0066da80
void ASpline::GetDirection( TFLOAT a_fProgress, TVector4& a_rOutDirection ) const
{
	TINT   iSegment;
	TFLOAT fT;
	GetSegment( a_fProgress, iSegment, fT );

	if ( iSegment == 0 && fT == 0.0f )
	{
		a_rOutDirection = m_pSegments[ 0 ].m_vStartTangent;
		a_rOutDirection.Normalise();
		return;
	}

	if ( iSegment == m_iNumSegments - 1 && fT == 1.0f )
	{
		a_rOutDirection = m_pSegments[ m_iNumSegments - 1 ].m_vEndTangent;
		a_rOutDirection.Normalise();
		return;
	}

	TVector4 vPosition = m_pSegments[ iSegment ].GetPoint( fT );
	fT += 0.01f;

	if ( fT <= 1.0f )
	{
		a_rOutDirection = m_pSegments[ iSegment ].GetPoint( fT );
	}
	else if ( iSegment + 1 < m_iNumSegments - 1 )
	{
		a_rOutDirection = m_pSegments[ iSegment + 1 ].GetPoint( fT - 1.0f );
	}
	else
	{
		// NOTE: retail subtracts the position from the end tangent (not a point) here, which
		// happens near the end of the second to last segment
		a_rOutDirection = m_pSegments[ m_iNumSegments - 1 ].m_vEndTangent;
	}

	a_rOutDirection.x -= vPosition.x;
	a_rOutDirection.y -= vPosition.y;
	a_rOutDirection.z -= vPosition.z;
	a_rOutDirection.Normalise();
}

// $Barnyard: FUNCTION 0066dc30
void ASpline::GetPositionAndDirection( TFLOAT a_fProgress, TVector4& a_rOutPosition, TVector4& a_rOutDirection ) const
{
	TINT   iSegment;
	TFLOAT fT;
	GetSegment( a_fProgress, iSegment, fT );

	a_rOutPosition = m_pSegments[ iSegment ].GetPoint( fT );
	GetDirection( a_fProgress, a_rOutDirection );
}

// $Barnyard: FUNCTION 0066dca0
TFLOAT ASpline::FindClosestProgress( const TVector4& a_rcPoint, TFLOAT a_fMaxDistance ) const
{
	constexpr TINT NUM_ITERATIONS = 10;

	TFLOAT fClosestDistance = a_fMaxDistance * a_fMaxDistance;
	TINT   iClosest         = -1;

	if ( m_iNumSegments <= 0 )
		return -1.0f;

	// Find the segment whose chord is the closest to the point
	for ( TINT i = 0; i < m_iNumSegments; i++ )
	{
		// NOTE: retail also calculates the closest point of the chord and doesn't use it
		TFLOAT fT;
		TFLOAT fDistance = TMath::DistancePointToSegmentSq( a_rcPoint, m_pSegments[ i ].m_vStart, m_pSegments[ i ].m_vEnd, fT );

		if ( fDistance < fClosestDistance )
		{
			iClosest         = i;
			fClosestDistance = fDistance;
		}
	}

	if ( iClosest < 0 )
		return -1.0f;

	TFLOAT fEnd      = m_pfSegmentEnds[ iClosest ];
	TFLOAT fStart    = GetSegmentStart( iClosest );
	TFLOAT fProgress = m_pSegments[ iClosest ].FindClosestT( a_rcPoint, NUM_ITERATIONS ) * ( fEnd - fStart ) + fStart;

	TVector4 vPosition;
	TVector4 vOtherPosition;

	// Check the previous segment
	if ( iClosest > 0 )
	{
		fEnd   = m_pfSegmentEnds[ iClosest - 1 ];
		fStart = GetSegmentStart( iClosest - 1 );

		TFLOAT fOtherProgress = m_pSegments[ iClosest - 1 ].FindClosestT( a_rcPoint, NUM_ITERATIONS ) * ( fEnd - fStart ) + fStart;

		GetPosition( fProgress, vPosition );
		GetPosition( fOtherProgress, vOtherPosition );

		TFLOAT fDistance = TVector4::DistanceSq( vPosition, a_rcPoint );

		if ( TVector4::DistanceSq( vOtherPosition, a_rcPoint ) < fDistance )
			fProgress = fOtherProgress;
	}

	// Check the next segment
	if ( iClosest < m_iNumSegments - 1 )
	{
		fEnd   = m_pfSegmentEnds[ iClosest + 1 ];
		fStart = GetSegmentStart( iClosest + 1 );

		TFLOAT fOtherProgress = m_pSegments[ iClosest + 1 ].FindClosestT( a_rcPoint, NUM_ITERATIONS ) * ( fEnd - fStart ) + fStart;

		GetPosition( fProgress, vPosition );
		GetPosition( fOtherProgress, vOtherPosition );

		TFLOAT fDistance = TVector4::DistanceSq( vPosition, a_rcPoint );

		if ( TVector4::DistanceSq( vOtherPosition, a_rcPoint ) < fDistance )
			fProgress = fOtherProgress;
	}

	return fProgress;
}

// $Barnyard: FUNCTION 0066df40
TVector4 ASpline::GetClosestPosition( const TVector4& a_rcPoint, TFLOAT a_fMaxDistance ) const
{
	TFLOAT fProgress = FindClosestProgress( a_rcPoint, a_fMaxDistance );

	if ( fProgress <= 0.0f )
		return TVector4::VEC_ZERO;

	TVector4 vPosition;
	GetPosition( fProgress, vPosition );

	return vPosition;
}

// $Barnyard: FUNCTION 0066dfe0
void ASpline::GetClosestPositionAndDirection( const TVector4& a_rcPoint, TVector4& a_rOutPosition, TVector4& a_rOutDirection, TFLOAT& a_rOutProgress, TFLOAT a_fMaxDistance ) const
{
	a_rOutProgress = FindClosestProgress( a_rcPoint, a_fMaxDistance );

	if ( a_rOutProgress <= 0.0f )
	{
		a_rOutProgress = 0.0f;
		a_rOutPosition = TVector4::VEC_ZERO;
	}
	else
	{
		GetPosition( a_rOutProgress, a_rOutPosition );
	}

	GetDirection( a_rOutProgress, a_rOutDirection );
}
