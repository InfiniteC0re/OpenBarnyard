#pragma once
#include "Cameras/ACameraSplinePath.h"

#include <Math/TVector4.h>
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Purpose: A path made of hermite segments read from a TRB symbol (the coin
// trails, the Joy Ride track, the pipes of ASharpSquirter...). Positions on the
// path are given as a progress from 0 to 1 (normalised length along the path).
// NOTE: the name is invented. The constructor (0054c710) and the destructor
// (00421c40) are shared folded functions of the original binary.
//-----------------------------------------------------------------------------
class ASpline
{
public:
	// Data of the TRB symbol
	struct Data
	{
		TINT                  iNumSegments; // 0x0
		ACameraSplineSegment* pSegments;    // 0x4
	};

public:
	ASpline()
	    : m_pfSegmentEnds( TNULL ), m_fLength( 0.0f ), m_iNumSegments( 0 ), m_pSegments( TNULL )
	{}

	~ASpline() { TFree( m_pfSegmentEnds ); }

	// Reads the spline from the symbol a_szSymbol of a_rTRB
	void Load( Toshi::TTRB& a_rTRB, const TCHAR* a_szSymbol );

	// Reads the spline from the symbol a_szSymbol of the asset file a_szFileName (see AAssetLoader)
	void Load( const TCHAR* a_szFileName, const TCHAR* a_szSymbol, TUINT a_eAssetType );

	// Position at a_fProgress (0..1)
	void GetPosition( TFLOAT a_fProgress, Toshi::TVector4& a_rOutPosition ) const;

	// Normalised direction at a_fProgress (0..1)
	void GetDirection( TFLOAT a_fProgress, Toshi::TVector4& a_rOutDirection ) const;

	// Position and normalised direction at a_fProgress (0..1)
	void GetPositionAndDirection( TFLOAT a_fProgress, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection ) const;

	// Progress of the point of the spline that is the closest to a_rcPoint, -1 if no segment
	// is closer than a_fMaxDistance.
	TFLOAT FindClosestProgress( const Toshi::TVector4& a_rcPoint, TFLOAT a_fMaxDistance ) const;

	// Point of the spline that is the closest to a_rcPoint (see FindClosestProgress), VEC_ZERO
	// (with w = 1) if none is closer than a_fMaxDistance.
	Toshi::TVector4 GetClosestPosition( const Toshi::TVector4& a_rcPoint, TFLOAT a_fMaxDistance ) const;

	// Point, direction and progress of the point of the spline that is the closest to a_rcPoint
	// (see FindClosestProgress). The position is VEC_ZERO (with w = 1) and the progress 0 if none is
	// closer than a_fMaxDistance.
	void GetClosestPositionAndDirection( const Toshi::TVector4& a_rcPoint, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection, TFLOAT& a_rOutProgress, TFLOAT a_fMaxDistance ) const;

	TINT                        GetNumSegments() const { return m_iNumSegments; }
	const ACameraSplineSegment& GetSegment( TINT a_iIndex ) const { return m_pSegments[ a_iIndex ]; }
	TFLOAT                      GetLength() const { return m_fLength; }
	TBOOL                       IsLoaded() const { return m_pSegments != TNULL; }

private:
	// Sets up m_pSegments and m_pfSegmentEnds from the symbol data
	void SetData( const Data* a_pData );

	// Measures the segments: fills m_pfSegmentEnds and m_fLength
	void CalculateLengths();

	// Converts a progress along the spline to a segment and the parameter on that segment
	void GetSegment( TFLOAT a_fProgress, TINT& a_rOutSegment, TFLOAT& a_rOutT ) const;

	// Progress at the start of the segment a_iSegment
	TFLOAT GetSegmentStart( TINT a_iSegment ) const { return ( a_iSegment > 0 ) ? m_pfSegmentEnds[ a_iSegment - 1 ] : 0.0f; }

private:
	TFLOAT*               m_pfSegmentEnds; // 0x0, normalised accumulated length at the end of every segment
	TFLOAT                m_fLength;       // 0x4, sum of the lengths stored in the segments
	TINT                  m_iNumSegments;  // 0x8
	ACameraSplineSegment* m_pSegments;     // 0xc, owned by the TRB
};

TSTATICASSERT( sizeof( ASpline ) == 0x10 );
