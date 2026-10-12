#include "pch.h"
#include "ABarnyardCollision.h"
#include "ACollisionManager.h"
#include "ACollisionObjectSimpleDummy.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Flags of the ACollisionManager::CollideSweptSphere calls
static constexpr TUINT BARNYARDCOLLISION_SWEEP_FLAGS = ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE;

// NOTE: inlined in AGameSystemManager::OnCreate
ABarnyardCollision::ABarnyardCollision()
{
}

// $Barnyard: FUNCTION 004019e0
// $Barnyard: FUNCTION 004019c0
ABarnyardCollision::~ABarnyardCollision()
{
}

// $Barnyard: FUNCTION 00653a10
TBOOL ABarnyardCollision::CollideImpl( APhysics::ARay* a_pShape, TFLOAT* a_pDistance, Toshi::TVector4* a_pNormal )
{
	ACollisionObjectSimpleDummy oObject;
	oObject.Create( a_pShape->vOrigin, 0.0f, 0.0f, 0.0f, 0, TNULL );

	TFLOAT fStepHeight  = 0.0f;
	TFLOAT fPenetration = 0.0f;

	// NOTE: the original passes a pointer to a local as the unused parameter
	return ACollisionManager::GetSingleton()->CollideSweptSphere( a_pShape->vOrigin, 0.001f, &oObject, a_pShape->vDirection, *a_pDistance, *a_pNormal, 0.0f, fStepHeight, fPenetration, 0, BARNYARDCOLLISION_SWEEP_FLAGS, TNULL, 0 ) != 0;
}

// $Barnyard: FUNCTION 00653af0
TBOOL ABarnyardCollision::CollideImpl( APhysics::ASphere* a_pShape, TFLOAT* a_pDistance, Toshi::TVector4* a_pNormal )
{
	ACollisionObjectSimpleDummy oObject;
	oObject.Create( a_pShape->vOrigin, a_pShape->fRadius, 0.0f, 0.0f, 0, TNULL );

	return ACollisionManager::CollideSphereWithWorld( a_pShape->vOrigin, &oObject, *a_pDistance, *a_pNormal, ACollisionObject::TYPE_IGNORED, 0 );
}

// $Barnyard: FUNCTION 00653bb0
TBOOL ABarnyardCollision::CollideImpl( APhysics::ASweptSphere* a_pShape, TFLOAT* a_pDistance, Toshi::TVector4* a_pNormal )
{
	ACollisionObjectSimpleDummy oObject;
	oObject.Create( a_pShape->vOrigin, a_pShape->fRadius, a_pShape->fLength, 0.0f, 0, TNULL );

	TFLOAT fStepHeight  = 0.0f;
	TFLOAT fPenetration = 0.0f;

	// NOTE: the original passes a pointer to a local as the unused parameter
	return ACollisionManager::GetSingleton()->CollideSweptSphere( a_pShape->vOrigin, a_pShape->fRadius, &oObject, a_pShape->vDirection, *a_pDistance, *a_pNormal, 0.0f, fStepHeight, fPenetration, 0, BARNYARDCOLLISION_SWEEP_FLAGS, TNULL, 0 ) != 0;
}
