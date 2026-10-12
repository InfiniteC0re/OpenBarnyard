#pragma once
#include <Math/TVector4.h>
#include <Render/TTransformObject.h>

class ACollisionObject;
class ACollisionModelInstance;

//-----------------------------------------------------------------------------
// Purpose: A ray used by the collision queries (Ghidra: ARayData).
// NOTE: the direction is expected to be normalised, fLength is the max distance.
//-----------------------------------------------------------------------------
struct ARayData
{
	ARayData() = default;

	// $Barnyard: FUNCTION 0061ee40
	ARayData( const Toshi::TVector4& a_rcOrigin, const Toshi::TVector4& a_rcDirection, TFLOAT a_fLength )
	    : vOrigin( a_rcOrigin ), vDirection( a_rcDirection ), fLength( a_fLength )
	{}

	// Creates a ray going from a_rcStart to a_rcEnd.
	ARayData( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcEnd );

	Toshi::TVector4 vOrigin;    // 0x00
	Toshi::TVector4 vDirection; // 0x10
	TFLOAT          fLength;    // 0x20
};

TSTATICASSERT( sizeof( ARayData ) == 0x24 );

//-----------------------------------------------------------------------------
// Purpose: Result of a collision query against a collision object.
//-----------------------------------------------------------------------------
struct ACollisionHit
{
	// NOTE: the retail constructor doesn't initialise iCollGroup
	// $Barnyard: FUNCTION 00479790
	ACollisionHit()
	    : fDistance( 0.0f ), pObject( TNULL ), vNormal( Toshi::TVector4::VEC_ZERO ), iCollGroup( 0 )
	{}

	TFLOAT            fDistance;  // 0x00
	ACollisionObject* pObject;    // 0x04
	Toshi::TVector4   vNormal;    // 0x08
	TINT              iCollGroup; // 0x18, TCollisionCommon::TOSHICGROUP of the hit triangle (ACollisionObjectMesh)
};

TSTATICASSERT( sizeof( ACollisionHit ) == 0x1c );

//-----------------------------------------------------------------------------
// Purpose: List of the triangles hit by ACollisionModel queries.
// The name comes from NT08 (ACollisionModelRayHitList).
//-----------------------------------------------------------------------------
struct ACollisionModelRayHitList
{
	static constexpr TINT MAX_NUM_HITS = 256;

	struct Hit
	{
		// Returns the world space normal of the hit triangle.
		Toshi::TVector4 GetNormal() const;

		// Outputs the world space vertices of the hit triangle.
		void GetTriangle( Toshi::TVector3 a_aOutVertices[ 3 ] ) const;

		ACollisionModelInstance* pInstance;  // 0x00
		TINT                     iModel;     // 0x04, index of the ACollisionModel in the model set
		TUINT                    uiFaceId;   // 0x08
		TFLOAT                   fDistance;  // 0x0c
		Toshi::TTransformObject* pTransform; // 0x10, transform of the instance
	};

	ACollisionModelRayHitList()
	    : iNumHits( 0 )
	{}

	TBOOL IsFull() const { return iNumHits == MAX_NUM_HITS; }

	// Removes a hit and moves the following ones.
	void Erase( TINT a_iIndex );

	// $Barnyard: FUNCTION 004cace0
	void Clear()
	{
		for ( TINT i = 0; i < iNumHits; i++ )
		{
			aHits[ i ].pInstance  = TNULL;
			aHits[ i ].iModel     = 0;
			aHits[ i ].uiFaceId   = 0;
			aHits[ i ].fDistance  = 0.0f;
			aHits[ i ].pTransform = TNULL;
		}

		iNumHits = 0;
	}

	TINT iNumHits;
	Hit  aHits[ MAX_NUM_HITS ];
};

TSTATICASSERT( sizeof( ACollisionModelRayHitList ) == 0x1404 );
