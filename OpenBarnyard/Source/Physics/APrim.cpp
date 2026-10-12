#include "pch.h"
#include "APrim.h"
#include "AGeomIntersectResult.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005798e0, CreateObject/CreateObjectInPlace are shared stubs returning TNULL
TDEFINE_CLASS_INIT_NORUNTIME( APrim, 1, 0 );

// NOTE: GetClass 0057af30, CreateObject 0057b0b0, CreateObjectInPlace 0057b120
TDEFINE_CLASS( APrimSphere, 1, 0 );

// NOTE: GetClass 0057af40, CreateObject 0057b190, CreateObjectInPlace 0057b1f0
TDEFINE_CLASS( APrimBox, 1, 0 );

// NOTE: GetClass 0057af60, CreateObject 0057b230, CreateObjectInPlace 0057b2b0
TDEFINE_CLASS( APrimCapsule, 1, 0 );

// NOTE: GetClass 0057af50, CreateObject 0057b4c0, CreateObjectInPlace 0057b520
TDEFINE_CLASS( APrimComposite, 1, 0 );

APrim::IntersectRayFunc APrim::ms_aIntersectRayFuncs[ TYPE_NUMOF ];
APrim::IntersectFunc    APrim::ms_aIntersectFuncs[ TYPE_NUMOF ][ TYPE_NUMOF ];

static constexpr TFLOAT s_fEpsilon = 1.1920929e-07f;

// Clips the ray parameter range against one side of a slab (Liang-Barsky).
// Returns TFALSE if the range becomes empty.
// $Barnyard: FUNCTION 00579a10
static TBOOL ClipSlab( TFLOAT& a_rfFar, TFLOAT& a_rfNear, TFLOAT a_fDenominator, TFLOAT a_fNumerator )
{
	if ( a_fDenominator > 0.0f )
	{
		if ( a_fDenominator * a_rfFar < a_fNumerator )
			return TFALSE;

		if ( a_fDenominator * a_rfNear < a_fNumerator )
			a_rfNear = a_fNumerator / a_fDenominator;

		return TTRUE;
	}

	if ( a_fDenominator < 0.0f )
	{
		if ( a_fDenominator * a_rfNear < a_fNumerator )
			return TFALSE;

		if ( a_fDenominator * a_rfFar < a_fNumerator )
			a_rfFar = a_fNumerator / a_fDenominator;

		return TTRUE;
	}

	return a_fNumerator <= 0.0f;
}

// $Barnyard: FUNCTION 0057aea0
void APrim::InitialiseClass()
{
	ms_aIntersectFuncs[ TYPE_COMPOSITE ][ TYPE_SPHERE ]    = IntersectComposite;
	ms_aIntersectFuncs[ TYPE_COMPOSITE ][ TYPE_BOX ]       = IntersectComposite;
	ms_aIntersectFuncs[ TYPE_COMPOSITE ][ TYPE_CAPSULE ]   = IntersectComposite;
	ms_aIntersectFuncs[ TYPE_COMPOSITE ][ TYPE_COMPOSITE ] = IntersectComposite;
	ms_aIntersectFuncs[ TYPE_COMPOSITE ][ TYPE_CUSTOM ]    = IntersectComposite;
	ms_aIntersectFuncs[ TYPE_SPHERE ][ TYPE_SPHERE ]       = IntersectSphereSphere;
	ms_aIntersectFuncs[ TYPE_SPHERE ][ TYPE_BOX ]          = IntersectSphereBox;
	ms_aIntersectFuncs[ TYPE_SPHERE ][ TYPE_CAPSULE ]      = IntersectSphereCapsule;
	ms_aIntersectFuncs[ TYPE_CUSTOM ][ TYPE_SPHERE ]       = IntersectCustom;
	ms_aIntersectFuncs[ TYPE_CUSTOM ][ TYPE_BOX ]          = IntersectCustom;
	ms_aIntersectFuncs[ TYPE_CUSTOM ][ TYPE_CAPSULE ]      = IntersectCustom;
	ms_aIntersectFuncs[ TYPE_CUSTOM ][ TYPE_COMPOSITE ]    = IntersectCustom;
	ms_aIntersectFuncs[ TYPE_CUSTOM ][ TYPE_CUSTOM ]       = IntersectCustom;

	ms_aIntersectRayFuncs[ TYPE_SPHERE ]    = IntersectRaySphere;
	ms_aIntersectRayFuncs[ TYPE_BOX ]       = IntersectRayBox;
	ms_aIntersectRayFuncs[ TYPE_CAPSULE ]   = IntersectRayCapsule;
	ms_aIntersectRayFuncs[ TYPE_COMPOSITE ] = IntersectRayComposite;
	ms_aIntersectRayFuncs[ TYPE_CUSTOM ]    = IntersectRayCustom;
}

