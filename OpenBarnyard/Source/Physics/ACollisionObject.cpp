#include "pch.h"
#include "ACollisionObject.h"
#include "AObjectHashMain.h"
#include "ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ACollisionObject );

// $Barnyard: FUNCTION 0061c2b0
ACollisionObject::ACollisionObject()
    : m_pModelInstance( TNULL )
    , m_eType( TYPE_NONE )
{
}

// $Barnyard: FUNCTION 0061c390
// $Barnyard: FUNCTION 0061c320
// $Barnyard: FUNCTION 0061c330
ACollisionObject::~ACollisionObject()
{
}

// $Barnyard: FUNCTION 0052de40
TFLOAT ACollisionObject::GetRadius()
{
	return GetObjectRadius() * 1.2f;
}

// $Barnyard: FUNCTION 004130f0
const TVector4& ACollisionObject::GetPosition()
{
	return GetObjectPosition();
}

// $Barnyard: FUNCTION 0052de30
TBOOL ACollisionObject::CollideShape( void*, void*, void*, void*, void*, void* )
{
	return TFALSE;
}

void ACollisionObject::UpdateTransform()
{
}

TBOOL ACollisionObject::UpdateHash( TFLOAT a_fRadius )
{
	return TFALSE;
}

void* ACollisionObject::GetOwnerUnit()
{
	return TNULL;
}

// $Barnyard: FUNCTION 0052de50
TBOOL ACollisionObject::ReciprocateSweptSphere( ACollisionObject* a_pObject, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	return a_pObject->CollideSweptSphere( this, a_rfDistance, a_rcDirection, a_rcCenter, a_rNormal, a_fMaxStepHeight, a_rfStepHeight );
}

// $Barnyard: FUNCTION 0061c590
// $Barnyard: FUNCTION 0061c680
void ACollisionObject::UnlinkCollisionObject()
{
	AHashedObjectAuto::RemoveHash();
	Node::Remove();
}

// $Barnyard: FUNCTION 0061cc20
void ACollisionObject::Activate()
{
	ACollisionManager::GetSingleton()->AddCollisionObject( this );
	CreateHash();
}

// $Barnyard: FUNCTION 0061f700
AObjectHash::CellObject* ACollisionObject::CreateHash()
{
	return AObjectHashMain::GetSingleton()->AssignHash( this );
}
