#include "pch.h"
#include "AWorld.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Frustum::Frustum()
{
	aFrustumPlanes[ 0 ].uiClipFlag = 0;
	aFrustumPlanes[ 1 ].uiClipFlag = 0;
	aFrustumPlanes[ 2 ].uiClipFlag = 0;
	aFrustumPlanes[ 3 ].uiClipFlag = 0;
	aFrustumPlanes[ 4 ].uiClipFlag = 0;
	aFrustumPlanes[ 5 ].uiClipFlag = 0;
	iActivePlaneCount              = 0;
}

void Frustum::InitReduce()
{
	iActivePlaneCount = 6;

	for ( TINT i = 0; i < iActivePlaneCount; i++ )
	{
		apActivePlanes[ i ] = &aFrustumPlanes[ i ].oPlane;
	}
}

FrustumIntersectSphereResult Frustum::IntersectSphereReduce( const Toshi::TSphere& a_rSphere )
{
	for ( TINT i = 0; i < iActivePlaneCount; )
	{
		TFLOAT fDistance = TVector4::DotProduct3( a_rSphere.AsVector4(), apActivePlanes[ i ]->AsVector4() ) - apActivePlanes[ i ]->GetD();

		if ( a_rSphere.GetRadius() < fDistance )
			return FISR_NOT_VISIBLE;

		if ( a_rSphere.GetRadius() <= -fDistance )
		{
			iActivePlaneCount--;
			TSwapValues( apActivePlanes[ i ], apActivePlanes[ iActivePlaneCount ] );
		}
		else
		{
			i += 1;
		}
	}

	if ( iActivePlaneCount == 0 )
		return FISR_ALL_VISIBLE;

	return FISR_PARTIALLY_VISIBLE;
}

void Frustum::Transform( const Frustum& a_rFrustum, const Toshi::TMatrix44& a_rMatrix )
{
	for ( TINT i = 0; i < 6; i++ )
	{
		TMatrix44::TransformPlaneOrthogonal(
		    aFrustumPlanes[ i ].oPlane,
		    a_rMatrix,
		    a_rFrustum.aFrustumPlanes[ i ].oPlane );

		aFrustumPlanes[ i ].uiClipFlag = a_rFrustum.aFrustumPlanes[ i ].uiClipFlag;
	}
}
