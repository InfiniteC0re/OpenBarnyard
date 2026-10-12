#include "pch.h"
#include "AKinematicsObstructionManager.h"
#include "Memory/AMemory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00573800, CreateObject 00573af0, CreateObjectInPlace 00573b50
TDEFINE_CLASS( AKinematicsObstructionManager );

// $Barnyard: FUNCTION 00573a70
AKinematicsObstructionManager::AKinematicsObstructionManager()
    : m_oPool( AMemory::GetAllocator( AMemory::POOL_Misc ), MAX_NUM_POOLED_OBSTRUCTIONS ) // T2DerivedDynamicObjectPool<AKinematicsObstruction> ctor (00573960)
{
}

// $Barnyard: FUNCTION 005738e0
// $Barnyard: FUNCTION 005739e0
AKinematicsObstructionManager::~AKinematicsObstructionManager()
{
	// NOTE: the pool frees its memory in its destructor (005738c0)
}

// $Barnyard: FUNCTION 00573810
TBOOL AKinematicsObstructionManager::AddObstruction( AKinematicsObstruction* a_pObstruction )
{
	if ( !a_pObstruction )
		return TFALSE;

	if ( !m_vecObstructions.FindItem( a_pObstruction ) )
		m_vecObstructions.PushBack( a_pObstruction );

	return TTRUE;
}

// $Barnyard: FUNCTION 00573840
void AKinematicsObstructionManager::RemoveObstruction( AKinematicsObstruction* a_pObstruction )
{
	if ( a_pObstruction )
		m_vecObstructions.FindAndErase( a_pObstruction );
}

// $Barnyard: FUNCTION 00573a00
AKinematicsObstruction* AKinematicsObstructionManager::CreateObstruction( TClass* a_pClass )
{
	// NOTE: retail calls the pool's Malloc( a_pClass->GetClassSize() ), which ignores the size
	AKinematicsObstruction* pMemory = m_oPool.AllocateObject();
	return TSTATICCAST( AKinematicsObstruction, a_pClass->CreateObjectInPlace( pMemory ) );
}

// $Barnyard: FUNCTION 00573a30
void AKinematicsObstructionManager::DeleteObstruction( AKinematicsObstruction* a_pObstruction )
{
	m_oPool.DeleteObject( a_pObstruction );
}
