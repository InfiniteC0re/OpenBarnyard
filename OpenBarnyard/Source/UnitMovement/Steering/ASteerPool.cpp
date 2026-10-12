#include "pch.h"
#include "ASteerPool.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00572100, CreateObject 00572410, CreateObjectInPlace 00572470
TDEFINE_CLASS( ASteerPool );

// $Barnyard: FUNCTION 00572350
ASteerPool::ASteerPool()
    : m_oSteerPool( GetGlobalAllocator(), MAX_NUM_STEERS, STEER_SLOT_SIZE, STEER_SLOT_ALIGNMENT )
    , m_oLargeSteerPool( GetGlobalAllocator(), MAX_NUM_LARGE_STEERS ) // T2DerivedDynamicObjectPool<ASteer> ctor (00572200)
{
}

// $Barnyard: FUNCTION 00572280
// $Barnyard: FUNCTION 00572170
ASteerPool::~ASteerPool()
{
	// NOTE: the pools free their memory in their destructors
}

// $Barnyard: FUNCTION 005722a0
ASteer* ASteerPool::CreateSteer( TClass* a_pClass )
{
	ASteer* pMemory;

	if ( TINT( a_pClass->GetClassSize() ) < LARGE_STEER_SIZE )
		pMemory = m_oSteerPool.AllocateObject();
	else
		pMemory = m_oLargeSteerPool.AllocateObject();

	return TSTATICCAST( ASteer, a_pClass->CreateObjectInPlace( pMemory ) );
}

// $Barnyard: FUNCTION 005722e0
void ASteerPool::DestroySteer( ASteer* a_pSteer )
{
	if ( TINT( a_pSteer->GetClass()->GetClassSize() ) < LARGE_STEER_SIZE )
		m_oSteerPool.DeleteObject( a_pSteer );
	else
		m_oLargeSteerPool.DeleteObject( a_pSteer );
}
