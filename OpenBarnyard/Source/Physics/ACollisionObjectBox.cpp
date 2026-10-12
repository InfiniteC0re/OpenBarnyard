#include "pch.h"
#include "ACollisionObjectBox.h"
#include "ACollisionObjectModel.h"
#include "ACollisionObjectMesh.h"
#include "ACollisionModelInstance.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// GetClass: 0061d580
// NOTE: GetClass 0061d580, CreateObject 0061d950, CreateObjectInPlace 0061d9c0
TDEFINE_CLASS( ACollisionObjectBox );

// $Barnyard: FUNCTION 0061d5f0
ACollisionObjectBox::ACollisionObjectBox()
{
	m_eType = TYPE_SIMPLE;
}

// $Barnyard: FUNCTION 0061d6c0
// $Barnyard: FUNCTION 0061da20
// $Barnyard: FUNCTION 0061d6b0
ACollisionObjectBox::~ACollisionObjectBox()
{
	if ( IsCollisionObjectLinked() )
		UnlinkCollisionObject();
}

// $Barnyard: FUNCTION 0061d5a0
void ACollisionObjectBox::SetExtents( const TVector4& a_rcExtents )
{
	m_oBox.m_vExtents = a_rcExtents;
	CreateHash();
}

// $Barnyard: FUNCTION 0061d5d0
void ACollisionObjectBox::SetTransform( const TMatrix44& a_rcTransform )
{
	m_oBox.m_oTransform = a_rcTransform;
	CreateHash();
}

// $Barnyard: FUNCTION 0061d670
TFLOAT ACollisionObjectBox::GetRadius()
{
	return m_oBox.m_fRadius;
}

const TVector4& ACollisionObjectBox::GetPosition()
{
	// NOTE: the same function as GetObjectPosition (0061d660)
	return m_oBox.GetCentre();
}

// $Barnyard: FUNCTION 0061d590
TBOOL ACollisionObjectBox::CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	return TFALSE;
}

// $Barnyard: FUNCTION 0061dda0
TBOOL ACollisionObjectBox::CollideSphere( ACollisionObject* a_pObstacle, const TVector4& a_rcCenter, TFLOAT& a_rfDistance, TVector4& a_rNormal )
{
	return TFALSE;
}

// $Barnyard: FUNCTION 0061d730
TBOOL ACollisionObjectBox::CollideShape( void* a_pOther, void* a_pShape, void* a_pHitList, void* a_pObjects, void* a_pUnk5, void* a_pUnk6 )
{
	ACollisionObject*          pOther   = TSTATICCAST( ACollisionObject, a_pOther );
	const TOBBox*              pBox     = TSTATICCAST( const TOBBox, a_pShape );
	ACollisionModelRayHitList* pHitList = TSTATICCAST( ACollisionModelRayHitList, a_pHitList );
	ObjectList*                pObjects = TSTATICCAST( ObjectList, a_pObjects );
	TBOOL                      bResult;

	if ( ACollisionModelInstance* pModelInstance = pOther->GetModelInstance() )
	{
		// Make sure the model instance uses the transform of the object
		if ( pOther->IsA( &TGetClass( ACollisionObjectModel ) ) == TTRUE )
		{
			// NOTE: ACollisionObjectModel isn't reconstructed yet, its transform object is at 0x44
			pModelInstance->SetTransformObject( TREINTERPRETCAST( TTransformObject*, TREINTERPRETCAST( TUINTPTR, pOther ) + 0x44 ) );
		}
		else if ( pOther->IsA( &TGetClass( ACollisionObjectMesh ) ) == TTRUE )
		{
			pModelInstance->SetTransformObject( &pModelInstance->GetSceneObject()->GetTransform() );
		}

		bResult = pModelInstance->CollideSweptBox( *pBox, *pBox, pHitList, -1, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );
	}
	else if ( pOther->GetClass() == &TGetClass( ACollisionObjectBox ) )
	{
		bResult = TOBBox::IntersectsOBB( *pBox, TSTATICCAST( ACollisionObjectBox, pOther )->m_oBox );
	}
	else if ( pOther->GetHeightScale() > 0.0f )
	{
		// Vertical capsule
		TVector4 vDirection = TVector4::VEC_NEGY;
		TFLOAT   fLength    = pOther->GetHeightScale();

		const TVector4& rcPosition = pOther->GetObjectPosition();
		TVector4        vPosition( rcPosition.x, rcPosition.y, rcPosition.z, 1.0f );

		TVector4 vOffset;
		vOffset.Multiply( vDirection, fLength );
		vOffset.Multiply( vOffset, 0.5f );

		TVector4 vCentre;
		vCentre.Add( vPosition, vOffset );

		TCapsule oCapsule( vCentre, vDirection, pOther->GetObjectRadius(), fLength );
		bResult = TOBBox::IntersectsCapsule( *pBox, oCapsule );
	}
	else
	{
		TFLOAT          fRadius    = pOther->GetObjectRadius();
		const TVector4& rcPosition = pOther->GetObjectPosition();
		TVector4        vSphere( rcPosition.x, rcPosition.y, rcPosition.z, fRadius );

		bResult = TOBBox::IntersectsSphere( *pBox, vSphere );
	}

	if ( bResult == TTRUE && pObjects != TNULL )
		pObjects->apObjects[ pObjects->iNumObjects++ ] = pOther;

	return bResult;
}

// $Barnyard: FUNCTION 0061d650
const TSphere& ACollisionObjectBox::GetObjectSphere()
{
	return m_oSphere;
}

// NOTE: shared with ACollisionObjectMesh::GetObjectPosition (0061d660)
const TVector4& ACollisionObjectBox::GetObjectPosition()
{
	return m_oBox.GetCentre();
}

TFLOAT ACollisionObjectBox::GetObjectRadius()
{
	// NOTE: the same function as GetRadius (0061d670)
	return m_oBox.m_fRadius;
}

// $Barnyard: FUNCTION 0061d680
void ACollisionObjectBox::GetVelocity( TVector4& a_rOutVector )
{
	a_rOutVector = TVector4::VEC_ZERO;
}