// DOES_NOTHING
void APrim::DeinitialiseClass()
{
}

// $Barnyard: FUNCTION 005798f0
APrim::APrim( TYPE a_eType )
    : m_eType( a_eType )
{
}

// $Barnyard: FUNCTION 00579ef0
// $Barnyard: FUNCTION 00579910
APrim::~APrim()
{
}

// $Barnyard: FUNCTION 00579920
TBOOL APrim::Intersect( APrim* a_pPrim1, const TMatrix44& a_rcTransform1, APrim* a_pPrim2, const TMatrix44& a_rcTransform2 )
{
	IntersectFunc fnIntersect = ms_aIntersectFuncs[ a_pPrim1->m_eType ][ a_pPrim2->m_eType ];

	if ( fnIntersect )
		return fnIntersect( a_pPrim1, a_rcTransform1, a_pPrim2, a_rcTransform2, TNULL );

	// Try the other way around
	fnIntersect = ms_aIntersectFuncs[ a_pPrim2->m_eType ][ a_pPrim1->m_eType ];

	if ( fnIntersect )
		return fnIntersect( a_pPrim2, a_rcTransform2, a_pPrim1, a_rcTransform1, TNULL );

	return TFALSE;
}

// $Barnyard: FUNCTION 00579990
TFLOAT APrim::IntersectRay( APrim* a_pPrim, const TMatrix44& a_rcTransform, const TVector3& a_rcOrigin, const TVector3& a_rcDirection, TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused )
{
	IntersectRayFunc fnIntersectRay = ms_aIntersectRayFuncs[ a_pPrim->m_eType ];

	if ( fnIntersectRay )
		return fnIntersectRay( a_pPrim, a_rcTransform, a_rcOrigin, a_rcDirection, a_pHits, a_fMaxDistance, a_iUnused );

	return -1.0f;
}

// $Barnyard: FUNCTION 005799e0
TBOOL APrim::IntersectCustom( APrim* a_pPrim1, const TMatrix44& a_rcTransform1, APrim* a_pPrim2, const TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts )
{
	APrimCustom* pCustom = TSTATICCAST( APrimCustom, a_pPrim1 );

	pCustom->PrepareIntersect( a_pPrim2 );
	return pCustom->IntersectPrim( a_rcTransform1, a_pPrim2, a_rcTransform2, a_pContacts );
}

// $Barnyard: FUNCTION 00579ec0
TFLOAT APrim::IntersectRayCustom( APrim* a_pPrim, const TMatrix44& a_rcTransform, const TVector3& a_rcOrigin, const TVector3& a_rcDirection, TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused )
{
	return TSTATICCAST( APrimCustom, a_pPrim )->IntersectRay( a_rcTransform, a_rcOrigin, a_rcDirection, a_pHits, a_fMaxDistance, a_iUnused );
}

