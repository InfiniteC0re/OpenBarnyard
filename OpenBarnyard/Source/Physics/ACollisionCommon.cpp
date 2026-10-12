#include "pch.h"
#include "ACollisionCommon.h"
#include "ACollisionModelInstance.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0061ee90
ARayData::ARayData( const TVector4& a_rcStart, const TVector4& a_rcEnd )
    : vOrigin( a_rcStart )
{
	vDirection.Set( a_rcEnd.x - a_rcStart.x, a_rcEnd.y - a_rcStart.y, a_rcEnd.z - a_rcStart.z, a_rcEnd.w );
	fLength = vDirection.NormaliseMag();
}

// $Barnyard: FUNCTION 0061b220
TVector4 ACollisionModelRayHitList::Hit::GetNormal() const
{
	TVector3* apVertices[ 3 ];
	pInstance->GetCollisionData( iModel )->GetTriangleVertices( uiFaceId, apVertices );

	// Normal of the triangle (not normalised yet)
	TVector4 vecNormal;
	TMath::TriangleUnscaledNormalCounterClockwise( vecNormal, apVertices );

	// Move it to the world space
	TMatrix44 matTransform;
	pTransform->GetLocalMatrixImp( matTransform );

	TMatrix44::RotateVector( vecNormal, matTransform, vecNormal );
	vecNormal.Normalise();

	return vecNormal;
}

// $Barnyard: FUNCTION 0061bce0
void ACollisionModelRayHitList::Hit::GetTriangle( TVector3 a_aOutVertices[ 3 ] ) const
{
	TVector3* apVertices[ 3 ];
	pInstance->GetCollisionData( iModel )->GetTriangleVertices( uiFaceId, apVertices );

	TMatrix44 matTransform;
	pTransform->GetLocalMatrixImp( matTransform );

	for ( TINT i = 0; i < 3; i++ )
		TMatrix44::TransformVector( a_aOutVertices[ i ], matTransform, *apVertices[ i ] );
}

// $Barnyard: FUNCTION 0061a730
void ACollisionModelRayHitList::Erase( TINT a_iIndex )
{
	for ( TINT i = a_iIndex; i < iNumHits - 1; i++ )
		aHits[ i ] = aHits[ i + 1 ];

	Hit& rLast       = aHits[ iNumHits - 1 ];
	rLast.pInstance  = TNULL;
	rLast.iModel     = 0;
	rLast.uiFaceId   = 0;
	rLast.fDistance  = 0.0f;
	rLast.pTransform = TNULL;

	iNumHits--;
}
