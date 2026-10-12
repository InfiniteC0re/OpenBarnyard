#include "pch.h"
#include "ACameraSplinePath.h"

#include <Math/TMath.h>
#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00460360
ACameraSplinePath::ACameraSplinePath( TINT a_iMaxSegments )
    : m_iMaxSegments( a_iMaxSegments )
    , m_iNumSegments( 0 )
{
	m_pSegments = new ACameraSplineSegment[ m_iMaxSegments ];
	m_pfLengths = new TFLOAT[ m_iMaxSegments ];
}

// $Barnyard: FUNCTION 00460410
// $Barnyard: FUNCTION 004608e0
ACameraSplinePath::~ACameraSplinePath()
{
	delete[] m_pSegments;
	delete[] m_pfLengths;
}

// Hermite basis (0x0077cec8)
static constexpr TMatrix44 s_matHermite(
    2.0f,
    -2.0f,
    1.0f,
    1.0f,
    -3.0f,
    3.0f,
    -2.0f,
    -1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f
);

// $Barnyard: FUNCTION 006b7390
void ACameraSplineSegment::Hermite( TVector4& a_rOut, const TVector4& a_rcStart, const TVector4& a_rcStartTangent, const TVector4& a_rcEnd, const TVector4& a_rcEndTangent, TFLOAT a_fT )
{
	TFLOAT fT2 = a_fT * a_fT;

	// x: start, y: end, z: start tangent, w: end tangent
	TVector4 vWeights;
	TMatrix44::TransformVector( vWeights, s_matHermite, TVector4( a_fT * fT2, fT2, a_fT, 1.0f ) );

	TVector4 vEndTangent;
	vEndTangent.Multiply( a_rcEndTangent, vWeights.w );

	TVector4 vStartTangent;
	vStartTangent.Multiply( a_rcStartTangent, vWeights.z );

	TVector4 vEnd;
	vEnd.Multiply( a_rcEnd, vWeights.y );

	TVector4 vStart;
	vStart.Multiply( a_rcStart, vWeights.x );

	a_rOut.x = vEndTangent.x + ( vStartTangent.x + ( vEnd.x + vStart.x ) );
	a_rOut.y = vEndTangent.y + ( vStartTangent.y + ( vStart.y + vEnd.y ) );
	a_rOut.z = vEndTangent.z + ( vStartTangent.z + ( vEnd.z + vStart.z ) );
	a_rOut.w = vStart.w;
}

// $Barnyard: FUNCTION 006cdc10
TVector4 ACameraSplineSegment::GetPoint( TFLOAT a_fT ) const
{
	TVector4 vPoint;
	Hermite( vPoint, m_vStart, m_vStartTangent, m_vEnd, m_vEndTangent, a_fT );

	return vPoint;
}

// $Barnyard: FUNCTION 006cdc60
TFLOAT ACameraSplineSegment::FindClosestT( const TVector4& a_rcPoint, TINT a_iNumIterations ) const
{
	TVector4 vLow  = m_vStart;
	TVector4 vHigh = m_vEnd;
	TFLOAT   fLow  = 0.0f;
	TFLOAT   fHigh = 1.0f;

	for ( TINT i = a_iNumIterations; i > 0; i-- )
	{
		TFLOAT fMid = ( fHigh - fLow ) * 0.5f + fLow;

		TVector4 vMid;
		Hermite( vMid, m_vStart, m_vStartTangent, m_vEnd, m_vEndTangent, fMid );

		TFLOAT fLowDistance = TMath::DistancePointToSegmentSq( a_rcPoint, vLow.AsVector3(), vMid.AsVector3() );

		if ( TMath::DistancePointToSegmentSq( a_rcPoint, vMid.AsVector3(), vHigh.AsVector3() ) <= fLowDistance )
		{
			fLow = fMid;
			vLow = vMid;
		}
		else
		{
			fHigh = fMid;
			vHigh = vMid;
		}
	}

	return fHigh;
}

// $Barnyard: FUNCTION 006b75b0
TFLOAT ACameraSplineSegment::CalculateLength( const TVector4& a_rcStart, const TVector4& a_rcStartTangent, const TVector4& a_rcEnd, const TVector4& a_rcEndTangent )
{
	TVector4 vPrevious = a_rcStart;
	TFLOAT   fLength   = 0.0f;
	TFLOAT   fT        = 0.0f;

	do
	{
		TVector4 vPoint;
		Hermite( vPoint, a_rcStart, a_rcStartTangent, a_rcEnd, a_rcEndTangent, fT );

		fLength += TVector4::Distance( vPrevious, vPoint );
		vPrevious = vPoint;
		fT += 0.1f;
	} while ( fT <= 1.0f );

	return fLength;
}

// $Barnyard: FUNCTION 006cddc0
void ACameraSplineSegment::Set( const TVector4& a_rcStart, const TVector4& a_rcStartTangent, const TVector4& a_rcEnd, const TVector4& a_rcEndTangent )
{
	m_vStart        = a_rcStart;
	m_vEnd          = a_rcEnd;
	m_vStartTangent = a_rcStartTangent;
	m_vEndTangent   = a_rcEndTangent;

	m_fLength    = CalculateLength( m_vStart, m_vStartTangent, m_vEnd, m_vEndTangent );
	m_fInvLength = 1.0f / m_fLength;
}

// $Barnyard: FUNCTION 006cde60
void ACameraSplineSegment::SetEnd( const TVector4& a_rcEnd, const TVector4& a_rcEndTangent )
{
	m_vEnd        = a_rcEnd;
	m_vEndTangent = a_rcEndTangent;

	m_fLength    = CalculateLength( m_vStart, m_vStartTangent, m_vEnd, m_vEndTangent );
	m_fInvLength = 1.0f / m_fLength;
}