// $Barnyard: FUNCTION 0057a7c0
TBOOL APrim::IntersectSphereSphere( const TVector3& a_rcCenter1, TFLOAT a_fRadius1, const TVector3& a_rcCenter2, TFLOAT a_fRadius2, AGeomContactList* a_pContacts )
{
	TVector3 vecDelta( a_rcCenter2.x - a_rcCenter1.x, a_rcCenter2.y - a_rcCenter1.y, a_rcCenter2.z - a_rcCenter1.z );

	TFLOAT fDistanceSq = vecDelta.x * vecDelta.x + vecDelta.z * vecDelta.z + vecDelta.y * vecDelta.y;
	TFLOAT fRadiusSum  = a_fRadius1 + a_fRadius2;

	if ( fDistanceSq > fRadiusSum * fRadiusSum )
		return TFALSE;

	if ( !a_pContacts )
		return TTRUE;

	if ( fDistanceSq <= s_fEpsilon )
	{
		// The centres are at the same place
		a_pContacts->AddContact( a_rcCenter1, TVector3::VEC_POSZ, fRadiusSum );
		return TTRUE;
	}

	TFLOAT fDistance = TMath::Sqrt( fDistanceSq );
	TFLOAT fContact  = ( a_fRadius1 - a_fRadius2 + fDistance ) * 0.5f;
	TFLOAT fInvDist  = 1.0f / fDistance;

	TVector3 vecNormal( vecDelta.x * fInvDist, vecDelta.y * fInvDist, vecDelta.z * fInvDist );
	TVector3 vecPosition( vecNormal.x * fContact + a_rcCenter1.x, vecNormal.y * fContact + a_rcCenter1.y, vecNormal.z * fContact + a_rcCenter1.z );

	a_pContacts->AddContact( vecPosition, vecNormal, fRadiusSum - fDistance );
	return TTRUE;
}

// $Barnyard: FUNCTION 0057a970
TBOOL APrim::IntersectSphereSphere( APrim* a_pPrim1, const TMatrix44& a_rcTransform1, APrim* a_pPrim2, const TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts )
{
	return IntersectSphereSphere(
	    a_rcTransform1.GetTranslation3(),
	    TSTATICCAST( APrimSphere, a_pPrim1 )->m_fRadius,
	    a_rcTransform2.GetTranslation3(),
	    TSTATICCAST( APrimSphere, a_pPrim2 )->m_fRadius,
	    a_pContacts
	);
}

