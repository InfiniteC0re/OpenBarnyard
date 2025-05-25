#include "pch.h"
#include "ACollisionObject.h"
#include "AObjectHashMain.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ACollisionObject );

// $Barnyard: FUNCTION 0061c2b0
ACollisionObject::ACollisionObject()
    : field3_0x18( 0 )
    , m_eType( Type_None )
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
const TVector3& ACollisionObject::GetPosition()
{
	return GetObjectPosition();
}

// $Barnyard: FUNCTION 0052de30
TBOOL ACollisionObject::VTable0x30( void*, void*, void*, void*, void*, void* )
{
	return TFALSE;
}

void ACollisionObject::VTable0x34()
{
}

TBOOL ACollisionObject::Create( TFLOAT a_fRadius )
{
	return TFALSE;
}

void* ACollisionObject::VTable0x50()
{
	return TNULL;
}

// $Barnyard: FUNCTION 0052de50
void* ACollisionObject::VTable0x54( void*, void*, void*, void*, void*, void*, void* )
{
	TASSERT( TFALSE && "Not implemented" );

	return TNULL; // does it even have a return value?
}

// $Barnyard: FUNCTION 0061f700
AObjectHash::CellObject* ACollisionObject::CreateHash()
{
	return AObjectHashMain::GetSingleton()->AssignHash( this );
}