// $Barnyard: FUNCTION 00460450
void ACameraSplinePath::Create( TINT a_iNumSegments, const TVector4* a_pPoints, const TVector4* a_pTangents )
{
	constexpr TINT NUM_STEPS = 12;

	m_iNumSegments = a_iNumSegments;

	for ( TINT i = 0; i < m_iNumSegments; i++ )
		m_pSegments[ i ].Set( a_pPoints[ i ], a_pTangents[ i ], a_pPoints[ i + 1 ], a_pTangents[ i + 1 ] );

	TFLOAT fTotalLength = 0.0f;

	for ( TINT i = 0; i < m_iNumSegments; i++ )
	{
		TFLOAT fT        = 0.0f;
		m_pfLengths[ i ] = 0.0f;

		for ( TINT k = NUM_STEPS; k != 0; k-- )
		{
			TVector4 vStart = m_pSegments[ i ].GetPoint( fT );
			fT += 1.0f / NUM_STEPS;
			TVector4 vEnd = m_pSegments[ i ].GetPoint( fT );

			m_pfLengths[ i ] += TVector4::Distance( vStart, vEnd );
		}

		fTotalLength += m_pfLengths[ i ];
	}

	// Normalise and accumulate
	TFLOAT fInvTotalLength = 1.0f / fTotalLength;

	for ( TINT i = 0; i < m_iNumSegments; i++ )
	{
		m_pfLengths[ i ] = fInvTotalLength * m_pfLengths[ i ];

		if ( i > 0 )
			m_pfLengths[ i ] = m_pfLengths[ i - 1 ] + m_pfLengths[ i ];
	}
}

// $Barnyard: FUNCTION 00460600
TFLOAT ACameraSplinePath::FindProgress( const TVector4& a_rcPosition, TFLOAT a_fMaxDistanceSq, TFLOAT a_fMaxHeight ) const
{
	TFLOAT fClosestDistanceSq = TMath::MAXFLOAT;
	TFLOAT fClosestHeight     = TMath::MAXFLOAT;
	TINT   iClosestSegment    = -1;
	TFLOAT fClosestT          = 0.0f;

	if ( m_iNumSegments <= 0 )
		return -1.0f;

	for ( TINT i = 0; i < m_iNumSegments; i++ )
	{
		const ACameraSplineSegment& rcSegment = m_pSegments[ i ];

		if ( a_fMaxHeight + a_rcPosition.y >= rcSegment.m_vStart.y )
		{
			// Find the closest point of the segment start-end on the XZ plane
			TVector4 vPosition( a_rcPosition.x, 0.0f, a_rcPosition.z, 1.0f );
			TVector4 vStart( rcSegment.m_vStart.x, 0.0f, rcSegment.m_vStart.z, 1.0f );
			TVector4 vEnd( rcSegment.m_vEnd.x, 0.0f, rcSegment.m_vEnd.z, 1.0f );

			TFLOAT fT;
			TFLOAT fDistanceSq = TMath::DistancePointToSegmentSq( vPosition, vStart, vEnd, fT );

			TVector4 vPoint = rcSegment.m_vEnd;
			vPoint.x -= rcSegment.m_vStart.x;
			vPoint.y -= rcSegment.m_vStart.y;
			vPoint.z -= rcSegment.m_vStart.z;
			vPoint.Multiply( fT );
			vPoint.x += rcSegment.m_vStart.x;
			vPoint.y += rcSegment.m_vStart.y;
			vPoint.z += rcSegment.m_vStart.z;

			if ( vPoint.y <= a_fMaxHeight + a_rcPosition.y )
			{
				TFLOAT fHeight = a_rcPosition.y - vPoint.y;

				if ( fDistanceSq < fClosestDistanceSq && fDistanceSq < a_fMaxDistanceSq && fClosestHeight + 0.0001f >= fHeight && fHeight < 5.0f )
				{
					fClosestDistanceSq = fDistanceSq;
					iClosestSegment    = i;
					fClosestT          = fT;
					fClosestHeight     = fHeight;
				}
			}
		}
	}

	if ( iClosestSegment < 0 )
		return -1.0f;

	TFLOAT fSegmentStart = ( iClosestSegment > 0 ) ? m_pfLengths[ iClosestSegment - 1 ] : 0.0f;

	return fSegmentStart + ( m_pfLengths[ iClosestSegment ] - fSegmentStart ) * fClosestT;
}

// $Barnyard: FUNCTION 00460830
void ACameraSplinePath::GetSegment( TINT& a_rOutSegment, TFLOAT& a_rOutT, TFLOAT a_fProgress ) const
{
	a_rOutSegment = 0;
	a_rOutT       = 0.0f;

	if ( a_fProgress <= 1.0f )
	{
		if ( a_fProgress > 0.0f )
		{
			for ( TINT i = 0; i < m_iNumSegments; i++ )
			{
				if ( a_fProgress <= m_pfLengths[ i ] )
				{
					TFLOAT fSegmentStart = ( i > 0 ) ? m_pfLengths[ i - 1 ] : 0.0f;

					a_rOutSegment = i;
					a_rOutT       = ( a_fProgress - fSegmentStart ) / ( m_pfLengths[ i ] - fSegmentStart );
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

// $Barnyard: FUNCTION 00460930
TBOOL ACameraSplinePath::GetPoint( TVector4& a_rOutPoint, TFLOAT a_fProgress ) const
{
	TINT   iSegment;
	TFLOAT fT;
	GetSegment( iSegment, fT, a_fProgress );

	a_rOutPoint = m_pSegments[ iSegment ].GetPoint( fT );
	return TTRUE;
}