// $Barnyard: FUNCTION 0057a3f0
TBOOL APrim::IntersectSphereBox( APrim* a_pPrim1, const TMatrix44& a_rcTransform1, APrim* a_pPrim2, const TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts )
{
	APrimSphere*    pSphere      = TSTATICCAST( APrimSphere, a_pPrim1 );
	APrimBox*       pBox         = TSTATICCAST( APrimBox, a_pPrim2 );
	const TVector3& rcSpherePos  = a_rcTransform1.GetTranslation3();
	TVector3        vecExtents   = pBox->m_vExtents;
	TMatrix44       oBoxRotation = a_rcTransform2;

	TVector3 vecDelta(
	    rcSpherePos.x - a_rcTransform2.m_f41,
	    rcSpherePos.y - a_rcTransform2.m_f42,
	    rcSpherePos.z - a_rcTransform2.m_f43
	);

	// The sphere in the space of the box
	TFLOAT afLocal[ 3 ];
	afLocal[ 0 ] = oBoxRotation.m_f13 * vecDelta.z + oBoxRotation.m_f12 * vecDelta.y + oBoxRotation.m_f11 * vecDelta.x;
	afLocal[ 1 ] = oBoxRotation.m_f22 * vecDelta.y + oBoxRotation.m_f21 * vecDelta.x + oBoxRotation.m_f23 * vecDelta.z;
	afLocal[ 2 ] = oBoxRotation.m_f33 * vecDelta.z + oBoxRotation.m_f32 * vecDelta.y + oBoxRotation.m_f31 * vecDelta.x;

	TVector3 vecMin( pBox->m_vExtents.x * -1.0f, pBox->m_vExtents.y * -1.0f, pBox->m_vExtents.z * -1.0f );
	TVector3 vecMax = pBox->m_vExtents;

	// Squared distance from the box
	TFLOAT fDistanceSq = 0.0f;
	TBOOL  bInside     = TTRUE;

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fOutside;

		if ( afLocal[ i ] < ( &vecMin.x )[ i ] )
			fOutside = afLocal[ i ] - ( &vecMin.x )[ i ];
		else if ( afLocal[ i ] > ( &vecMax.x )[ i ] )
			fOutside = afLocal[ i ] - ( &vecMax.x )[ i ];
		else
			continue;

		bInside = TFALSE;
		fDistanceSq += fOutside * fOutside;
	}

	if ( !bInside )
	{
		TFLOAT fRadius = pSphere->m_fRadius;

		if ( fDistanceSq >= fRadius * fRadius || fDistanceSq < s_fEpsilon )
			return TFALSE;
	}

	if ( !a_pContacts )
		return TTRUE;

	if ( bInside )
	{
		// Push out along the axis the centre is the furthest on
		TINT   iAxis     = 0;
		TFLOAT fDistance = -1.0f;

		TFLOAT fAbs = TMath::Abs( afLocal[ 0 ] );
		if ( fAbs > -1.0f ) fDistance = fAbs;

		fAbs = TMath::Abs( afLocal[ 1 ] );
		if ( fAbs > fDistance )
		{
			fDistance = fAbs;
			iAxis     = 1;
		}

		fAbs = TMath::Abs( afLocal[ 2 ] );
		if ( fAbs > fDistance )
		{
			fDistance = fAbs;
			iAxis     = 2;
		}

		TVector3 vecLocalNormal        = TVector3::VEC_ZERO;
		( &vecLocalNormal.x )[ iAxis ] = ( ( afLocal[ iAxis ] > 0.0f ) ? 1.0f : -1.0f ) * -1.0f;

		TVector3 vecNormal;
		TMatrix44::RotateVector( vecNormal, a_rcTransform2, vecLocalNormal );

		a_pContacts->AddContact( rcSpherePos, vecNormal, ( &vecExtents.x )[ iAxis ] - fDistance + pSphere->m_fRadius );
		return TTRUE;
	}

	TFLOAT fDistance = TMath::Sqrt( fDistanceSq );

	// NOTE: the normal points from the centre of the sphere to the centre of the box
	TVector4 vecNormal( vecDelta.x * -1.0f, vecDelta.y * -1.0f, vecDelta.z * -1.0f, 1.0f );
	vecNormal.Normalise();

	TVector3 vecPosition(
	    vecNormal.x * fDistance + rcSpherePos.x,
	    vecNormal.y * fDistance + rcSpherePos.y,
	    vecNormal.z * fDistance + rcSpherePos.z
	);

	a_pContacts->AddContact( vecPosition, vecNormal.AsVector3(), pSphere->m_fRadius - fDistance );
	return TTRUE;
}

// $Barnyard: FUNCTION 0057a9b0
TBOOL APrim::IntersectSphereCapsule( APrim* a_pPrim1, const TMatrix44& a_rcTransform1, APrim* a_pPrim2, const TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts )
{
	APrimCapsule* pCapsule = TSTATICCAST( APrimCapsule, a_pPrim2 );

	TVector3 vecAxis;
	TMatrix44::RotateVector( vecAxis, a_rcTransform2, TVector3::VEC_POSZ );

	const TVector3& rcSpherePos = a_rcTransform1.GetTranslation3();

	TFLOAT fDX = rcSpherePos.x - a_rcTransform2.m_f41;
	TFLOAT fDY = rcSpherePos.y - a_rcTransform2.m_f42;
	TFLOAT fDZ = rcSpherePos.z - a_rcTransform2.m_f43;

	// Closest point on the axis
	TFLOAT fT          = vecAxis.z * fDZ + vecAxis.y * fDY + vecAxis.x * fDX;
	TFLOAT fHalfLength = pCapsule->m_fLength * 0.5f;
	TMath::Clip( fT, -fHalfLength, fHalfLength );

	TVector3 vecClosest(
	    vecAxis.x * fT + a_rcTransform2.m_f41,
	    vecAxis.y * fT + a_rcTransform2.m_f42,
	    vecAxis.z * fT + a_rcTransform2.m_f43
	);

	return IntersectSphereSphere( rcSpherePos, TSTATICCAST( APrimSphere, a_pPrim1 )->m_fRadius, vecClosest, pCapsule->m_fRadius, a_pContacts );
}

