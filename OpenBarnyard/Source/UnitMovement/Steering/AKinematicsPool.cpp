#include "pch.h"
#include "AKinematicsPool.h"
#include "Memory/AMemory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00574010, CreateObject 00574350, CreateObjectInPlace 005743b0
TDEFINE_CLASS( AKinematicsPool );

// $Barnyard: FUNCTION 005741c0
AKinematicsPool::AKinematicsPool()
    : m_oPool( AMemory::GetAllocator( AMemory::POOL_Misc ), MAX_NUM_KINEMATICS ) // T2DerivedDynamicObjectPool<ASteerKinematics> ctor (005740b0)
{
}

// $Barnyard: FUNCTION 00574240
// $Barnyard: FUNCTION 005743f0
AKinematicsPool::~AKinematicsPool()
{
	while ( !m_llKinematics.IsEmpty() )
	{
		ASteerKinematics* pKinematics = m_llKinematics.PopBack();
		m_oPool.DeleteObject( pKinematics );
	}

	// NOTE: the pool frees its memory in its destructor (00574090)
}

// $Barnyard: FUNCTION 00574130
ASteerKinematics* AKinematicsPool::CreateKinematics( TClass* a_pClass )
{
	// NOTE: retail calls the pool's Malloc( a_pClass->GetClassSize() ), which ignores the size
	ASteerKinematics* pMemory = m_oPool.AllocateObject();
	return TSTATICCAST( ASteerKinematics, a_pClass->CreateObjectInPlace( pMemory ) );
}

// $Barnyard: FUNCTION 00574160
void AKinematicsPool::DestroyKinematics( ASteerKinematics* a_pKinematics )
{
	// NOTE: retail unlinks unconditionally; the kinematics are never put in a list, so the node is
	// usually self-linked (a no-op unlink) and T2GenericDList::Node::Remove would assert on it
	if ( a_pKinematics->IsLinked() )
		a_pKinematics->Remove();

	m_oPool.DeleteObject( a_pKinematics );
}
