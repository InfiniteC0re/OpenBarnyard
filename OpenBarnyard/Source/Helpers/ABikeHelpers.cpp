#include "pch.h"
#include "ABikeHelpers.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/Steering/AFallingCowSteer.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionObjectSimple.h"
#include "Physics/AVehicleManager.h"
#include "Physics/APhysicsBike.h"
#include "Physics/ACollisionObject.h"
#include "Units/AVisibleUnit.h"

#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( bike1 );
TPSTRING8_DECLARE( bike2 );
TPSTRING8_DECLARE( bike3 );
TPSTRING8_DECLARE( bike4 );
TPSTRING8_DECLARE( bike5 );
TPSTRING8_DECLARE( body_loc );
TPSTRING8_DECLARE( headlight_loc );
TPSTRING8_DECLARE( c_head );
TPSTRING8_DECLARE( bike_origin );
TPSTRING8_DECLARE( frontwheel );
TPSTRING8_DECLARE( rearwheel );

namespace ABikeHelpers
{

// $Barnyard: FUNCTION 00525960
TBOOL SnapToGround( TVector4& a_rPosition )
{
	ARigidBodyCollision oCollision;

	TVector4 vecStart;
	vecStart.x = TVector4::VEC_NEGY.x + a_rPosition.x;
	vecStart.y = TVector4::VEC_NEGY.y + a_rPosition.y;
	vecStart.z = TVector4::VEC_NEGY.z + a_rPosition.z;
	vecStart.w = a_rPosition.w;

	TVector4 vecRay;
	vecRay.Multiply( TVector4::VEC_POSY, 40.0f );

	TVector4 vecEnd;
	vecEnd.x = vecRay.x + vecStart.x;
	vecEnd.y = vecRay.y + vecStart.y;
	vecEnd.z = vecRay.z + vecStart.z;
	vecEnd.w = vecStart.w;

	TBOOL bHit = AVehicleManager::CollideRay( vecStart, vecEnd, &oCollision, ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_FOLIAGE | ACollisionObject::TYPE_UNIT, 0xb0000 );

	if ( bHit )
		a_rPosition.y = oCollision.m_vPosition.y;
	else
		a_rPosition.y = 0.0f;

	return bHit;
}

// $Barnyard: FUNCTION 00525a90
TBOOL SnapToGround( TVector4& a_rPosition, TVector4& a_rOutNormal, TFLOAT a_fHeight, TFLOAT a_fLength )
{
	ARigidBodyCollision oCollision;

	TVector4 vecStart = a_rPosition;
	vecStart.y        = vecStart.y - a_fHeight;

	TVector4 vecRay;
	vecRay.Multiply( TVector4::VEC_POSY, a_fLength );

	TVector4 vecEnd;
	vecEnd.x = vecRay.x + vecStart.x;
	vecEnd.y = vecRay.y + vecStart.y;
	vecEnd.z = vecRay.z + vecStart.z;
	vecEnd.w = vecStart.w;

	TBOOL bHit = AVehicleManager::CollideRay( vecStart, vecEnd, &oCollision, ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_FOLIAGE | ACollisionObject::TYPE_UNIT, 0x90000 );

	if ( bHit )
	{
		a_rOutNormal  = oCollision.m_vNormal;
		a_rPosition.y = oCollision.m_vPosition.y;
	}
	else
	{
		a_rOutNormal  = TVector4::VEC_NEGY;
		a_rPosition.y = 0.0f;
	}

	return bHit;
}

// $Barnyard: FUNCTION 00525be0
TBOOL AlignToGround( TVector4& a_rPosition, TVector4& a_rDirection, TFLOAT a_fHeight, TFLOAT a_fLength )
{
	TVector4 vecNormal = TVector4::VEC_NEGY;
	TBOOL    bHit      = SnapToGround( a_rPosition, vecNormal, a_fHeight, a_fLength );

	TVector4 vecSide;
	vecSide.CrossProduct( vecNormal, a_rDirection );
	a_rDirection.CrossProduct( vecSide, vecNormal );
	a_rDirection.Normalise();

	return bHit;
}

// $Barnyard: FUNCTION 00525c60
TBOOL SnapToGroundAround( TVector4& a_rPosition, TFLOAT a_fRadius, const TVector4& a_rcDirection, SNAPFLAGS a_eFlags )
{
	TVector4 vecForward = a_rcDirection;
	vecForward.y        = 0.0f;
	vecForward.Normalise();

	TVector4 vecSide;
	vecSide.CrossProduct( TVector4::VEC_NEGY, vecForward );

	TFLOAT fHighest = a_rPosition.y;
	TBOOL  bResult  = TFALSE;

	if ( a_eFlags & SNAPFLAGS_CENTRE )
	{
		TVector4 vecPoint = a_rPosition;
		bResult           = SnapToGround( vecPoint );
		fHighest          = ( vecPoint.y >= fHighest ) ? fHighest : vecPoint.y;
	}

	if ( a_eFlags & SNAPFLAGS_LEFT )
	{
		TVector4 vecOffset;
		vecOffset.Multiply( vecSide, a_fRadius );

		TVector4 vecPoint( vecOffset.x + a_rPosition.x, vecOffset.y + a_rPosition.y, vecOffset.z + a_rPosition.z, a_rPosition.w );
		SnapToGround( vecPoint );
		fHighest = ( vecPoint.y >= fHighest ) ? fHighest : vecPoint.y;
	}

	if ( a_eFlags & SNAPFLAGS_RIGHT )
	{
		TVector4 vecOffset;
		vecOffset.Multiply( vecSide, a_fRadius );

		TVector4 vecPoint( a_rPosition.x - vecOffset.x, a_rPosition.y - vecOffset.y, a_rPosition.z - vecOffset.z, a_rPosition.w );
		SnapToGround( vecPoint );
		fHighest = ( vecPoint.y >= fHighest ) ? fHighest : vecPoint.y;
	}

	if ( a_eFlags & SNAPFLAGS_FORWARD )
	{
		TVector4 vecOffset;
		vecOffset.Multiply( vecForward, a_fRadius );

		TVector4 vecPoint( vecOffset.x + a_rPosition.x, vecOffset.y + a_rPosition.y, vecOffset.z + a_rPosition.z, a_rPosition.w );
		SnapToGround( vecPoint );
		fHighest = ( vecPoint.y >= fHighest ) ? fHighest : vecPoint.y;
	}

	if ( a_eFlags & SNAPFLAGS_BACKWARD )
	{
		TVector4 vecOffset;
		vecOffset.Multiply( vecForward, a_fRadius );

		TVector4 vecPoint( a_rPosition.x - vecOffset.x, a_rPosition.y - vecOffset.y, a_rPosition.z - vecOffset.z, a_rPosition.w );
		SnapToGround( vecPoint );
		fHighest = ( vecPoint.y >= fHighest ) ? fHighest : vecPoint.y;
	}

	a_rPosition.y = fHighest;
	return bResult;
}

// $Barnyard: FUNCTION 00525fc0
void PushOutOfCollision( TVector4& a_rPosition, TFLOAT a_fRadius )
{
	TVector4 vecNormal = TVector4::VEC_NEGY;

	ACollisionObjectSimple oSphere;
	oSphere.UpdateHash( a_fRadius, a_rPosition.AsVector3() );

	TFLOAT fDistance = a_fRadius;

	// Test a bit higher than the position
	TVector4 vecUp;
	vecUp.Multiply( TVector4::VEC_NEGY, a_fRadius );
	TVector4 vecOffset;
	vecOffset.Multiply( vecUp, 1.5f );

	TVector4 vecCentre( vecOffset.x + a_rPosition.x, vecOffset.y + a_rPosition.y, vecOffset.z + a_rPosition.z, a_rPosition.w );

	if ( ACollisionManager::CollideSphereWithWorld( vecCentre, &oSphere, fDistance, vecNormal, ~TUINT( ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN ), 0x70000 ) == TTRUE )
	{
		TFLOAT fPenetration = TMath::Max( a_fRadius - fDistance, 0.0f );

		TVector4 vecPush;
		vecPush.Multiply( vecNormal, fPenetration );

		// NOTE: only moved on the XZ plane
		a_rPosition.x = vecPush.x + vecCentre.x;
		a_rPosition.z = vecPush.z + vecCentre.z;
		a_rPosition.w = vecCentre.w;
	}
}

// $Barnyard: FUNCTION 005261e0
void SetupFallingCow( AUnit* a_pUnit, const TVector4& a_rcVelocity, const TVector4& a_rcDirection, const TVector4& a_rcPrevPosition )
{
	AFallingCowSteer* pSteer = TSTATICCAST( AFallingCowSteer, a_pUnit->GetSteer() );
	pSteer->SetVelocity( a_rcVelocity );
	pSteer->SetDirection( a_rcDirection );

	TVector4 vecPosition = *pSteer->GetPosition();
	PushOutOfCollision( vecPosition, 0.35f );
	pSteer->SetPosition( vecPosition );

	// The body moves from a_rcPrevPosition to where it is now
	TVector4 vecBodyPosition = pSteer->GetBody().GetPosition();
	pSteer->GetBody().SetPosition( a_rcPrevPosition, TTRUE );
	pSteer->GetBody().SetPosition( vecBodyPosition, TFALSE );

	TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject()->SetSkeletonUpdating( TTRUE, TTRUE );
}

// $Barnyard: FUNCTION 005262b0
void AttachBikeModel( AUnit* a_pUnit, TINT a_iBikeType )
{
	AAnimatableObject*        pRiderObject = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();
	AAnimatableObjectManager* pManager     = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pType;

	switch ( a_iBikeType )
	{
		case 0: pType = pManager->FindType( TPS8( bike1 ) ); break;
		case 1: pType = pManager->FindType( TPS8( bike2 ) ); break;
		case 2: pType = pManager->FindType( TPS8( bike3 ) ); break;
		case 3: pType = pManager->FindType( TPS8( bike4 ) ); break;
		default: pType = pManager->FindType( TPS8( bike5 ) ); break;
	}

	AAnimatableObject* pBikeObject = pManager->CreateAnimatableObject( pType, TNULL, 0 );
	pManager->AttachObject( pBikeObject, pRiderObject, TPS8( body_loc ), TPS8( body_loc ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
	pBikeObject->UpdateAttachmentTransform();
	pBikeObject->SetSkeletonUpdating( TTRUE, TTRUE );
}

// $Barnyard: FUNCTION 00526360
void RemoveGlowObject( AGlowViewport::GlowObject* a_pGlowObject )
{
	AGlowViewport::GetSingleton()->RemoveGlowObject( a_pGlowObject );
}

// $Barnyard: FUNCTION 00526370
TBOOL CanReachBike( AUnit* a_pBike, AUnit* a_pUnit )
{
	if ( !a_pBike || !a_pUnit )
		return TTRUE;

	if ( a_pBike->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
	{
		// Both wheels have to stand on the ground
		APhysicsBike* pPhysics = TSTATICCAST( ABikeSteer, a_pBike->GetSteer() )->GetPhysics();

		TVector4 vecFrontWheel = pPhysics->m_aWheels[ 0 ].m_vPosition;
		TVector4 vecRearWheel  = pPhysics->m_aWheels[ 1 ].m_vPosition;

		if ( !SnapToGround( vecFrontWheel ) || !SnapToGround( vecRearWheel ) )
			return TFALSE;
	}

	TVector4 vecUp;
	vecUp.Multiply( TVector4::VEC_NEGY, 0.5f );

	TVector4 vecBikePos;
	vecBikePos.Add( *a_pBike->GetSteer()->GetPosition(), vecUp );

	TVector4 vecUnitPos;
	vecUnitPos.Add( *a_pUnit->GetSteer()->GetPosition(), vecUp );

	ARigidBodyCollision oCollision;
	return AVehicleManager::CollideRay( vecUnitPos, vecBikePos, &oCollision, ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_FOLIAGE | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE, 0xb0000 ) == TFALSE;
}

// $Barnyard: FUNCTION 005264d0
void PlaceBikeAndRider( AUnit* a_pPlayerBike, AUnit* a_pRider, AUnit* a_pBike, TBOOL a_bSnapToGround )
{
	AAnimatableObject* pPlayerBikeObject = TSTATICCAST( AAnimatedUnit, a_pPlayerBike )->GetAnimatableObject();
	AModelInstance*    pPlayerBikeModel  = pPlayerBikeObject->GetModelInstance();
	AAnimatableObject* pRiderObject      = TSTATICCAST( AAnimatedUnit, a_pRider )->GetAnimatableObject();
	AModelInstance*    pRiderModel       = pRiderObject->GetModelInstance();

	// Don't collide with each other
	ACollisionObject* pRiderCollision      = TSTATICCAST( AVisibleUnit, a_pRider )->GetCollisionObject();
	ACollisionObject* pPlayerBikeCollision = TSTATICCAST( AVisibleUnit, a_pPlayerBike )->GetCollisionObject();
	ACollisionObject* pBikeCollision       = TSTATICCAST( AVisibleUnit, a_pBike )->GetCollisionObject();
	pRiderCollision->SetType( pRiderCollision->GetType() | ACollisionObject::TYPE_IGNORED );
	pPlayerBikeCollision->SetType( pPlayerBikeCollision->GetType() | ACollisionObject::TYPE_IGNORED );
	pBikeCollision->SetType( pBikeCollision->GetType() | ACollisionObject::TYPE_IGNORED );

	// The rider stands where the cow riding the bike is
	TMatrix44 mTransform;
	pPlayerBikeModel->GetLocalTransform( mTransform );

	TVector4 vecPosition = mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( a_bSnapToGround )
		SnapToGround( vecPosition );

	a_pRider->GetSteer()->SetPosition( vecPosition );

	// NOTE: retail calculates the rotation but doesn't use it
	TQuaternion qUnused;
	TMatrix44::MatToQuat( qUnused, mTransform );

	TVector4 vecDirection;
	vecDirection.Multiply( mTransform.AsBasisVector4( BASISVECTOR_UP ), -1.0f );
	vecDirection.y = 0.0f;
	vecDirection.w = 1.0f;
	vecDirection.Normalise();
	a_pRider->GetSteer()->SetDirection( vecDirection );

	mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vecPosition;
	pRiderModel->SetTransform( mTransform );
	pRiderObject->Update( 0.0f );

	pRiderCollision->SetType( pRiderCollision->GetType() & ~ACollisionObject::TYPE_IGNORED );
	pPlayerBikeCollision->SetType( pPlayerBikeCollision->GetType() & ~ACollisionObject::TYPE_IGNORED );
	pBikeCollision->SetType( pBikeCollision->GetType() & ~ACollisionObject::TYPE_IGNORED );

	// The bike where the bike model of the cow riding it is
	AAnimatableObject* pBikeModelObject = AAnimatableObjectManager::GetSingleton()->GetAttachedObjectByBoneName( pPlayerBikeObject, TPS8( body_loc ), 0 );
	pBikeModelObject->UpdateAttachmentTransform();
	AModelInstance* pBikeModel = pBikeModelObject->GetModelInstance();

	TMatrix44 mBone;
	pBikeModel->GetBoneTransform( pBikeModel->GetBoneID( TPS8( bike_origin ) ), mBone );
	TVector4 vecOrigin = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

	pBikeModel->GetBoneTransform( pBikeModel->GetBoneID( TPS8( frontwheel ) ), mBone );
	TVector4 vecFrontWheel = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

	pBikeModel->GetBoneTransform( pBikeModel->GetBoneID( TPS8( rearwheel ) ), mBone );
	TVector4 vecRearWheel = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

	pBikeModel->GetBoneTransform( pBikeModel->GetBoneID( TPS8( body_loc ) ), mBone );
	TVector4 vecBody = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

	// The front wheel relative to the body
	TVector4 vecFront;
	vecFront.x = ( vecFrontWheel.x - vecRearWheel.x ) + vecBody.x;
	vecFront.y = ( vecFrontWheel.y - vecRearWheel.y ) + vecBody.y;
	vecFront.z = ( vecFrontWheel.z - vecRearWheel.z ) + vecBody.z;
	vecFront.w = vecBody.w;

	TSTATICCAST( AVisibleUnit, a_pPlayerBike )->SetVisible( TFALSE );
	TSTATICCAST( AAnimatedUnit, a_pRider )->GetAnimatableObject()->SetSkeletonUpdating( TTRUE, TTRUE );
	TSTATICCAST( AAnimatedUnit, a_pBike )->GetAnimatableObject()->SetSkeletonUpdating( TTRUE, TTRUE );

	if ( !a_bSnapToGround )
		return;

	TVector4 vecFrontGround = vecFront;
	SnapToGround( vecFrontGround );

	TVector4 vecBodyGround = vecBody;
	SnapToGround( vecBodyGround );

	TFLOAT fFrontDrop = TVector4( vecFrontGround.x - vecFront.x, vecFrontGround.y - vecFront.y, vecFrontGround.z - vecFront.z, vecFront.w ).Magnitude();
	TFLOAT fBodyDrop  = TVector4( vecBodyGround.x - vecBody.x, vecBodyGround.y - vecBody.y, vecBodyGround.z - vecBody.z, vecBody.w ).Magnitude();

	TVector4 vecOriginOffset( vecOrigin.x - vecBody.x, vecOrigin.y - vecBody.y, vecOrigin.z - vecBody.z, vecOrigin.w );

	TVector4 vecBikeDirection;
	TVector4 vecBikePosition;

	if ( fFrontDrop > 0.05f || fBodyDrop > 0.05f )
	{
		// Follow the ground
		vecBikeDirection.x = vecFrontGround.x - vecBodyGround.x;
		vecBikeDirection.y = vecFrontGround.y - vecBodyGround.y;
		vecBikeDirection.z = vecFrontGround.z - vecBodyGround.z;
		vecBikeDirection.w = vecFrontGround.w;
		vecBikeDirection.Normalise();

		TVector4 vecOffset;
		vecOffset.Multiply( vecBikeDirection, vecOriginOffset.Magnitude() );

		vecBikePosition.x = vecOffset.x + vecBodyGround.x;
		vecBikePosition.y = vecOffset.y + vecBodyGround.y;
		vecBikePosition.z = vecOffset.z + vecBodyGround.z;
		vecBikePosition.w = vecBodyGround.w;
	}
	else
	{
		vecBikeDirection.Substract( vecFront, vecBody );
		vecBikeDirection.Normalise();

		TVector4 vecOffset;
		vecOffset.Multiply( vecBikeDirection, vecOriginOffset.Magnitude() );
		vecBikePosition.Add( vecBody, vecOffset );
	}

	a_pBike->GetSteer()->SetDirection( vecBikeDirection );
	a_pBike->GetSteer()->SetPosition( vecBikePosition );
}

// $Barnyard: FUNCTION 00526f90
void PlaceBikeAndRider( TRef<AUnitProxy> a_pPlayerBike, TRef<AUnitProxy> a_pRider, TRef<AUnitProxy> a_pBike, TBOOL a_bSnapToGround )
{
	PlaceBikeAndRider( a_pPlayerBike ? a_pPlayerBike->GetUnit() : TNULL, a_pRider ? a_pRider->GetUnit() : TNULL, a_pBike ? a_pBike->GetUnit() : TNULL, a_bSnapToGround );
}

// $Barnyard: FUNCTION 00526cc0
void MoveAttachedObjects( AAnimatableObject* a_pFrom, AAnimatableObject* a_pTo, const TPString8& a_rcBone )
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	for ( AAnimatableObject* pObject = pManager->GetAttachedObjectByBoneName( a_pFrom, a_rcBone, 0 ); pObject; pObject = pManager->GetAttachedObjectByBoneName( a_pFrom, a_rcBone, 0 ) )
	{
		TINT  iBone       = a_pTo->GetModelInstance()->GetBoneID( a_rcBone );
		TINT8 iObjectBone = TINT8( pManager->GetAttachmentBone( pObject ) );

		pManager->DetachObject( pObject, a_pFrom );
		pManager->AttachObject( pObject, a_pTo, iObjectBone, TINT8( iBone ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		pObject->SetSkeletonUpdating( TTRUE, TTRUE );
		pObject->SetOpacity( 1.0f, TTRUE );
		pObject->SetAnimControllersDisabled( TFALSE );
	}
}

// $Barnyard: FUNCTION 00526dd0
AGlowViewport::GlowObject* CreateHeadlightGlow( AUnit* a_pUnit )
{
	AAnimatableObject* pRiderObject = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();
	AAnimatableObject* pBikeObject  = AAnimatableObjectManager::GetSingleton()->GetAttachedObjectByBoneName( pRiderObject, TPS8( body_loc ), 0 );

	pRiderObject->GetModelInstance()->SetReceivesLight( TFALSE );
	pBikeObject->GetModelInstance()->SetReceivesLight( TFALSE );

	TSceneObject* pSceneObject = pBikeObject->GetModelInstance()->GetSceneObject();

	AGlowViewport::GlowObject* pGlowObject = AGlowViewport::GetSingleton()->CreateGlowObject();
	pGlowObject->m_pSceneObject            = pSceneObject;
	pGlowObject->m_iAttachBone             = pSceneObject->GetInstance()->GetSkeletonInstance()->GetSkeleton()->GetBoneID( TPS8( headlight_loc ).GetString(), 0 );

	static TVector4 s_vecOffset( 0.0f, 1.0f, -2.2f, 1.0f ); // 0x007838ac

	TMatrix44 mTransform = TMatrix44::IDENTITY;
	mTransform.RotateX( 20.0f * 0.017453292f ); // 0x00773e6c degrees
	mTransform.TransformVector( mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ), mTransform, s_vecOffset );
	pGlowObject->m_oTransform = mTransform;

	return pGlowObject;
}

// $Barnyard: FUNCTION 00527060
void MoveHeadAttachments( AUnit* a_pFrom, AUnit* a_pTo )
{
	if ( a_pFrom && a_pTo )
		MoveAttachedObjects( TSTATICCAST( AAnimatedUnit, a_pFrom )->GetAnimatableObject(), TSTATICCAST( AAnimatedUnit, a_pTo )->GetAnimatableObject(), TPS8( c_head ) );
}

} // namespace ABikeHelpers