// $Barnyard: FUNCTION 00579f10
TBOOL APrim::IntersectComposite( APrim* a_pPrim1, const TMatrix44& a_rcTransform1, APrim* a_pPrim2, const TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts )
{
	APrimComposite* pComposite = TSTATICCAST( APrimComposite, a_pPrim1 );
	TBOOL           bResult    = TFALSE;
	TINT            iNumPrims  = pComposite->m_aElements.Size();

	for ( TINT i = 0; i < iNumPrims; i++ )
	{
		APrimComposite::Element& rElement = pComposite->m_aElements[ i ];

		TMatrix44 oTransform;
		oTransform = a_rcTransform1;
		oTransform.Multiply( rElement.oTransform );

		// NOTE: the contacts aren't passed on
		bResult |= Intersect( rElement.pPrim, oTransform, a_pPrim2, a_rcTransform2 );

		if ( !a_pContacts && bResult )
			break;
	}

	return bResult;
}

// $Barnyard: FUNCTION 0057aaa0
TFLOAT APrim::IntersectRaySphere( APrim* a_pPrim, const TMatrix44& a_rcTransform, const TVector3& a_rcOrigin, const TVector3& a_rcDirection, TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused )
{
	TFLOAT fLength = a_rcDirection.Magnitude();

	if ( fLength <= s_fEpsilon )
		return -1.0f;

	TFLOAT   fInvLength = 1.0f / fLength;
	TVector3 vecDirection( a_rcDirection.x * fInvLength, a_rcDirection.y * fInvLength, a_rcDirection.z * fInvLength );

	TFLOAT fNear;
	TFLOAT fFar;
	TINT   iNumHits = TMath::RayVsSphere( a_rcOrigin, vecDirection, a_rcTransform.GetTranslation3(), TSTATICCAST( APrimSphere, a_pPrim )->m_fRadius, fNear, fFar );

	if ( iNumHits < 1 )
		return -1.0f;

	a_pHits->Push( fNear );

	if ( iNumHits >= 2 )
		a_pHits->Push( fFar );

	return fNear;
}

// $Barnyard: FUNCTION 0057ab80
TFLOAT APrim::IntersectRayBox( APrim* a_pPrim, const TMatrix44& a_rcTransform, const TVector3& a_rcOrigin, const TVector3& a_rcDirection, TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused )
{
	APrimBox* pBox = TSTATICCAST( APrimBox, a_pPrim );

	// The ray in the space of the box
	TMatrix44 oInvTransform = a_rcTransform;
	oInvTransform.InvertOrthogonal();

	TVector3 vecOrigin;
	TMatrix44::TransformVector( vecOrigin, oInvTransform, a_rcOrigin );

	TVector3 vecDirection;
	TMatrix44::RotateVector( vecDirection, oInvTransform, a_rcDirection );

	TFLOAT          fNear     = -TMath::MAXFLOAT;
	TFLOAT          fFar      = TMath::MAXFLOAT;
	const TVector3& rcExtents = pBox->m_vExtents;

	if ( !ClipSlab( fFar, fNear, vecDirection.x, -vecOrigin.x - rcExtents.x ) ) return -1.0f;
	if ( !ClipSlab( fFar, fNear, -vecDirection.x, vecOrigin.x - rcExtents.x ) ) return -1.0f;
	if ( !ClipSlab( fFar, fNear, vecDirection.y, -vecOrigin.y - rcExtents.y ) ) return -1.0f;
	if ( !ClipSlab( fFar, fNear, -vecDirection.y, vecOrigin.y - rcExtents.y ) ) return -1.0f;
	if ( !ClipSlab( fFar, fNear, vecDirection.z, -vecOrigin.z - rcExtents.z ) ) return -1.0f;
	if ( !ClipSlab( fFar, fNear, -vecDirection.z, vecOrigin.z - rcExtents.z ) ) return -1.0f;

	a_pHits->Push( fNear );
	a_pHits->Push( fFar );

	return fNear;
}

