#include "pch.h"
#include "APlayerKnowledgeManager.h"
#include "Memory/AMemory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005d2720, CreateObject 005d29b0, CreateObjectInPlace 005d2a10
TDEFINE_CLASS( APlayerKnowledgeManager );

// $Barnyard: FUNCTION 005d27d0
APlayerKnowledgeManager::APlayerKnowledgeManager()
    : m_mapValues( AMemory::GetAllocator( AMemory::POOL_Misc ) )
    , m_iNumKnowledge( 0 )
{
}

// $Barnyard: FUNCTION 005d2a50
// $Barnyard: FUNCTION 005d2ae0
APlayerKnowledgeManager::~APlayerKnowledgeManager()
{
	// NOTE: the map (T2Map destructor, 0059adc0) and the strings are destroyed by their destructors
}

// $Barnyard: FUNCTION 005d2740
TINT APlayerKnowledgeManager::FindKnowledge( TPString8 a_strName )
{
	for ( TINT i = 0; i < m_iNumKnowledge; i++ )
	{
		if ( m_aKnowledge[ i ] == a_strName )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005d2850
TINT APlayerKnowledgeManager::AddKnowledge( TPString8 a_strName )
{
	m_aKnowledge[ m_iNumKnowledge ] = a_strName;
	return m_iNumKnowledge++;
}

// $Barnyard: FUNCTION 005d28e0
TPString8 APlayerKnowledgeManager::GetKnowledgeValue( TPString8 a_strName )
{
	auto it = m_mapValues.Find( FindKnowledge( a_strName ) ); // T2RedBlackTree::Find (00596f80)

	if ( it == m_mapValues.End() )
		return TPString8();

	return it->GetSecond(); // T2Pair second by value (00597400, folded, marked in AInputMapManager.cpp)
}
