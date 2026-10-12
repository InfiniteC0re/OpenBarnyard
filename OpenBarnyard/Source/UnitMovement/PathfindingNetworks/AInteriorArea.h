#pragma once

#include "UnitMovement/PathfindingNetworks/AInteriorArcBounds.h"

#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Toshi/TPString8.h>

class AInteriorNetwork;

struct AInteriorAreaLink
{
	TINT   m_iAreaIndex;
	TFLOAT m_fCost;
};

//-----------------------------------------------------------------------------
// Purpose: View cone used to pull the path straight through the portals
// (linked edges) of the areas of a path. The cone starts at m_vApex and is
// bounded by the directions m_vEdgeA and m_vEdgeB.
// NOTE: 0x1c bytes, lives on the stack of AInteriorNetwork::FindNextPathPoint.
//-----------------------------------------------------------------------------
struct AInteriorPathFunnel
{
	enum STATE : TINT
	{
		STATE_OPEN   = 0, // nothing narrowed it yet, everything is inside
		STATE_NARROW = 1,
		STATE_CLOSED = 2, // nothing is inside
	};

	// $Barnyard: FUNCTION 0054f660
	void Init( const Toshi::TVector2& a_rcApex )
	{
		m_eState = STATE_OPEN;
		m_vApex  = a_rcApex;
	}

	AInteriorPathFunnel& operator=( const AInteriorPathFunnel& a_rcOther );

	TBOOL IsDirectionInside( const Toshi::TVector2& a_rcDirection ) const;
	TBOOL IsPointInside( const Toshi::TVector2& a_rcPoint ) const;

	void NarrowEdgeA( const Toshi::TVector2& a_rcDirection );
	void NarrowEdgeB( const Toshi::TVector2& a_rcDirection );
	void NarrowEdgeAToPoint( const Toshi::TVector2& a_rcPoint );
	void NarrowEdgeBToPoint( const Toshi::TVector2& a_rcPoint );

	// Narrows the cone to the portal a_rcEdgeA - a_rcEdgeB
	void NarrowToPortal( const Toshi::TVector2& a_rcEdgeA, const Toshi::TVector2& a_rcEdgeB );

	STATE           m_eState; // 0x00
	Toshi::TVector2 m_vApex;  // 0x04
	Toshi::TVector2 m_vEdgeA; // 0x0c, normalised
	Toshi::TVector2 m_vEdgeB; // 0x14, normalised
};

//-----------------------------------------------------------------------------
// Purpose: Moves a circle on the XZ plane through the areas of a network
// (AInteriorArea::CollideMove), stopping it at the outer edges of the network
// and recording the linked edges it crosses on the way.
// NOTE: 0xc4 bytes, lives on the stack of AInteriorNetwork::UpdateHardAvoidance.
//-----------------------------------------------------------------------------
struct AInteriorMoveQuery
{
	static constexpr TINT MAX_NUM_CROSSINGS = 16;

	struct Crossing
	{
		TFLOAT fDistance; // from m_vStart
		TINT   iArea;     // the area that is entered
	};

	void Reset();

	// Area the circle ends up in: the one of the last crossing before the hit (m_iArea if there's none)
	TINT GetEndArea() const;

	// Inserts the crossing sorted by its distance
	void AddCrossing( TFLOAT a_fDistance, TINT a_iArea );

	Toshi::TVector2 m_vStart;                          // 0x00
	TINT            m_iArea;                           // 0x08, area of m_vStart
	Toshi::TVector2 m_vTarget;                         // 0x0c
	TFLOAT          m_fRadius;                         // 0x14
	TBOOL           m_bHit;                            // 0x18, an outer edge was hit
	TBOOL           m_bSlide;                          // 0x19, slide along the hit edge
	TFLOAT          m_fHitDistance;                    // 0x1c, 0.0f once hit (FLT_MAX otherwise)
	TINT            m_iNumCrossings;                   // 0x20
	Crossing        m_aCrossings[ MAX_NUM_CROSSINGS ]; // 0x24
	Toshi::TVector2 m_vHitPosition;                    // 0xa4, where the circle stops
	Toshi::TVector2 m_vUnused;                         // 0xac
	Toshi::TVector2 m_vHitNormal;                      // 0xb4, normal of the hit edge
	Toshi::TVector2 m_vSlideNormal;                    // 0xbc, m_vHitNormal bent towards the normal of a close corner
};