// $Barnyard: FUNCTION 00579ac0
TINT APrim::RayVsCapsule( APrimCapsule* a_pCapsule, const TMatrix44& a_rcTransform, const TVector3& a_rcOrigin, const TVector3& a_rcRay, TFLOAT* a_pOutDistances )
{
	TFLOAT fLength = a_rcRay.Magnitude();

	if ( fLength < s_fEpsilon )
		return 0;

	// The ray in the space of the capsule (the axis goes from 0 to fLength along Z)
	TMatrix44 oInvTransform = a_rcTransform;
	oInvTransform.InvertOrthogonal();

	TVector3 vecOrigin;
	TMatrix44::TransformVector( vecOrigin, oInvTransform, a_rcOrigin );

	TVector3 vecRay;
	TMatrix44::RotateVector( vecRay, oInvTransform, a_rcRay );

	TFLOAT fInvLength = 1.0f / fLength;
	TFLOAT fRadiusSq  = a_pCapsule->m_fRadius * a_pCapsule->m_fRadius;

	// NOTE: m_fLength isn't used, the axis is as long as the ray (retail bug)
	if ( 1.0f - s_fEpsilon <= TMath::Abs( vecRay.z ) )
	{
		// Parallel to the axis
		TFLOAT fDiscriminant = fRadiusSq - ( vecOrigin.y * vecOrigin.y + vecOrigin.x * vecOrigin.x );

		if ( fDiscriminant < 0.0f )
			return 0;

		TFLOAT fRoot = TMath::Sqrt( fDiscriminant );

		a_pOutDistances[ 0 ] = -( ( vecOrigin.z + fRoot ) * fInvLength );
		a_pOutDistances[ 1 ] = ( fLength - vecOrigin.z + fRoot ) * fInvLength;
		return 2;
	}

	// The infinite cylinder
	TFLOAT fA = vecRay.y * vecRay.y + vecRay.x * vecRay.x;
	TFLOAT fB = vecRay.y * vecOrigin.y + vecRay.x * vecOrigin.x;
	TFLOAT fC = ( vecOrigin.y * vecOrigin.y + vecOrigin.x * vecOrigin.x ) - fRadiusSq;

	TFLOAT fDiscriminant = fB * fB - fC * fA;

	if ( fDiscriminant < 0.0f )
		return 0;

	TINT iNumHits = 0;

	if ( fDiscriminant > 0.0f )
	{
		TFLOAT fRoot = TMath::Sqrt( fDiscriminant );
		TFLOAT fInvA = 1.0f / fA;

		TFLOAT fT = ( -fB - fRoot ) * fInvA;
		TFLOAT fZ = fT * vecRay.z + vecOrigin.z;

		if ( 0.0f <= fZ && fZ <= fLength )
		{
			iNumHits             = 1;
			a_pOutDistances[ 0 ] = fT * fInvLength;
		}

		fT = ( fRoot - fB ) * fInvA;
		fZ = fT * vecRay.z + vecOrigin.z;

		if ( 0.0f <= fZ && fZ <= fLength )
			a_pOutDistances[ iNumHits++ ] = fT * fInvLength;

		if ( iNumHits == 2 )
			return iNumHits;
	}
	else
	{
		TFLOAT fT = -( fB / fA );
		TFLOAT fZ = fT * vecRay.z + vecOrigin.z;

		if ( 0.0f <= fZ && fZ <= fLength )
		{
			a_pOutDistances[ 0 ] = fT * fInvLength;
			return 1;
		}
	}

	// The sphere at the start of the axis
	// NOTE: assumes a normalised ray (no fA)
	fB = vecOrigin.z * vecRay.z + fB;
	fC = vecOrigin.z * vecOrigin.z + fC;

	TFLOAT fCapDiscriminant = fB * fB - fC;

	if ( fCapDiscriminant > 0.0f )
	{
		TFLOAT fRoot = TMath::Sqrt( fCapDiscriminant );
		TFLOAT fT    = -fB - fRoot;

		if ( vecRay.z * fT + vecOrigin.z <= 0.0f )
		{
			a_pOutDistances[ iNumHits++ ] = fT * fInvLength;
			if ( iNumHits == 2 ) return iNumHits;
		}

		fT = fRoot - fB;

		if ( vecRay.z * fT + vecOrigin.z <= 0.0f )
		{
			a_pOutDistances[ iNumHits++ ] = fT * fInvLength;
			if ( iNumHits == 2 ) return iNumHits;
		}
	}
	else if ( fCapDiscriminant == 0.0f )
	{
		TFLOAT fT = -fB;

		if ( vecRay.z * fT + vecOrigin.z <= 0.0f )
		{
			a_pOutDistances[ iNumHits++ ] = fT * fInvLength;
			if ( iNumHits == 2 ) return iNumHits;
		}
	}

	// The sphere at the end of the axis
	fB = fB - vecRay.z * fLength;
	fC = ( fLength - ( vecOrigin.z + vecOrigin.z ) ) * fLength + fC;

	fCapDiscriminant = fB * fB - fC;

	if ( fCapDiscriminant > 0.0f )
	{
		TFLOAT fRoot = TMath::Sqrt( fCapDiscriminant );
		TFLOAT fT    = -fB - fRoot;

		if ( vecRay.z * fT + vecOrigin.z >= fLength )
		{
			a_pOutDistances[ iNumHits++ ] = fT * fInvLength;
			if ( iNumHits == 2 ) return iNumHits;
		}

		fT = fRoot - fB;

		if ( vecRay.z * fT + vecOrigin.z >= fLength )
			a_pOutDistances[ iNumHits++ ] = fT * fInvLength;
	}
	else if ( fCapDiscriminant == 0.0f )
	{
		TFLOAT fT = -fB;

		if ( vecRay.z * fT + vecOrigin.z >= fLength )
			a_pOutDistances[ iNumHits++ ] = fT * fInvLength;
	}

	return iNumHits;
}

