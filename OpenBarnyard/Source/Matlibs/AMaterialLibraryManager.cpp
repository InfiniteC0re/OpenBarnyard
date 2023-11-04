#include "pch.h"
#include "AMaterialLibraryManager.h"

TOSHI_NAMESPACE_USING


AMaterialLibraryManager::AMaterialLibraryManager() :
	m_Tree1(AMemory::GetAllocator(AMemory::POOL_StringPool)),
	m_LoadedLibraries(AMemory::GetAllocator(AMemory::POOL_StringPool))
{
	
}
