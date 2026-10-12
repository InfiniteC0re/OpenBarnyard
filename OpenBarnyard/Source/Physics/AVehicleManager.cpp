#include "pch.h"
#include "AVehicleManager.h"
#include "APhysicsBike.h"
#include "APhysicsCar.h"
#include "ACollisionManager.h"
#include "ACollisionObjectSimple.h"
#include "ACollisionObjectBox.h"
#include "ACollisionModelInstance.h"
#include "Terrain/ATerrainInterface.h"

#include <Toshi/T2ObjectPool.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Pools of the vehicles (retail object at 0x007bdf88 constructed by 0065b9a0 also holds
// AVehicleManager::ms_vecBodies and AVehicleManager::ms_vecVehicles)
// $Barnyard: FUNCTION 0065b9a0
// $Barnyard: FUNCTION 0065a280

// T2ObjectPool<APhysicsCar, 8, 3644> methods (vtables 0x0073aea4, 0x0073aef4):
// $Barnyard: FUNCTION 0065a0c0
// $Barnyard: FUNCTION 0065a0e0
// $Barnyard: FUNCTION 0065a0f0
// $Barnyard: FUNCTION 0065a160
// $Barnyard: FUNCTION 0065a260
// $Barnyard: FUNCTION 0065b960
static T2ObjectPool<APhysicsCar, AVehicleManager::MAX_NUM_VEHICLES, 3644, 4> s_oCarPool;

// T2ObjectPool<APhysicsBike, 8, 4044> methods (vtables 0x0073aecc, 0x0073af1c):
// $Barnyard: FUNCTION 0065a110
// $Barnyard: FUNCTION 0065a130
// $Barnyard: FUNCTION 0065a140
// $Barnyard: FUNCTION 0065a180
// $Barnyard: FUNCTION 0065a270
// $Barnyard: FUNCTION 0065b980
static T2ObjectPool<APhysicsBike, AVehicleManager::MAX_NUM_VEHICLES, 4044, 4> s_oBikePool;