// $Barnyard: FUNCTION 0057ad00
TFLOAT APrim::IntersectRayCapsule( APrim* a_pPrim, const TMatrix44& a_rcTransform, const TVector3& a_rcOrigin, const TVector3& a_rcDirection, TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused )
{
	APrimCapsule* pCapsule = TSTATICCAST( APrimCapsule, a_pPrim );

	if ( pCapsule->m_fLength < s_fEpsilon )
	{
		// No length, it's a sphere
		APrimSphere oSphere( pCapsule->m_fRadius );
		return IntersectRaySphere( &oSphere, a_rcTransform, a_rcOrigin, a_rcDirection, a_pHits, a_fMaxDistance, a_iUnused );
	}

	TVector3 vecRay( a_fMaxDistance * a_rcDirection.x, a_fMaxDistance * a_rcDirection.y, a_fMaxDistance * a_rcDirection.z );

	TFLOAT afDistances[ 2 ];
	TINT   iNumHits = RayVsCapsule( pCapsule, a_rcTransform, a_rcOrigin, vecRay, afDistances );

	TBOOL  bFound   = TFALSE;
	TFLOAT fClosest = TMath::MAXFLOAT;

	for ( TINT i = 0; i < iNumHits; i++ )
	{
		a_pHits->Push( afDistances[ i ] );

		if ( afDistances[ i ] < fClosest && afDistances[ i ] >= 0.0f )
		{
			bFound   = TTRUE;
			fClosest = afDistances[ i ];
		}
	}

	return bFound ? fClosest : -1.0f;
}

// $Barnyard: FUNCTION 0057a000
TFLOAT APrim::IntersectRayComposite( APrim* a_pPrim, const TMatrix44& a_rcTransform, const TVector3& a_rcOrigin, const TVector3& a_rcDirection, TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused )
{
	APrimComposite* pComposite = TSTATICCAST( APrimComposite, a_pPrim );

	TFLOAT fClosest = TMath::MAXFLOAT;
	TBOOL  bFound   = TFALSE;

	if ( pComposite->m_aElements.Size() <= 0 )
		return -1.0f;

	for ( TINT i = 0; i < pComposite->m_aElements.Size(); i++ )
	{
		APrimComposite::Element& rElement = pComposite->m_aElements[ i ];

		TMatrix44 oTransform;
		oTransform = a_rcTransform;
		oTransform.Multiply( rElement.oTransform );

		TFLOAT fDistance = IntersectRay( rElement.pPrim, oTransform, a_rcOrigin, a_rcDirection, a_pHits, a_fMaxDistance, a_iUnused );

		if ( fDistance > 0.0f && fDistance < fClosest )
		{
			fClosest = fDistance;
			bFound   = TTRUE;
		}
	}

	return bFound ? fClosest : -1.0f;
}

