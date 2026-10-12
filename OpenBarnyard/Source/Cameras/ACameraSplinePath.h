#pragma once
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: One cubic segment of a camera spline (0x50 bytes).
// NOTE: an engine helper of the original binary (ctor 006cdb60, evaluated by 006cdc10 and set up
// from its control points by 006cddc0). The name is invented.
//-----------------------------------------------------------------------------
struct ACameraSplineSegment
{
	// $Barnyard: FUNCTION 006cdb60
	ACameraSplineSegment()
	    : m_vStart( Toshi::TVector4::VEC_ZERO )
	    , m_vStartTangent( Toshi::TVector4::VEC_ZERO )
	    , m_vEnd( Toshi::TVector4::VEC_ZERO )
	    , m_vEndTangent( Toshi::TVector4::VEC_ZERO )
	    , m_fLength( -1.0f )
	    , m_fInvLength( -1.0f )
	{}

	// Hermite interpolation between a_rcStart (tangent a_rcStartTangent) and a_rcEnd (tangent
	// a_rcEndTangent). w is the one of a_rcStart.
	static void Hermite( Toshi::TVector4& a_rOut, const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcStartTangent, const Toshi::TVector4& a_rcEnd, const Toshi::TVector4& a_rcEndTangent, TFLOAT a_fT );

	// Point of the segment at a_fT (0..1)
	Toshi::TVector4 GetPoint( TFLOAT a_fT ) const;

	// Parameter of the point of the segment closest to a_rcPoint, found with a_iNumIterations
	// steps of a binary search
	TFLOAT FindClosestT( const Toshi::TVector4& a_rcPoint, TINT a_iNumIterations ) const;

	// Sets the control points of the segment and calculates its length
	void Set( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcStartTangent, const Toshi::TVector4& a_rcEnd, const Toshi::TVector4& a_rcEndTangent );

	// Changes only the end point and tangent and recalculates the length
	void SetEnd( const Toshi::TVector4& a_rcEnd, const Toshi::TVector4& a_rcEndTangent );

	// Approximate length of the Hermite curve (sum of 11 chords)
	static TFLOAT CalculateLength( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcStartTangent, const Toshi::TVector4& a_rcEnd, const Toshi::TVector4& a_rcEndTangent );

	Toshi::TVector4 m_vStart;        // 0x00
	Toshi::TVector4 m_vStartTangent; // 0x10
	Toshi::TVector4 m_vEnd;          // 0x20
	Toshi::TVector4 m_vEndTangent;   // 0x30
	TFLOAT          m_fLength;       // 0x40, length (see Set)
	TFLOAT          m_fInvLength;    // 0x44, 1 / length, ACameraHelperSpline scales its speed with it
	TFLOAT          m_afUnk48[ 2 ];  // 0x48, not initialised by the ctor
};

TSTATICASSERT( sizeof( ACameraSplineSegment ) == 0x50 );

//-----------------------------------------------------------------------------
// Purpose: A camera path made of up to m_iMaxSegments spline segments, with the normalised
// accumulated length of every segment (used to map a 0..1 progress to a segment).
// vtable 0x0071ccd0 (only the deleting destructor). Name from the RTTI of the Wii build (vtable 8050afc8).
//-----------------------------------------------------------------------------
class ACameraSplinePath
{
public:
	ACameraSplinePath( TINT a_iMaxSegments );
	virtual ~ACameraSplinePath();

	// Builds a_iNumSegments segments going through a_pPoints (a_iNumSegments + 1 points with their tangents)
	void Create( TINT a_iNumSegments, const Toshi::TVector4* a_pPoints, const Toshi::TVector4* a_pTangents );

	// Returns the progress (0..1) of the point of the curve closest to a_rcPosition on the XZ plane,
	// considering only the points lower than a_rcPosition.y + a_fMaxHeight and closer than
	// sqrt( a_fMaxDistanceSq ). Returns -1 if there is no such point.
	TFLOAT FindProgress( const Toshi::TVector4& a_rcPosition, TFLOAT a_fMaxDistanceSq, TFLOAT a_fMaxHeight ) const;

	// Converts a progress of the curve to a segment index and its parameter
	void GetSegment( TINT& a_rOutSegment, TFLOAT& a_rOutT, TFLOAT a_fProgress ) const;

	// Point of the curve at a_fProgress (0..1). Always returns TTRUE.
	TBOOL GetPoint( Toshi::TVector4& a_rOutPoint, TFLOAT a_fProgress ) const;

private:
	TINT                  m_iMaxSegments; // 0x04
	TINT                  m_iNumSegments; // 0x08, set up by Create
	ACameraSplineSegment* m_pSegments;    // 0x0c
	TFLOAT*               m_pfLengths;    // 0x10, accumulated normalised length of every segment
};

TSTATICASSERT( sizeof( ACameraSplinePath ) == 0x14 );