class AInteriorArea
{
public:
	AInteriorArea()  = default;
	~AInteriorArea() = default;

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                 = 0,
		FLAGS_CHECK_CORNERS        = BITFLAG( 0 ), // ClipLineToCorners checks the corners of the outer edges
		FLAGS_HEIGHT_RAYCAST_LONG  = BITFLAG( 2 ),
		FLAGS_HEIGHT_RAYCAST_SHORT = BITFLAG( 3 ),
		FLAGS_HEIGHT_FROM_TERRAIN  = BITFLAG( 4 ),
		FLAGS_ACCEPT_GOAL_HEIGHT   = BITFLAG( 5 ),
		FLAGS_NORMAL_NAV_AREA      = BITFLAG( 6 ),
		FLAGS_SPECIAL_GROUP_AREA   = BITFLAG( 7 ),
		FLAGS_NON_ROUTABLE         = BITFLAG( 9 ),
		FLAGS_RUNTIME_SPECIAL      = BITFLAG( 10 ),
	};

public:
	void  GetPosition( Toshi::TVector4& a_rOutPosition ) const;
	void  GetPositionXZ( Toshi::TVector4& a_rOutPosition ) const;
	TBOOL CheckPointInsideArea( const Toshi::TVector4& a_rcPoint ) const;

	// Same as CheckPointInsideArea but takes a regular 3D position (the height is ignored)
	TBOOL CheckPositionInsideArea( const Toshi::TVector4& a_rcPosition ) const;

	// Returns TTRUE if the circle (a_rcPoint.x and .y are the XZ center) is completely inside the area.
	// a_bOnlyOuterEdges ignores the edges shared with the neighbour areas.
	TBOOL IsCircleInsideArea( const Toshi::TVector4& a_rcPoint, TFLOAT a_fRadius, TBOOL a_bOnlyOuterEdges ) const;

	// Same as IsCircleInsideArea but takes a regular 3D position (the height is ignored)
	TBOOL IsCircleAtPositionInsideArea( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TBOOL a_bOnlyOuterEdges ) const;

	// Height of the special group area (0.0f if it isn't one)
	TFLOAT GetSpecialHeight() const;

	// Point a_iPoint of the special group area (see AInteriorNetwork::GetSpecialAreaPoint)
	const Toshi::TVector4& GetSpecialPoint( TINT a_iPoint ) const;

	// Link to the area with index a_iAreaIndex, TNULL if they aren't linked
	AInteriorAreaLink* FindLink( TINT a_iAreaIndex ) const;

	// Whether the area is one of the special areas (stored after the normal ones), a_iUnused isn't used
	TBOOL IsSpecialArea( TINT a_iUnused ) const;

	// Random point (XZ packed into x and y) at least 0.5 units inside of the area, a_fUnused isn't used
	void GetRandomPointXZ( Toshi::TVector2& a_rOutPoint, TFLOAT a_fUnused ) const;

	// Random position in the area with its height resolved by the network, a_fUnused isn't used
	void GetRandomPosition( Toshi::TVector4& a_rOutPosition, TFLOAT a_fUnused ) const;

	// Moves the circle of a_rQuery from this area towards its target through the linked areas.
	// a_pVisitedAreas is a bit set of the areas that were checked already.
	void CollideMove( AInteriorMoveQuery& a_rQuery, TUINT32* a_pVisitedAreas ) const;

	// Checks if the line a_rcFrom - a_rcTo keeps a_fRadius away from the corners of the outer edges.
	// a_rOutEnd gets the farthest point that can be walked to in a straight line, the corners narrow a_pFunnel (optional).
	TBOOL ClipLineToCorners( const Toshi::TVector2& a_rcFrom, const Toshi::TVector2& a_rcTo, TFLOAT a_fRadius, Toshi::TVector2& a_rOutEnd, AInteriorPathFunnel* a_pFunnel ) const;

public:
	TFLOAT              m_fOriginX;
	TFLOAT              m_fOriginZ;
	TFLOAT              m_fMinX;
	TFLOAT              m_fMinZ;
	TFLOAT              m_fMaxX;
	TFLOAT              m_fMaxZ;
	AInteriorNetwork*   m_pNetwork;
	TINT                m_iIndex;
	TINT                m_iNumArcBounds;
	AInteriorArcBounds* m_pArcBounds;
	TINT                m_iNumLinks;
	AInteriorAreaLink*  m_pLinks;
	TFLOAT              m_fBaseHeight;
	TFLOAT              m_fPathHeuristicScale;

	// HACK: swap this with pooled name after reading the TRB data
	union
	{
		TCHAR*            m_szName_Internal;
		Toshi::TPString8* m_strName;
	};

	FLAGS m_eFlags;
};
