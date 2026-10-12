#pragma once
#include "TMatrix44.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Capsule (segment of fLength along vDirection centred on vCentre).
//-----------------------------------------------------------------------------
struct TCapsule
{
	TCapsule( const TVector4& a_rcCentre, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength );

	TVector4 vCentre;         // 0x00
	TVector4 vDirection;      // 0x10
	TFLOAT   fRadius;         // 0x20
	TFLOAT   fLength;         // 0x24
	TFLOAT   fBoundingRadius; // 0x28
};

//-----------------------------------------------------------------------------
// Purpose: Triangle used by the box queries.
//-----------------------------------------------------------------------------
struct TCollisionTriangle
{
	TCollisionTriangle() = default;
	TCollisionTriangle( const TVector4& a_rcVertex1, const TVector4& a_rcVertex2, const TVector4& a_rcVertex3, const TVector4& a_rcNormal );

	TVector4 aVertices[ 3 ]; // 0x00
	TVector4 vNormal;        // 0x30
};

//-----------------------------------------------------------------------------
// Purpose: Contact found by the box queries.
//-----------------------------------------------------------------------------
struct TCollisionContact
{
	TVector4 vPosition; // 0x00
	TVector4 vNormal;   // 0x10
	TFLOAT   fDepth;    // 0x20
};

//-----------------------------------------------------------------------------
// Purpose: Oriented bounding box. The rows of the transform are the axes of the
// box and its centre, m_vExtents are the half sizes along the axes.
// Name from NT08 (Toshi::TOBBox::PointInside), size 0x58.
//-----------------------------------------------------------------------------
class TOBBox
{
public:
	TOBBox();
	TOBBox( const TOBBox& a_rcOther );
	TOBBox( const TMatrix44& a_rcTransform, const TVector4& a_rcExtents );

	TOBBox& operator=( const TOBBox& a_rcOther );

	// Returns the point of the box that is the closest to a_rcPoint
	void GetClosestPoint( const TVector4& a_rcPoint, TVector4& a_rOutPoint ) const;

	// Projects the box onto a_rcAxis
	static void GetProjection( const TVector4& a_rcAxis, const TMatrix44& a_rcTransform, const TVector4& a_rcExtents, TFLOAT& a_rfMin, TFLOAT& a_rfMax );

	const TVector4& GetAxis( TINT a_iAxis ) const { return m_oTransform.AsBasisVector4( a_iAxis ); }
	const TVector4& GetCentre() const { return m_oTransform.GetTranslation(); }

	//-----------------------------------------------------------------------------
	// Overlap tests
	//-----------------------------------------------------------------------------
	static TBOOL IntersectsOBB( const TOBBox& a_rcBox1, const TOBBox& a_rcBox2 );
	static TBOOL IntersectsSphere( const TOBBox& a_rcBox, const TVector4& a_rcSphere );
	static TBOOL IntersectsCapsule( const TOBBox& a_rcBox, const TCapsule& a_rcCapsule );

	//-----------------------------------------------------------------------------
	// Contacts (the normal points from the second shape to the first one)
	//-----------------------------------------------------------------------------
	static TBOOL CollideOBB( const TOBBox& a_rcBox1, const TOBBox& a_rcBox2, TCollisionContact& a_rContact );
	static TBOOL CollideSphere( const TOBBox& a_rcBox, const TVector4& a_rcSphere, TCollisionContact& a_rContact );
	static TBOOL CollideCapsule( const TOBBox& a_rcBox, const TCapsule& a_rcCapsule, TCollisionContact& a_rContact );
	static TBOOL CollideSphereSphere( const TVector4& a_rcSphere1, const TVector4& a_rcSphere2, TCollisionContact& a_rContact );

	//-----------------------------------------------------------------------------
	// Box against a triangle. Return the time of the hit (0..1) or 2.0f if the box doesn't hit the triangle.
	//-----------------------------------------------------------------------------

	// Sweeps a_rcBox along a_rcMovement
	static TFLOAT SweepVsTriangle( const TOBBox& a_rcBox, const TVector4& a_rcMovement, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal );

	// Sweeps the box from a_rcPrevBox to a_rcBox
	static TFLOAT SweepVsTriangle( const TOBBox& a_rcPrevBox, const TOBBox& a_rcBox, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal );

	// Sweeps the box against the triangle from outside of the triangle's plane
	static TFLOAT PenetrationVsTriangle( const TOBBox& a_rcBox, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal );

	// Closest points of the segment a_rcStart - a_rcEnd and the box
	static void ClosestPointsToSegment( const TVector4& a_rcStart, const TVector4& a_rcEnd, const TOBBox& a_rcBox, TVector4& a_rOutSegmentPoint, TVector4& a_rOutBoxPoint );

private:
	static TBOOL TestSweptAxis( TFLOAT a_fVelocity, TFLOAT a_fMin1, TFLOAT a_fMax1, TFLOAT a_fMin2, TFLOAT a_fMax2, TFLOAT& a_rfEnter, TFLOAT& a_rfExit );
	static void  ProjectTriangle( const TVector4& a_rcAxis, const TCollisionTriangle& a_rcTriangle, TFLOAT& a_rfMin, TFLOAT& a_rfMax );
	static void  ClosestPointOnSegmentToLine( const TVector4& a_rcLinePoint, const TVector4& a_rcSegmentVector, TVector4& a_rOutPoint, const TVector4& a_rcSegmentStart, const TVector4& a_rcLineDirection );
	static void  LineClosestApproach( const TVector4& a_rcPointA, const TVector4& a_rcDirA, const TVector4& a_rcPointB, const TVector4& a_rcDirB, TFLOAT& a_rfAlpha, TFLOAT& a_rfBeta );
	static TINT  ClipRectangleQuad( const TFLOAT* a_pRectangle, TFLOAT* a_pQuad, TFLOAT* a_pOutPoints );

public:
	TMatrix44 m_oTransform; // 0x00
	TVector4  m_vExtents;   // 0x40
	TFLOAT    m_fRadius;    // 0x50, length of the extents
	TFLOAT    m_fMinExtent; // 0x54
};

TSTATICASSERT( sizeof( TOBBox ) == 0x58 );

TOSHI_NAMESPACE_END