// NOTE: inlined into CreateObject / CreateObjectInPlace
APrimSphere::APrimSphere()
    : APrim( TYPE_SPHERE )
    , m_fRadius( 0.0f )
{
}

// $Barnyard: FUNCTION 0057af70
APrimSphere::APrimSphere( TFLOAT a_fRadius )
    : APrim( TYPE_SPHERE )
    , m_fRadius( a_fRadius )
{
}

// $Barnyard: FUNCTION 0057b320
// $Barnyard: FUNCTION 0057af90
APrimSphere::~APrimSphere()
{
}

// $Barnyard: FUNCTION 0057afa0
TVector4 APrimSphere::GetBoundingSphere()
{
	TVector4 vecSphere;
	vecSphere.x = TVector3::VEC_ZERO.x;
	vecSphere.y = TVector3::VEC_ZERO.y;
	vecSphere.z = TVector3::VEC_ZERO.z;
	vecSphere.w = m_fRadius;

	return vecSphere;
}

// $Barnyard: FUNCTION 0057afd0
APrimBox::APrimBox()
    : APrim( TYPE_BOX )
    , m_vUnk08( TVector3::VEC_ZERO )
    , m_vExtents( TVector3::VEC_ZERO )
{
}

// $Barnyard: FUNCTION 0057b340
// $Barnyard: FUNCTION 0057b020
APrimBox::~APrimBox()
{
}

// $Barnyard: FUNCTION 0057b360
TVector4 APrimBox::GetBoundingSphere()
{
	TFLOAT fZ = TMath::Abs( m_vExtents.z );
	TFLOAT fY = TMath::Abs( m_vExtents.y );
	TFLOAT fX = TMath::Abs( m_vExtents.x );

	const TFLOAT* pMax = ( fY >= fZ ) ? &fY : &fZ;
	if ( fX >= *pMax ) pMax = &fX;

	TVector4 vecSphere;
	vecSphere.x = TVector3::VEC_ZERO.x;
	vecSphere.y = TVector3::VEC_ZERO.y;
	vecSphere.z = TVector3::VEC_ZERO.z;
	vecSphere.w = *pMax;

	return vecSphere;
}

// NOTE: inlined into CreateObject / CreateObjectInPlace
APrimCapsule::APrimCapsule()
    : APrim( TYPE_CAPSULE )
    , m_fRadius( 0.0f )
    , m_fLength( 0.0f )
{
}

// $Barnyard: FUNCTION 0057b3d0
// $Barnyard: FUNCTION 0057b060
APrimCapsule::~APrimCapsule()
{
}

// $Barnyard: FUNCTION 0057b070
TVector4 APrimCapsule::GetBoundingSphere()
{
	TVector4 vecSphere;
	vecSphere.x = TVector3::VEC_ZERO.x;
	vecSphere.y = TVector3::VEC_ZERO.y;
	vecSphere.z = TVector3::VEC_ZERO.z;
	vecSphere.w = m_fLength * 0.5f + m_fRadius;

	return vecSphere;
}

// $Barnyard: FUNCTION 0057b3f0
APrimComposite::APrimComposite()
    : APrim( TYPE_COMPOSITE )
    , m_aElements( 10, 0 )
{
}

// $Barnyard: FUNCTION 0057b440
// $Barnyard: FUNCTION 0057b460
APrimComposite::~APrimComposite()
{
}

// $Barnyard: FUNCTION 0057b030
TVector4 APrimComposite::GetBoundingSphere()
{
	TVector4 vecSphere;
	vecSphere.x = TVector3::VEC_ZERO.x;
	vecSphere.y = TVector3::VEC_ZERO.y;
	vecSphere.z = TVector3::VEC_ZERO.z;
	vecSphere.w = 0.0f;

	return vecSphere;
}