// $Barnyard: FUNCTION 0065e650
void AVehicleManager::UpdatePhysics( TFLOAT a_fDeltaTime )
{
	ms_iNumUsedContacts = 0;

	ClearContacts( a_fDeltaTime );
	CollideAllBodies( a_fDeltaTime );
	CollideAllShapes( a_fDeltaTime );
	ResolveAllContacts( a_fDeltaTime );
	IntegrateAll( a_fDeltaTime );
	SendContactEvents();
	PostUpdateVehicles( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0065b850
void AVehicleManager::AddRigidBody( ARigidBody* a_pBody )
{
	ms_vecBodies.PushBack( a_pBody );
}

// $Barnyard: FUNCTION 0065d820
void AVehicleManager::RemoveRigidBody( ARigidBody* a_pBody )
{
	ms_vecBodies.FindAndErase( a_pBody );
}

// $Barnyard: FUNCTION 0065b870
void AVehicleManager::AddVehicle( APhysicsVehicle* a_pVehicle )
{
	ms_vecVehicles.PushBack( a_pVehicle );
}

// $Barnyard: FUNCTION 0065d890
void AVehicleManager::RemoveVehicle( APhysicsVehicle* a_pVehicle )
{
	ms_vecVehicles.FindAndErase( a_pVehicle );
}

// $Barnyard: FUNCTION 0065b900
// $Barnyard: FUNCTION 0065a1a0
APhysicsCar* AVehicleManager::CreateCar()
{
	return s_oCarPool.NewObject();
}

// $Barnyard: FUNCTION 0065b910
void AVehicleManager::DestroyCar( APhysicsCar* a_pCar )
{
	s_oCarPool.DeleteObject( a_pCar );
}

// $Barnyard: FUNCTION 0065b930
// $Barnyard: FUNCTION 0065a200
APhysicsBike* AVehicleManager::CreateBike()
{
	return s_oBikePool.NewObject();
}

// $Barnyard: FUNCTION 0065b940
void AVehicleManager::DestroyBike( APhysicsBike* a_pBike )
{
	s_oBikePool.DeleteObject( a_pBike );
}

// $Barnyard: FUNCTION 00659f60
void AVehicleManager::SetNumIterations( TINT a_iNumIterations )
{
	ms_iNumIterations = a_iNumIterations;
}

// $Barnyard: FUNCTION 00659f70
TINT AVehicleManager::GetNumIterations()
{
	return ms_iNumIterations;
}

// $Barnyard: FUNCTION 00659f80
void AVehicleManager::SetSlideOnObjects( TBOOL a_bSlide )
{
	ms_bSlideOnObjects = a_bSlide;
}

// $Barnyard: FUNCTION 00659f90
void AVehicleManager::SetUseTriangleNormal( TBOOL a_bUse )
{
	ms_bUseTriangleNormal = a_bUse;
}

ARigidBodyContact* AVehicleManager::AllocateContact()
{
	if ( ms_iNumUsedContacts < ms_iMaxNumContacts )
	{
		ARigidBodyContact* pContact = &ms_pContacts[ ms_iNumUsedContacts++ ];
		pContact->Reset();

		return pContact;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0065b690
void AVehicleManager::ClearContacts( TFLOAT a_fDeltaTime )
{
	TINT iNumBodies = ms_vecBodies.Size();

	for ( TINT i = 0; i < iNumBodies; i++ )
	{
		ARigidBody* pBody = ms_vecBodies[ i ];

		if ( pBody->m_bEnabled )
		{
			for ( TINT k = 0; k < pBody->m_iNumContacts; k++ )
				pBody->m_apContacts[ k ] = TNULL;

			pBody->m_iNumContacts = 0;
		}
	}

	TINT iNumVehicles = ms_vecVehicles.Size();

	for ( TINT i = 0; i < iNumVehicles; i++ )
	{
		if ( ms_vecVehicles[ i ]->m_bEnabled )
			ms_vecVehicles[ i ]->UseCentreOfMass2();
	}
}

// $Barnyard: FUNCTION 0065b720
void AVehicleManager::IntegrateAll( TFLOAT a_fDeltaTime )
{
	TINT iNumVehicles = ms_vecVehicles.Size();

	for ( TINT i = 0; i < iNumVehicles; i++ )
	{
		if ( ms_vecVehicles[ i ]->m_bEnabled )
			ms_vecVehicles[ i ]->UseCentreOfMass1();
	}

	for ( TINT i = 0; i < iNumVehicles; i++ )
	{
		if ( ms_vecVehicles[ i ]->m_bEnabled )
			ms_vecVehicles[ i ]->PreUpdate( a_fDeltaTime );
	}

	TINT iNumBodies = ms_vecBodies.Size();

	for ( TINT i = 0; i < iNumBodies; i++ )
	{
		if ( ms_vecBodies[ i ]->m_bEnabled )
			ms_vecBodies[ i ]->Integrate( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 0065b7c0
void AVehicleManager::PostUpdateVehicles( TFLOAT a_fDeltaTime )
{
	TINT iNumVehicles = ms_vecVehicles.Size();

	for ( TINT i = 0; i < iNumVehicles; i++ )
	{
		if ( ms_vecVehicles[ i ]->m_bEnabled )
			ms_vecVehicles[ i ]->PostUpdate( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 0065b810
void AVehicleManager::ResolveAllContacts( TFLOAT a_fDeltaTime )
{
	TINT iNumBodies = ms_vecBodies.Size();

	for ( TINT i = 0; i < iNumBodies; i++ )
	{
		ARigidBody* pBody = ms_vecBodies[ i ];

		if ( pBody->m_bEnabled )
		{
			ContactSummary oSummary;
			ResolveContacts( pBody, &oSummary );
		}
	}
}

// $Barnyard: FUNCTION 0065b890
void AVehicleManager::SendContactEvents()
{
	TINT iNumBodies = ms_vecBodies.Size();

	for ( TINT i = 0; i < iNumBodies; i++ )
	{
		ARigidBody*             pBody    = ms_vecBodies[ i ];
		ARigidBodyEventHandler* pHandler = pBody->m_pEventHandler;

		if ( pHandler && pBody->m_iNumContacts > 0 )
		{
			for ( TINT k = 0; k < pBody->m_iNumContacts; k++ )
			{
				ARigidBodyContact* pContact = pBody->m_apContacts[ k ];

				if ( pContact->m_pOtherBody )
					pHandler->OnBodyContact( pContact );
				else
					pHandler->OnStaticContact( pContact );
			}
		}
	}
}

// $Barnyard: FUNCTION 0065e330
void AVehicleManager::CollideAllShapes( TFLOAT a_fDeltaTime )
{
	TINT iNumBodies = ms_vecBodies.Size();

	for ( TINT i = 0; i < iNumBodies; i++ )
	{
		if ( ms_vecBodies[ i ]->m_bEnabled )
			CollideShape( ms_vecBodies[ i ], a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 0065e5b0
void AVehicleManager::CollideAllBodies( TFLOAT a_fDeltaTime )
{
	TINT iNumBodies = ms_vecBodies.Size();

	for ( TINT i = 0; i < iNumBodies - 1; i++ )
	{
		ARigidBody* pBody1 = ms_vecBodies[ i ];

		if ( pBody1->m_bEnabled )
		{
			// NOTE: starts at 1 and not at i + 1 in the original, so most pairs are tested twice
			for ( TINT k = 1; k < iNumBodies; k++ )
			{
				ARigidBody* pBody2 = ms_vecBodies[ k ];

				if ( pBody2->m_bEnabled && pBody1 != pBody2 && pBody1->m_oShape.m_eType == ARigidBodyShape::TYPE_BOX && BoundingSpheresOverlap( pBody1, pBody2 ) )
					CollideBodies( pBody1, pBody2, a_fDeltaTime );
			}
		}
	}
}

// $Barnyard: FUNCTION 00659fa0
TBOOL AVehicleManager::BoundingSpheresOverlap( ARigidBody* a_pBody1, ARigidBody* a_pBody2 )
{
	TVector4 vCentre1 = a_pBody1->m_oShape.GetBoundingCentre();
	TMatrix44::TransformVector( vCentre1, a_pBody1->m_oTransform, vCentre1 );

	TVector4 vCentre2 = a_pBody2->m_oShape.GetBoundingCentre();
	TMatrix44::TransformVector( vCentre2, a_pBody2->m_oTransform, vCentre2 );

	TFLOAT fRadius2 = a_pBody2->m_oShape.GetBoundingRadius();
	TFLOAT fRadius  = a_pBody1->m_oShape.GetBoundingRadius() + fRadius2;

	TVector4 vDistance;
	vDistance.Substract( vCentre1, vCentre2 );

	return fRadius * fRadius > vDistance.MagnitudeSq();
}

// $Barnyard: FUNCTION 0065db80
TBOOL AVehicleManager::CollideShape( ARigidBody* a_pBody, TFLOAT a_fDeltaTime )
{
	ARigidBodyShape& rShape = a_pBody->m_oShape;

	if ( rShape.m_eType == ARigidBodyShape::TYPE_SPHERE )
		return CollideSphere( a_pBody, rShape.m_aPoints[ 0 ], a_fDeltaTime );

	if ( rShape.m_eType == ARigidBodyShape::TYPE_POINTS )
	{
		TVector4 aPoints[ 8 ];
		TINT     iNumPoints = TMath::Min( rShape.m_iNumPoints, 8 );

		if ( iNumPoints > 0 )
			TUtil::MemCopy( aPoints, rShape.m_aPoints, iNumPoints * sizeof( TVector4 ) );

		return CollidePoints( a_pBody, aPoints, iNumPoints, a_fDeltaTime );
	}

	if ( rShape.m_eType == ARigidBodyShape::TYPE_BOX )
		return CollideBox( a_pBody, rShape.m_oBox, a_fDeltaTime, TTRUE );

	return TFALSE;
}

// $Barnyard: FUNCTION 0065d9a0
TBOOL AVehicleManager::CollideSphereAndResolve( ARigidBody* a_pBody, const TVector4& a_rcSphere, TFLOAT a_fDeltaTime, ContactSummary* a_pSummary )
{
	CollideSphere( a_pBody, a_rcSphere, a_fDeltaTime );
	return ResolveContacts( a_pBody, a_pSummary );
}

// $Barnyard: FUNCTION 0065d9d0
TBOOL AVehicleManager::CollidePointsAndResolve( ARigidBody* a_pBody, const TVector4* a_pPoints, TINT a_iNumPoints, TFLOAT a_fDeltaTime, ContactSummary* a_pSummary )
{
	CollidePoints( a_pBody, a_pPoints, a_iNumPoints, a_fDeltaTime );
	return ResolveContacts( a_pBody, a_pSummary );
}

// $Barnyard: FUNCTION 0065dc50
TBOOL AVehicleManager::CollideBoxAndResolve( ARigidBody* a_pBody, const TOBBox& a_rcBox, TFLOAT a_fDeltaTime, ContactSummary* a_pSummary, TBOOL a_bSwept )
{
	CollideBox( a_pBody, a_rcBox, a_fDeltaTime, a_bSwept );
	return ResolveContacts( a_pBody, a_pSummary );
}

// $Barnyard: FUNCTION 0065ba30
TBOOL AVehicleManager::CollideSphere( ARigidBody* a_pBody, const TVector4& a_rcSphere, TFLOAT a_fDeltaTime )
{
	TMatrix44 oPrevTransform = a_pBody->m_oPrevTransform;

	TVector4 vCentre( a_rcSphere.x, a_rcSphere.y, a_rcSphere.z, 1.0f );
	TFLOAT   fRadius = a_rcSphere.w;

	TVector4 vPrevCentre;
	TMatrix44::TransformVector( vPrevCentre, oPrevTransform, vCentre );

	for ( TINT i = 0; i < ms_iNumIterations; i++ )
	{
		TMatrix44 oTransform = a_pBody->m_oTransform;

		TVector4 vNewCentre;
		TMatrix44::TransformVector( vNewCentre, oTransform, vCentre );

		TVector4 vMovement;
		vMovement.Substract( vPrevCentre, vNewCentre );

		if ( vMovement.Magnitude() > 0.0f )
		{
			ARigidBodyCollision oCollision;

			if ( CollideSweptSphere( a_pBody, &oCollision, vPrevCentre, vNewCentre, fRadius ) == TTRUE )
				AddStaticContact( a_pBody, &oCollision, a_fDeltaTime );
		}
	}

	return a_pBody->m_iNumContacts > 0;
}

// $Barnyard: FUNCTION 0065bbb0
TBOOL AVehicleManager::CollidePoints( ARigidBody* a_pBody, const TVector4* a_pPoints, TINT a_iNumPoints, TFLOAT a_fDeltaTime )
{
	TMatrix44 oPrevTransform = a_pBody->m_oPrevTransform;

	for ( TINT i = 0; i < ms_iNumIterations; i++ )
	{
		for ( TINT k = 0; k < a_iNumPoints; k++ )
		{
			TVector4 vCentre( a_pPoints[ k ].x, a_pPoints[ k ].y, a_pPoints[ k ].z, 1.0f );
			TFLOAT   fRadius = a_pPoints[ k ].w;

			TVector4 vPrevCentre;
			TMatrix44::TransformVector( vPrevCentre, oPrevTransform, vCentre );

			TMatrix44 oTransform = a_pBody->m_oTransform;

			TVector4 vNewCentre;
			TMatrix44::TransformVector( vNewCentre, oTransform, vCentre );

			TVector4 vMovement;
			vMovement.Substract( vPrevCentre, vNewCentre );

			if ( vMovement.Magnitude() > 0.0f )
			{
				ARigidBodyCollision oCollision;

				if ( CollideSweptSphere( a_pBody, &oCollision, vPrevCentre, vNewCentre, fRadius ) == TTRUE )
					AddStaticContact( a_pBody, &oCollision, a_fDeltaTime );
			}
		}
	}

	return a_pBody->m_iNumContacts > 0;
}

// $Barnyard: FUNCTION 0065da00
TBOOL AVehicleManager::CollideBox( ARigidBody* a_pBody, const TOBBox& a_rcBox, TFLOAT a_fDeltaTime, TBOOL a_bSwept )
{
	TMatrix44 oPrevTransform;
	TMatrix44 oTemp = a_pBody->m_oPrevTransform;
	oPrevTransform.Multiply( oTemp, a_rcBox.m_oTransform );

	TOBBox oPrevBox( oPrevTransform, a_rcBox.m_vExtents );

	for ( TINT i = 0; i < ms_iNumIterations; i++ )
	{
		TMatrix44 oTransform;
		TMatrix44 oBodyTransform = a_pBody->m_oTransform;
		oTransform.Multiply( oBodyTransform, a_rcBox.m_oTransform );

		ARigidBodyCollision oCollision;
		TOBBox              oBox( oTransform, a_rcBox.m_vExtents );

		TBOOL bCollided = ( a_bSwept == TTRUE ) ?
		    CollideBoxSwept( a_pBody, &oCollision, oPrevBox, oBox ) :
		    CollideBoxStatic( a_pBody, &oCollision, oPrevBox, oBox );

		if ( bCollided == TTRUE )
			AddStaticContact( a_pBody, &oCollision, a_fDeltaTime );
	}

	return a_pBody->m_iNumContacts > 0;
}

// $Barnyard: FUNCTION 0065a3d0
TBOOL AVehicleManager::ResolveContacts( ARigidBody* a_pBody, ContactSummary* a_pSummary )
{
	TINT iNumContacts = a_pBody->m_iNumContacts;

	if ( iNumContacts > 0 )
	{
		TVector4 vSumImpulse = TVector4::VEC_ZERO;
		TVector4 vSumOffset  = TVector4::VEC_ZERO;
		TVector4 vSumPushOut = TVector4::VEC_ZERO;
		TINT     iCount      = 0;

		for ( TINT i = 0; i < iNumContacts; i++ )
		{
			ARigidBodyContact* pContact = a_pBody->m_apContacts[ i ];

			if ( pContact->m_pOtherBody != TNULL || TMath::Abs( pContact->m_fImpulse ) > 0.0f )
			{
				iCount++;
				vSumImpulse.x += pContact->m_vImpulse.x;
				vSumImpulse.y += pContact->m_vImpulse.y;
				vSumImpulse.z += pContact->m_vImpulse.z;
				vSumOffset.x += pContact->m_vOffset.x;
				vSumOffset.y += pContact->m_vOffset.y;
				vSumOffset.z += pContact->m_vOffset.z;
				vSumPushOut.x += pContact->m_vPushOut.x;
				vSumPushOut.y += pContact->m_vPushOut.y;
				vSumPushOut.z += pContact->m_vPushOut.z;
			}
		}

		if ( iCount > 0 )
		{
			TFLOAT fScale = 1.0f / iCount;

			TVector4 vOffset;
			vOffset.Multiply( vSumOffset, fScale );

			TVector4 vImpulse;
			vImpulse.Multiply( vSumImpulse, fScale );

			TVector4 vPoint;
			vPoint.Multiply( vOffset, a_pBody->m_fContactScale );
			vPoint.x += a_pBody->m_vCentreOfMass.x;
			vPoint.y += a_pBody->m_vCentreOfMass.y;
			vPoint.z += a_pBody->m_vCentreOfMass.z;
			vPoint.w = a_pBody->m_vCentreOfMass.w;

			// Apply the impulse without the accumulated forces
			TVector4 vForce    = a_pBody->m_vForce;
			TVector4 vTorque   = a_pBody->m_vTorque;
			a_pBody->m_vForce  = TVector4::VEC_ZERO;
			a_pBody->m_vTorque = TVector4::VEC_ZERO;

			a_pBody->ApplyForce( vImpulse, vPoint );
			a_pBody->IntegrateVelocity( 1.0f );

			a_pBody->m_vForce  = vForce;
			a_pBody->m_vTorque = vTorque;

			TVector4 vPosition;
			vPosition.x = vSumPushOut.x + a_pBody->m_oTransform.GetTranslation().x;
			vPosition.y = vSumPushOut.y + a_pBody->m_oTransform.GetTranslation().y;
			vPosition.z = vSumPushOut.z + a_pBody->m_oTransform.GetTranslation().z;
			vPosition.w = a_pBody->m_oTransform.GetTranslation().w;
			a_pBody->SetPosition( vPosition, TFALSE );

			a_pSummary->iNumContacts = iCount;
			a_pSummary->vImpulse.Multiply( vSumImpulse, fScale );

			return TTRUE;
		}
	}

	a_pSummary->iNumContacts = 0;
	a_pSummary->vImpulse     = TVector4::VEC_ZERO;

	return TFALSE;
}

// $Barnyard: FUNCTION 0065b050
void AVehicleManager::AddStaticContact( ARigidBody* a_pBody, ARigidBodyCollision* a_pCollision, TFLOAT a_fDeltaTime )
{
	ARigidBodyMaterial oWorldMaterial( 1.0f, 0.1f, ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, 0, -1 );

	TVector4          vOffset  = TVector4::VEC_ZERO;
	TVector4          vImpulse = TVector4::VEC_ZERO;
	TFLOAT            fImpulse = 0.0f;
	ACollisionObject* pObject  = a_pCollision->m_pCollisionObject;

	if ( pObject == TNULL || ( ~a_pBody->m_oMaterial.m_uiResponseTypes & pObject->GetType() ) == 0 )
	{
		TFLOAT fRestitution = TMath::Sqrt( oWorldMaterial.m_fRestitution * a_pBody->m_oMaterial.m_fRestitution );
		TFLOAT fFriction    = TMath::Sqrt( oWorldMaterial.m_fFriction * a_pBody->m_oMaterial.m_fFriction );

		// Remove the gravity of this step from the velocity while the impulse is calculated
		TVector4 vGravity;
		vGravity.Multiply( TVector4::VEC_POSY, 19.6f );
		vGravity.Multiply( vGravity, a_fDeltaTime );

		TVector4 vVelocity = a_pBody->m_vVelocity;
		a_pBody->m_vVelocity.Set(
		    vVelocity.x - vGravity.x,
		    vVelocity.y - vGravity.y,
		    vVelocity.z - vGravity.z,
		    vVelocity.w
		);

		// Position of the contact relative to the centre of mass at the time of the collision
		TFLOAT fRemaining = 1.0f - a_pCollision->m_fTime;
		vOffset           = a_pCollision->m_vPosition;

		TVector4 vCentre;
		vCentre.Multiply( a_pCollision->m_vOffset, fRemaining );
		vCentre.Substract( a_pBody->m_vCentreOfMass, vCentre );

		vOffset.x = vOffset.x - vCentre.x;
		vOffset.y = vOffset.y - vCentre.y;
		vOffset.z = vOffset.z - vCentre.z;

		TFLOAT fCollisionImpulse;

		if ( pObject != TNULL && ( pObject->GetType() & ACollisionObject::TYPE_MOVABLE ) )
		{
			// Hit a movable object, treat it like a sphere
			TVector4 vObjectVelocity = TVector4::VEC_ZERO;
			TVector4 vObjectCentre   = a_pCollision->m_vPosition;

			TVector4 vPoint;
			vPoint.Add( a_pBody->m_vCentreOfMass, vOffset );
			fCollisionImpulse = a_pBody->CalculateCollisionImpulse( vPoint, a_pCollision->m_vNormal, 100.0f, vObjectVelocity, vObjectCentre );
		}
		else
		{
			TVector4 vPoint;
			vPoint.Add( vOffset, a_pBody->m_vCentreOfMass );
			fCollisionImpulse = a_pBody->CalculateCollisionImpulse( vPoint, a_pCollision->m_vNormal );
		}

		fImpulse                 = fCollisionImpulse * ( fRestitution + 1.0f );
		a_pCollision->m_fImpulse = fImpulse;
		vImpulse.Multiply( a_pCollision->m_vNormal, fImpulse );

		if ( fFriction > 0.0f )
		{
			TVector4 vPoint;
			vPoint.Add( a_pBody->m_vCentreOfMass, vOffset );

			TVector4 vFriction = a_pBody->CalculateFrictionImpulse( vPoint, a_pCollision->m_vNormal, fImpulse, fFriction );
			vImpulse.x += vFriction.x;
			vImpulse.y += vFriction.y;
			vImpulse.z += vFriction.z;
		}

		a_pBody->m_vVelocity = vVelocity;

		// Push the body out of the collision
		TVector4 vPushOut;
		vPushOut.Multiply( a_pCollision->m_vNormal, a_pCollision->m_fDepth );

		TVector4 vPosition;
		vPosition.Add( vPushOut, a_pBody->m_oTransform.GetTranslation() );
		vPosition.w = a_pBody->m_oTransform.GetTranslation().w;
		a_pBody->SetPosition( vPosition, TFALSE );
	}

	ARigidBodyContact* pContact  = AllocateContact();
	pContact->m_fImpulse         = fImpulse;
	pContact->m_vImpulse         = vImpulse;
	pContact->m_vOffset          = vOffset;
	pContact->m_vPosition        = a_pCollision->m_vPosition;
	pContact->m_vNormal          = a_pCollision->m_vNormal;
	pContact->m_fTime            = a_pCollision->m_fTime;
	pContact->m_vPushOut         = TVector4::VEC_ZERO;
	pContact->m_iSurfaceType     = a_pCollision->m_iSurfaceType;
	pContact->m_pOtherMaterial   = TNULL;
	pContact->m_pOtherBody       = TNULL;
	pContact->m_pCollisionObject = a_pCollision->m_pCollisionObject;

	a_pBody->AddContact( pContact );
}

// $Barnyard: FUNCTION 0065dc80
void AVehicleManager::AddBodyContacts( ARigidBody* a_pBody1, ARigidBody* a_pBody2, ARigidBodyCollision* a_pCollision, TFLOAT a_fDeltaTime )
{
	TVector4 vPushOut1 = TVector4::VEC_ZERO;
	TVector4 vPushOut2 = TVector4::VEC_ZERO;

	TFLOAT fDepth    = a_pCollision->m_fDepth + 0.0099999998f;
	TFLOAT fInvMass  = 1.0f / ( a_pBody1->m_fMass + a_pBody2->m_fMass );
	TFLOAT fPushOut1 = fInvMass * a_pBody2->m_fMass;
	TFLOAT fPushOut2 = -( fInvMass * a_pBody1->m_fMass );

	// Separate the bodies relative to their masses and collide their shapes with the world at the new positions
	if ( fPushOut1 != 0.0f )
	{
		TVector4 vPosition = a_pBody1->m_oTransform.GetTranslation();

		TVector4 vPushOut;
		vPushOut.Multiply( a_pCollision->m_vNormal, fDepth );
		vPushOut.Multiply( vPushOut, fPushOut1 );

		TVector4 vNewPosition;
		vNewPosition.Add( vPushOut, vPosition );
		a_pBody1->SetPosition( vNewPosition, TFALSE );
		CollideShape( a_pBody1, a_fDeltaTime );

		vPushOut1.Substract( a_pBody1->m_oTransform.GetTranslation(), vPosition );
		vPushOut1.w = a_pBody1->m_oTransform.GetTranslation().w;
		a_pBody1->SetPosition( vPosition, TFALSE );
	}

	if ( fPushOut2 != 0.0f )
	{
		TVector4 vPosition = a_pBody2->m_oTransform.GetTranslation();

		TVector4 vPushOut;
		vPushOut.Multiply( a_pCollision->m_vNormal, fDepth );
		vPushOut.Multiply( vPushOut, fPushOut2 );

		TVector4 vNewPosition;
		vNewPosition.Add( vPushOut, vPosition );
		a_pBody2->SetPosition( vNewPosition, TFALSE );
		CollideShape( a_pBody2, a_fDeltaTime );

		vPushOut2.Substract( a_pBody2->m_oTransform.GetTranslation(), vPosition );
		vPushOut2.w = a_pBody2->m_oTransform.GetTranslation().w;
		a_pBody2->SetPosition( vPosition, TFALSE );
	}

	TFLOAT fFriction = TMath::Sqrt( a_pBody2->m_oMaterial.m_fFriction * a_pBody1->m_oMaterial.m_fFriction );

	TFLOAT fRestitution = TMath::Sqrt( a_pBody2->m_oMaterial.m_fRestitution * a_pBody1->m_oMaterial.m_fRestitution );
	TFLOAT fImpulse     = a_pBody1->CalculateCollisionImpulse( a_pCollision->m_vPosition, a_pCollision->m_vNormal, a_pBody2 ) * ( fRestitution + 1.0f );

	TVector4 vImpulse;
	vImpulse.Multiply( a_pCollision->m_vNormal, fImpulse );

	if ( fFriction > 0.0f )
	{
		TVector4 vFriction = a_pBody1->CalculateFrictionImpulse( a_pCollision->m_vPosition, a_pCollision->m_vNormal, a_pBody2, fImpulse, fFriction );
		vImpulse.x += vFriction.x;
		vImpulse.y += vFriction.y;
		vImpulse.z += vFriction.z;
	}

	// Contact of the first body
	ARigidBodyContact* pContact1  = AllocateContact();
	pContact1->m_vPosition        = a_pCollision->m_vPosition;
	pContact1->m_vNormal          = a_pCollision->m_vNormal;
	pContact1->m_vImpulse         = vImpulse;
	pContact1->m_vOffset          = TVector4::VEC_ZERO;
	pContact1->m_fImpulse         = fImpulse;
	pContact1->m_pOtherMaterial   = &a_pBody2->m_oMaterial;
	pContact1->m_iSurfaceType     = SURFACE_NONE;
	pContact1->m_pOtherBody       = a_pBody2;
	pContact1->m_pCollisionObject = a_pCollision->m_pCollisionObject;
	pContact1->m_fTime            = a_pCollision->m_fTime;
	pContact1->m_vPushOut         = vPushOut1;
	a_pBody1->AddContact( pContact1 );

	// Contact of the second body
	ARigidBodyContact* pContact2 = AllocateContact();
	pContact2->m_vPosition       = a_pCollision->m_vPosition;
	pContact2->m_vNormal         = a_pCollision->m_vNormal;

	TVector4 vImpulse2;
	vImpulse2.Multiply( vImpulse, -1.0f );
	pContact2->m_vImpulse         = vImpulse2;
	pContact2->m_vOffset          = TVector4::VEC_ZERO;
	pContact2->m_fImpulse         = fImpulse;
	pContact2->m_pOtherMaterial   = &a_pBody1->m_oMaterial;
	pContact2->m_iSurfaceType     = SURFACE_NONE;
	pContact2->m_pOtherBody       = a_pBody1;
	pContact2->m_pCollisionObject = a_pCollision->m_pCollisionObject;
	pContact2->m_fTime            = a_pCollision->m_fTime;
	pContact2->m_vPushOut         = vPushOut2;
	a_pBody2->AddContact( pContact2 );
}

// $Barnyard: FUNCTION 0065b5e0
AVehicleManager::SURFACE AVehicleManager::GetSurfaceType( const ACollisionModelRayHitList::Hit& a_rcHit )
{
	TModelCollisionData* pCollisionData = a_rcHit.pInstance->GetCollisionData( a_rcHit.iModel );

	switch ( pCollisionData->GetCollGroup( pCollisionData->GetGroupForTriangleImp( a_rcHit.uiFaceId ) ).eCollGroup )
	{
		case TCollisionCommon::TOSHICGROUP_GRASS: return SURFACE_GRASS;
		case TCollisionCommon::TOSHICGROUP_TIN: return SURFACE_TIN;
		case TCollisionCommon::TOSHICGROUP_TIMBER: return SURFACE_TIMBER;
		case TCollisionCommon::TOSHICGROUP_STONE: return SURFACE_STONE;
		case TCollisionCommon::TOSHICGROUP_DIRT: return SURFACE_DIRT;
		case TCollisionCommon::TOSHICGROUP_WATER: return SURFACE_WATER;
		case TCollisionCommon::TOSHICGROUP_SAND: return SURFACE_SAND;
		case TCollisionCommon::TOSHICGROUP_MUD: return SURFACE_MUD;
		case TCollisionCommon::TOSHICGROUP_HAY: return SURFACE_HAY;
		default: return SURFACE_NONE;
	}
}

// $Barnyard: FUNCTION 0065aa10
TBOOL AVehicleManager::CollideSweptSphere( ARigidBody* a_pBody, ARigidBodyCollision* a_pCollision, const TVector4& a_rcStart, const TVector4& a_rcEnd, TFLOAT a_fRadius )
{
	TCollisionCommon::TOSHICGROUPFLAG eIgnoreGroups = TCollisionCommon::TOSHICGROUPFLAG( a_pBody->m_oMaterial.m_uiIgnoreGroups );

	TVector4           vNormal           = TVector4::VEC_NEGY;
	ACollisionManager* pCollisionManager = ACollisionManager::GetSingleton();

	// Is the sphere already touching something at the start?
	ACollisionObjectSimple oSphere;
	oSphere.UpdateHash( a_fRadius, a_rcStart.AsVector3() );

	TFLOAT fDistance = a_fRadius;

	if ( ACollisionManager::CollideSphereWithWorld( a_rcStart, &oSphere, fDistance, vNormal, ~a_pBody->m_oMaterial.m_uiCollideTypes, eIgnoreGroups ) == TTRUE )
	{
		a_pCollision->m_vNormal = vNormal;
		a_pCollision->m_fDepth  = fDistance;

		TFLOAT fZero           = 0.0f;
		a_pCollision->m_fDepth = *( ( a_pCollision->m_fDepth > 0.0f ) ? &a_pCollision->m_fDepth : &fZero );
		TFLOAT fPenetration    = a_fRadius - a_pCollision->m_fDepth;

		TVector4 vOffset;
		vOffset.Multiply( a_pCollision->m_vNormal, fPenetration );

		a_pCollision->m_vPosition.Set( a_rcStart.x - vOffset.x, a_rcStart.y - vOffset.y, a_rcStart.z - vOffset.z, a_rcStart.w );
		a_pCollision->m_fTime = 0.0f;
		a_pCollision->m_vOffset.Set( a_rcEnd.x - a_rcStart.x, a_rcEnd.y - a_rcStart.y, a_rcEnd.z - a_rcStart.z, a_rcEnd.w );
		a_pCollision->m_pCollisionObject = TNULL;
		a_pCollision->m_iSurfaceType     = SURFACE_NONE;

		return TTRUE;
	}

	// Sweep the sphere from the start to the end
	TVector4 vDirection( a_rcEnd.x - a_rcStart.x, a_rcEnd.y - a_rcStart.y, a_rcEnd.z - a_rcStart.z, a_rcEnd.w );
	fDistance             = vDirection.NormaliseMag();
	TFLOAT fTotalDistance = fDistance;

	if ( fDistance <= 0.0f )
		return TFALSE;

	TVector4 vHitNormal = TVector4::VEC_NEGY;

	ACollisionObjectSimple oSweptSphere;
	oSweptSphere.UpdateHash( a_fRadius, a_rcStart.AsVector3() );

	TFLOAT fStepHeight  = 0.0f;
	TFLOAT fPenetration = 0.0f;

	// NOTE: the original passes the address of a local TBOOL as the unused argument
	if ( pCollisionManager->CollideSweptSphere( a_rcStart, a_fRadius, &oSweptSphere, vDirection, fDistance, vHitNormal, 0.15f, fStepHeight, fPenetration, 0, ~a_pBody->m_oMaterial.m_uiCollideTypes, TNULL, eIgnoreGroups ) == 0 )
		return TFALSE;

	a_pCollision->m_fDepth = ( fTotalDistance - fDistance ) * TMath::Abs( TVector4::DotProduct3( vDirection, vHitNormal ) );

	TVector4 vRadius;
	vRadius.Multiply( vHitNormal, a_fRadius );

	TVector4 vMovement;
	vMovement.Multiply( vDirection, fDistance );

	TVector4 vCentre;
	vCentre.Add( a_rcStart, vMovement );

	a_pCollision->m_vPosition.Substract( vCentre, vRadius );
	a_pCollision->m_vNormal = vHitNormal;

	if ( fTotalDistance <= 0.001f )
	{
		a_pCollision->m_fTime = 0.0f;
	}
	else
	{
		a_pCollision->m_fTime = fDistance / fTotalDistance;
		TMath::Clip( a_pCollision->m_fTime, 0.0f, 1.0f );
	}

	a_pCollision->m_vOffset          = vDirection;
	a_pCollision->m_iSurfaceType     = SURFACE_NONE;
	a_pCollision->m_pCollisionObject = TNULL;

	return TTRUE;
}

// $Barnyard: FUNCTION 0065aea0
TBOOL AVehicleManager::CollideBoxWithObject( const TOBBox& a_rcBox, ACollisionObject* a_pObject, TCollisionContact* a_pContact )
{
	if ( a_pObject->GetModelInstance() != TNULL )
		return TFALSE;

	TBOOL bResult;

	if ( a_pObject->GetClass() == &TGetClass( ACollisionObjectBox ) )
	{
		bResult = TOBBox::CollideOBB( a_rcBox, TSTATICCAST( ACollisionObjectBox, a_pObject )->GetBox(), *a_pContact );
	}
	else if ( a_pObject->GetHeightScale() > 0.0f )
	{
		// Vertical capsule
		TVector4 vDirection = TVector4::VEC_NEGY;
		TFLOAT   fLength    = a_pObject->GetHeightScale();

		const TVector4& rcPosition = a_pObject->GetObjectPosition();
		TVector4        vPosition( rcPosition.x, rcPosition.y, rcPosition.z, 1.0f );

		TVector4 vOffset;
		vOffset.Multiply( vDirection, fLength );
		vOffset.Multiply( vOffset, 0.5f );

		TVector4 vCentre;
		vCentre.Add( vPosition, vOffset );

		TCapsule oCapsule( vCentre, vDirection, a_pObject->GetObjectRadius(), fLength );
		bResult = TOBBox::CollideCapsule( a_rcBox, oCapsule, *a_pContact );
	}
	else
	{
		TFLOAT          fRadius    = a_pObject->GetObjectRadius();
		const TVector4& rcPosition = a_pObject->GetObjectPosition();
		TVector4        vSphere( rcPosition.x, rcPosition.y, rcPosition.z, fRadius );

		bResult = TOBBox::CollideSphere( a_rcBox, vSphere, *a_pContact );
	}

	if ( bResult == TTRUE )
		a_pContact->vNormal.Multiply( -1.0f );

	return bResult;
}

// Returns the surface type of a collision object hit by the box queries
static AVehicleManager::SURFACE GetObjectSurfaceType( ACollisionObject* a_pObject )
{
	TUINT uiType = a_pObject->GetType();

	if ( uiType == ACollisionObject::TYPE_TRUNK || uiType == ACollisionObject::TYPE_FOLIAGE )
		return AVehicleManager::SURFACE_TIMBER;

	// SURFACE_GRASS for the bushes, SURFACE_NONE otherwise
	return ( uiType == ACollisionObject::TYPE_BUSH ) - 1;
}

// Returns the collision object of a_rcHit's model instance from the list of the hit objects
static ACollisionObject* FindHitObject( const ACollisionObjectBox::ObjectList& a_rcObjects, const ACollisionModelRayHitList::Hit& a_rcHit )
{
	for ( TINT i = 0; i < a_rcObjects.iNumObjects; i++ )
	{
		ACollisionModelInstance* pInstance = a_rcObjects.apObjects[ i ]->GetModelInstance();

		if ( pInstance != TNULL && pInstance == a_rcHit.pInstance )
			return a_rcObjects.apObjects[ i ];
	}

	return TNULL;
}

// Triangles of the swept box query (retail global at 0x007b5f88)
static TCollisionTriangle s_aTriangles[ AVehicleManager::MAX_NUM_TRIANGLES ];

// $Barnyard: FUNCTION 0065c200
TBOOL AVehicleManager::CollideBoxStatic( ARigidBody* a_pBody, ARigidBodyCollision* a_pCollision, const TOBBox& a_rcPrevBox, TOBBox& a_rBox )
{
	ACollisionModelRayHitList       oHits;
	ACollisionObjectBox::ObjectList oObjects;
	oObjects.iNumObjects = 0;

	ACollisionObjectBox oBoxObject;
	oBoxObject.SetExtents( a_rBox.m_vExtents );
	oBoxObject.SetTransform( a_rBox.m_oTransform );

	TVector4 vNormal = TVector4::VEC_NEGY;
	TFLOAT   fRadius = a_rBox.m_fRadius;

	TCollisionCommon::TOSHICGROUPFLAG eIgnoreGroups = TCollisionCommon::TOSHICGROUPFLAG( a_pBody->m_oMaterial.m_uiIgnoreGroups );

	TBOOL bObjects = ACollisionManager::GetSingleton()->CollideShapeObjects( &a_rBox, &oBoxObject, &oHits, &oObjects, &fRadius, &vNormal, ~a_pBody->m_oMaterial.m_uiCollideTypes ) != TNULL;

	if ( ( bObjects | ACollisionManager::CollideBoxWithTerrain( a_rBox, &oHits, eIgnoreGroups ) ) != TTRUE )
		return TFALSE;

	TFLOAT fBestDepth = -3.4028235e38f;
	TOBBox oBox( a_rBox );
	TBOOL  bResult = TFALSE;

	for ( TINT iIteration = 0;; )
	{
		TBOOL bHit     = TFALSE;
		TINT  iBestHit = -1;
		TINT  iNumHits = oHits.iNumHits;

		// Triangles
		for ( TINT i = 0; i < iNumHits; i++ )
		{
			TVector3 avVertices[ 3 ];
			oHits.aHits[ i ].GetTriangle( avVertices );

			TCollisionTriangle oTriangle(
			    TVector4( avVertices[ 0 ].x, avVertices[ 0 ].y, avVertices[ 0 ].z, 1.0f ),
			    TVector4( avVertices[ 1 ].x, avVertices[ 1 ].y, avVertices[ 1 ].z, 1.0f ),
			    TVector4( avVertices[ 2 ].x, avVertices[ 2 ].y, avVertices[ 2 ].z, 1.0f ),
			    oHits.aHits[ i ].GetNormal()
			);

			TVector4 vToBox( oBox.GetCentre().x - oTriangle.aVertices[ 0 ].x, oBox.GetCentre().y - oTriangle.aVertices[ 0 ].y, oBox.GetCentre().z - oTriangle.aVertices[ 0 ].z, oBox.GetCentre().w );

			if ( TVector4::DotProduct3( vToBox, oTriangle.vNormal ) >= 0.0f )
			{
				TVector4 vPoint;
				TVector4 vHitNormal;
				TFLOAT   fTime  = TOBBox::PenetrationVsTriangle( oBox, oTriangle, vPoint, vHitNormal );
				TFLOAT   fDepth = sqrtf( oBox.m_vExtents.MagnitudeSq() ) * ( 1.0f - fTime ) + 0.0099999998f;

				if ( fTime <= 1.0f && fDepth > fBestDepth )
				{
					a_pCollision->m_fDepth    = fDepth;
					a_pCollision->m_vNormal   = vHitNormal;
					a_pCollision->m_vPosition = vPoint;
					a_pCollision->m_fTime     = fTime;
					a_pCollision->m_vOffset.Set( a_rBox.GetCentre().x - a_rcPrevBox.GetCentre().x, a_rBox.GetCentre().y - a_rcPrevBox.GetCentre().y, a_rBox.GetCentre().z - a_rcPrevBox.GetCentre().z, a_rBox.GetCentre().w );

					iBestHit   = i;
					bResult    = TTRUE;
					bHit       = TTRUE;
					fBestDepth = fDepth;
				}
			}
		}

		if ( iBestHit != -1 )
		{
			a_pCollision->m_iSurfaceType     = GetSurfaceType( oHits.aHits[ iBestHit ] );
			a_pCollision->m_pCollisionObject = FindHitObject( oObjects, oHits.aHits[ iBestHit ] );
		}

		// Objects
		for ( TINT i = 0; i < oObjects.iNumObjects; i++ )
		{
			ACollisionObject* pObject = oObjects.apObjects[ i ];

			TCollisionContact oContact;

			if ( CollideBoxWithObject( oBox, pObject, &oContact ) && oContact.fDepth > fBestDepth )
			{
				a_pCollision->m_vNormal   = oContact.vNormal;
				a_pCollision->m_fDepth    = oContact.fDepth;
				a_pCollision->m_vPosition = oContact.vPosition;
				a_pCollision->m_fTime     = 1.0f - oContact.fDepth / sqrtf( oBox.m_vExtents.MagnitudeSq() );
				TMath::Clip( a_pCollision->m_fTime, 0.0f, 1.0f );
				a_pCollision->m_vOffset.Set( a_rBox.GetCentre().x - a_rcPrevBox.GetCentre().x, a_rBox.GetCentre().y - a_rcPrevBox.GetCentre().y, a_rBox.GetCentre().z - a_rcPrevBox.GetCentre().z, a_rBox.GetCentre().w );
				a_pCollision->m_iSurfaceType     = GetObjectSurfaceType( pObject );
				a_pCollision->m_pCollisionObject = pObject;

				fBestDepth = oContact.fDepth;
				bResult    = TTRUE;
				bHit       = TTRUE;
			}
		}

		if ( bHit != TTRUE )
			break;

		// Push the box out and try again
		TVector4 vPushOut;
		vPushOut.Multiply( a_pCollision->m_vNormal, a_pCollision->m_fDepth );

		oBox.m_oTransform.m_f41 = oBox.m_oTransform.m_f41 + vPushOut.x;
		oBox.m_oTransform.m_f42 = oBox.m_oTransform.m_f42 + vPushOut.y;
		oBox.m_oTransform.m_f43 = oBox.m_oTransform.m_f43 + vPushOut.z;

		if ( ++iIteration >= 10 )
			break;
	}

	return bResult;
}

// $Barnyard: FUNCTION 0065c920
TBOOL AVehicleManager::CollideBoxSwept( ARigidBody* a_pBody, ARigidBodyCollision* a_pCollision, const TOBBox& a_rcPrevBox, TOBBox& a_rBox )
{
	ACollisionModelRayHitList       oHits;
	ACollisionObjectBox::ObjectList oObjects;
	oObjects.iNumObjects = 0;

	ACollisionObjectBox oBoxObject;
	oBoxObject.SetExtents( a_rBox.m_vExtents );
	oBoxObject.SetTransform( a_rBox.m_oTransform );

	TVector4 vNormal = TVector4::VEC_NEGY;
	TFLOAT   fRadius = a_rBox.m_fRadius;

	TCollisionCommon::TOSHICGROUPFLAG eIgnoreGroups = TCollisionCommon::TOSHICGROUPFLAG( a_pBody->m_oMaterial.m_uiIgnoreGroups );

	TBOOL bObjects = ACollisionManager::GetSingleton()->CollideShapeObjects( &a_rBox, &oBoxObject, &oHits, &oObjects, &fRadius, &vNormal, ~a_pBody->m_oMaterial.m_uiCollideTypes ) != TNULL;

	if ( !ATerrainInterface::GetSingleton()->CollideSweptBox( a_rcPrevBox, a_rBox, &oHits, -1, eIgnoreGroups ) && !bObjects )
		return TFALSE;

	TOBBox oStartBox( a_rcPrevBox );
	TOBBox oEndBox( a_rBox );

	// Cache the hit triangles
	TINT iNumTriangles = 0;

	for ( TINT i = 0; i < oHits.iNumHits; i++ )
	{
		TVector3 avVertices[ 3 ];
		oHits.aHits[ i ].GetTriangle( avVertices );

		TCollisionTriangle& rTriangle = s_aTriangles[ iNumTriangles ];
		rTriangle.aVertices[ 0 ].Set( avVertices[ 0 ].x, avVertices[ 0 ].y, avVertices[ 0 ].z, 1.0f );
		rTriangle.aVertices[ 1 ].Set( avVertices[ 1 ].x, avVertices[ 1 ].y, avVertices[ 1 ].z, 1.0f );
		rTriangle.aVertices[ 2 ].Set( avVertices[ 2 ].x, avVertices[ 2 ].y, avVertices[ 2 ].z, 1.0f );
		rTriangle.vNormal = oHits.aHits[ i ].GetNormal();

		if ( ++iNumTriangles == MAX_NUM_TRIANGLES )
			break;
	}

	// The start box has the orientation of the end box
	oStartBox.m_oTransform                  = oEndBox.m_oTransform;
	oStartBox.m_oTransform.GetTranslation() = a_rcPrevBox.GetCentre();

	TVector4 vHitNormal;

	// Make sure the start position doesn't collide
	for ( TINT iIteration = 0; iIteration < 10; iIteration++ )
	{
		TBOOL  bHit       = TFALSE;
		TFLOAT fBestDepth = 3.4028235e38f;

		for ( TINT i = 0; i < oObjects.iNumObjects; i++ )
		{
			TCollisionContact oContact;
			TBOOL             bCollided = CollideBoxWithObject( oStartBox, oObjects.apObjects[ i ], &oContact );
			TFLOAT            fDepth    = oContact.fDepth + 0.0099999998f;

			if ( bCollided == TTRUE && fBestDepth > fDepth )
			{
				fBestDepth = fDepth;
				vHitNormal = oContact.vNormal;
				bHit       = TTRUE;
			}
		}

		for ( TINT i = 0; i < iNumTriangles; i++ )
		{
			TCollisionTriangle oTriangle( s_aTriangles[ i ].aVertices[ 0 ], s_aTriangles[ i ].aVertices[ 1 ], s_aTriangles[ i ].aVertices[ 2 ], s_aTriangles[ i ].vNormal );

			TVector4 vToBox(
			    oStartBox.GetCentre().x - oTriangle.aVertices[ 0 ].x,
			    oStartBox.GetCentre().y - oTriangle.aVertices[ 0 ].y,
			    oStartBox.GetCentre().z - oTriangle.aVertices[ 0 ].z,
			    oStartBox.GetCentre().w
			);

			if ( TVector4::DotProduct3( vToBox, oTriangle.vNormal ) >= 0.0f )
			{
				TVector4 vPoint;
				TVector4 vNormal2;
				TFLOAT   fTime = TOBBox::PenetrationVsTriangle( oStartBox, oTriangle, vPoint, vNormal2 );

				if ( fTime <= 1.0f )
				{
					TFLOAT fDepth = ( 1.0f - fTime ) * oStartBox.m_fRadius + 0.0099999998f;

					if ( fBestDepth > fDepth )
					{
						fBestDepth = fDepth;
						vHitNormal = vNormal2;
						bHit       = TTRUE;
					}
				}
			}
		}

		if ( bHit != TTRUE )
			break;

		TVector4 vPushOut;
		vPushOut.Multiply( vHitNormal, fBestDepth );

		oStartBox.m_oTransform.m_f41 = oStartBox.m_oTransform.m_f41 + vPushOut.x;
		oStartBox.m_oTransform.m_f42 = oStartBox.m_oTransform.m_f42 + vPushOut.y;
		oStartBox.m_oTransform.m_f43 = oStartBox.m_oTransform.m_f43 + vPushOut.z;
	}

	TVector4 vMovement(
	    oEndBox.GetCentre().x - oStartBox.GetCentre().x,
	    oEndBox.GetCentre().y - oStartBox.GetCentre().y,
	    oEndBox.GetCentre().z - oStartBox.GetCentre().z,
	    oEndBox.GetCentre().w
	);

	TFLOAT            fBestTime    = 0.99000001f;
	TINT              iSurfaceType = SURFACE_NONE;
	TVector4          vPushOut     = TVector4::VEC_ZERO;
	ACollisionObject* pHitObject   = TNULL;
	TVector4          vHitPoint;

	if ( ms_bSlideOnObjects && vMovement.MagnitudeSq() > 0.0f )
	{
		// Find the first hit of the movement
		TBOOL bHit = TFALSE;

		for ( TINT i = 0; i < oObjects.iNumObjects; i++ )
		{
			ACollisionObject* pObject = oObjects.apObjects[ i ];

			TCollisionContact oContact;

			if ( CollideBoxWithObject( oEndBox, pObject, &oContact ) == TTRUE )
			{
				TFLOAT fTime = 1.0f - oContact.fDepth / oEndBox.m_fRadius;

				if ( fBestTime > fTime )
				{
					pHitObject   = pObject;
					iSurfaceType = GetObjectSurfaceType( pObject );
					fBestTime    = fTime;
					vHitPoint    = oContact.vPosition;
					vHitNormal   = oContact.vNormal;
					bHit         = TTRUE;
				}
			}
		}

		TINT iBestTriangle = -1;

		for ( TINT i = 0; i < iNumTriangles; i++ )
		{
			TCollisionTriangle oTriangle( s_aTriangles[ i ].aVertices[ 0 ], s_aTriangles[ i ].aVertices[ 1 ], s_aTriangles[ i ].aVertices[ 2 ], s_aTriangles[ i ].vNormal );

			if ( TVector4::DotProduct3( oTriangle.vNormal, vMovement ) <= 0.0f )
			{
				// NOTE: the point and the normal are written even if the hit isn't the closest one
				TFLOAT fTime = TOBBox::SweepVsTriangle( oStartBox, oEndBox, oTriangle, vHitPoint, vHitNormal );

				if ( fBestTime > fTime )
				{
					fBestTime     = fTime;
					iBestTriangle = i;
					bHit          = TTRUE;
				}
			}
		}

		if ( iBestTriangle != -1 )
		{
			pHitObject   = FindHitObject( oObjects, oHits.aHits[ iBestTriangle ] );
			iSurfaceType = GetSurfaceType( oHits.aHits[ iBestTriangle ] );

			if ( ms_bUseTriangleNormal )
				vHitNormal = s_aTriangles[ iBestTriangle ].vNormal;
		}

		if ( bHit == TTRUE )
		{
			// Push the box back to the time of the hit along the normal
			TVector4 vBack;
			vBack.Multiply( vMovement, fBestTime - 1.0f );

			vPushOut.Multiply( vHitNormal, TVector4::DotProduct3( vBack, vHitNormal ) + 0.0099999998f + 0.0099999998f );
		}
	}

	TVector4 vEndCentre(
	    oEndBox.GetCentre().x + vPushOut.x,
	    oEndBox.GetCentre().y + vPushOut.y,
	    oEndBox.GetCentre().z + vPushOut.z,
	    oEndBox.GetCentre().w
	);

	oEndBox.m_oTransform.m_f41 = vEndCentre.x;
	oEndBox.m_oTransform.m_f42 = vEndCentre.y;
	oEndBox.m_oTransform.m_f43 = vEndCentre.z;

	// Push the end position out of the collisions
	for ( TINT iIteration = 0; iIteration < 20; iIteration++ )
	{
		TBOOL  bHit       = TFALSE;
		TFLOAT fBestDepth = 3.4028235e38f;

		for ( TINT i = 0; i < oObjects.iNumObjects; i++ )
		{
			ACollisionObject* pObject = oObjects.apObjects[ i ];

			TCollisionContact oContact;
			TBOOL             bCollided = CollideBoxWithObject( oEndBox, pObject, &oContact );
			TFLOAT            fDepth    = oContact.fDepth + 0.0099999998f;

			if ( bCollided == TTRUE && fBestDepth > fDepth )
			{
				pHitObject   = pObject;
				iSurfaceType = GetObjectSurfaceType( pObject );
				fBestDepth   = fDepth;
				vHitNormal   = oContact.vNormal;
				vHitPoint    = oContact.vPosition;
				bHit         = TTRUE;
			}
		}

		TINT iBestTriangle = -1;

		for ( TINT i = 0; i < iNumTriangles; i++ )
		{
			TCollisionTriangle oTriangle( s_aTriangles[ i ].aVertices[ 0 ], s_aTriangles[ i ].aVertices[ 1 ], s_aTriangles[ i ].aVertices[ 2 ], s_aTriangles[ i ].vNormal );

			// NOTE: the original tests against the end position before the push outs of this loop
			TVector4 vToBox( vEndCentre.x - oTriangle.aVertices[ 0 ].x, vEndCentre.y - oTriangle.aVertices[ 0 ].y, vEndCentre.z - oTriangle.aVertices[ 0 ].z, vEndCentre.w );

			if ( TVector4::DotProduct3( vToBox, oTriangle.vNormal ) >= 0.0f )
			{
				// NOTE: the point is written even if the hit isn't the deepest one
				TVector4 vNormal2;
				TFLOAT   fTime = TOBBox::PenetrationVsTriangle( oEndBox, oTriangle, vHitPoint, vNormal2 );

				if ( fTime <= 1.0f )
				{
					TFLOAT fDepth = ( 1.0f - fTime ) * oEndBox.m_fRadius + 0.0099999998f;

					if ( fBestDepth > fDepth )
					{
						fBestDepth    = fDepth;
						vHitNormal    = oTriangle.vNormal;
						iBestTriangle = i;
						bHit          = TTRUE;
					}
				}
			}
		}

		if ( iBestTriangle != -1 )
		{
			pHitObject   = FindHitObject( oObjects, oHits.aHits[ iBestTriangle ] );
			iSurfaceType = GetSurfaceType( oHits.aHits[ iBestTriangle ] );
		}

		if ( bHit != TTRUE )
			break;

		TVector4 vStep;
		vStep.Multiply( vHitNormal, fBestDepth );
		vPushOut.x += vStep.x;
		vPushOut.y += vStep.y;
		vPushOut.z += vStep.z;

		vStep.Multiply( vHitNormal, fBestDepth );
		oEndBox.m_oTransform.m_f41 = oEndBox.m_oTransform.m_f41 + vStep.x;
		oEndBox.m_oTransform.m_f42 = oEndBox.m_oTransform.m_f42 + vStep.y;
		oEndBox.m_oTransform.m_f43 = oEndBox.m_oTransform.m_f43 + vStep.z;
	}

	if ( vPushOut.MagnitudeSq() > 0.0f )
	{
		a_pCollision->m_vNormal = vPushOut;
		a_pCollision->m_vNormal.Normalise();
		a_pCollision->m_fDepth    = sqrtf( vPushOut.MagnitudeSq() );
		a_pCollision->m_vPosition = vHitPoint;
		a_pCollision->m_fTime     = fBestTime;
		a_pCollision->m_vOffset.Set( a_rBox.GetCentre().x - a_rcPrevBox.GetCentre().x, a_rBox.GetCentre().y - a_rcPrevBox.GetCentre().y, a_rBox.GetCentre().z - a_rcPrevBox.GetCentre().z, a_rBox.GetCentre().w );
		a_pCollision->m_iSurfaceType     = iSurfaceType;
		a_pCollision->m_pCollisionObject = pHitObject;

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0065e380
TBOOL AVehicleManager::CollideBodies( ARigidBody* a_pBody1, ARigidBody* a_pBody2, TFLOAT a_fDeltaTime )
{
	TMatrix44 oTransform1;
	TMatrix44 oTemp = a_pBody1->m_oTransform;
	oTransform1.Multiply( oTemp, a_pBody1->m_oShape.m_oBox.m_oTransform );

	TVector4 vExtents1 = a_pBody1->m_oShape.m_oBox.m_vExtents;
	TOBBox   oBox1( oTransform1, vExtents1 );

	TMatrix44 oTransform2;
	oTemp = a_pBody2->m_oTransform;
	oTransform2.Multiply( oTemp, a_pBody2->m_oShape.m_oBox.m_oTransform );

	TVector4 vExtents2 = a_pBody2->m_oShape.m_oBox.m_vExtents;
	TOBBox   oBox2( oTransform2, vExtents2 );

	TCollisionContact oContact;

	if ( TOBBox::CollideOBB( oBox1, oBox2, oContact ) )
	{
		ARigidBodyCollision oCollision;
		oCollision.m_fDepth = oContact.fDepth;
		oCollision.m_vNormal.Multiply( oContact.vNormal, -1.0f );
		oCollision.m_vPosition = oContact.vPosition;
		oCollision.m_fTime     = 1.0f - oContact.fDepth / oBox1.m_fRadius;
		TMath::Clip( oCollision.m_fTime, 0.0f, 1.0f );
		oCollision.m_vOffset = TVector4::VEC_POSY;
		oCollision.m_vNormal.Normalise();

		AddBodyContacts( a_pBody1, a_pBody2, &oCollision, a_fDeltaTime );
	}

	return a_pBody1->m_iNumContacts > 0;
}

// $Barnyard: FUNCTION 0065bd60
TBOOL AVehicleManager::CollideRay( const TVector4& a_rcStart, const TVector4& a_rcEnd, ARigidBodyCollision* a_pOutCollision, TUINT a_eIgnoreTypes, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TVector4 vDirection;
	vDirection.Substract( a_rcEnd, a_rcStart );
	vDirection.w = a_rcEnd.w;

	TFLOAT fDistance = vDirection.NormaliseMag();
	TFLOAT fLength   = fDistance;

	// Terrain
	ACollisionModelRayHitList oHits;
	TBOOL                     bTerrainHit = ATerrainInterface::GetSingleton()->CollideRay( a_rcStart, vDirection, fDistance, &oHits, a_eIgnoreGroups, TTRUE );

	if ( bTerrainHit )
	{
		TFLOAT   fClosest      = 3.4028235e38f;
		TVector4 vNormal       = TVector4::VEC_POSX;
		TINT     iSurfaceType  = SURFACE_NONE;
		TINT     iClosestIndex = -1;
		TINT     iNumHits      = oHits.iNumHits;

		for ( TINT i = 0; i < iNumHits; i++ )
		{
			TFLOAT fHitDistance = oHits.aHits[ i ].fDistance;

			if ( fHitDistance < fClosest )
			{
				TVector4 vHitNormal = oHits.aHits[ i ].GetNormal();

				if ( TVector4::DotProduct3( vHitNormal, vDirection ) < 0.0f )
				{
					fClosest      = fHitDistance;
					vNormal       = vHitNormal;
					iClosestIndex = i;
					iSurfaceType  = GetSurfaceType( oHits.aHits[ i ] );
				}
			}
		}

		if ( iClosestIndex == -1 )
			bTerrainHit = TFALSE;

		TVector4 vOffset;
		vOffset.Multiply( vDirection, fClosest );

		a_pOutCollision->m_vPosition.Set( vOffset.x + a_rcStart.x, vOffset.y + a_rcStart.y, vOffset.z + a_rcStart.z, a_rcStart.w );
		a_pOutCollision->m_fDepth           = fLength - fClosest;
		a_pOutCollision->m_vNormal          = vNormal;
		a_pOutCollision->m_iSurfaceType     = iSurfaceType;
		a_pOutCollision->m_pCollisionObject = TNULL;
	}

	// Collision objects
	ARayData      oRay( a_rcStart, vDirection, fLength );
	ACollisionHit oHit;
	oHit.vNormal = TVector4::VEC_ZERO;

	ACollisionManager::ms_pRayHits->Clear();
	TBOOL bObjectHit = ACollisionManager::GetSingleton()->CollideRayObjects( TNULL, oRay, ACollisionManager::ms_pRayHits, oHit, TCollisionCommon::TOSHICGROUPFLAG( 0 ), a_eIgnoreTypes );

	if ( TVector4::DotProduct3( vDirection, oHit.vNormal ) < 0.0f )
	{
		if ( bObjectHit )
		{
			TFLOAT fTerrainDistance = 3.4028235e38f;

			if ( bTerrainHit )
			{
				TVector4 vToHit;
				vToHit.Substract( a_rcStart, a_pOutCollision->m_vPosition );
				fTerrainDistance = sqrtf( vToHit.MagnitudeSq() );
			}

			if ( fTerrainDistance > oHit.fDistance )
			{
				TVector4 vOffset;
				vOffset.Multiply( vDirection, oHit.fDistance );

				a_pOutCollision->m_vPosition.Set( vOffset.x + a_rcStart.x, vOffset.y + a_rcStart.y, vOffset.z + a_rcStart.z, a_rcStart.w );
				a_pOutCollision->m_fDepth           = fLength - oHit.fDistance;
				a_pOutCollision->m_vNormal          = oHit.vNormal;
				a_pOutCollision->m_pCollisionObject = oHit.pObject;

				TUINT uiType = oHit.pObject->GetType();

				if ( uiType == ACollisionObject::TYPE_TRUNK || uiType == ACollisionObject::TYPE_FOLIAGE )
					a_pOutCollision->m_iSurfaceType = SURFACE_TIMBER;
				else
					a_pOutCollision->m_iSurfaceType = ( uiType == ACollisionObject::TYPE_BUSH ) - 1; // SURFACE_GRASS or SURFACE_NONE
			}
		}
	}
	else
	{
		bObjectHit = TFALSE;
	}

	return bTerrainHit | bObjectHit;
}
