#include "ToshiPCH.h"
#include "T2REDatabase.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

T2REDatabase database;
T2REDatabase* g_pREDatabase = &database;

T2REDatabase::ClassInfo* T2REDatabase::FindClass( const TCHAR* a_pchClassName )
{
	auto pResultNode = m_mapClasses.FindNode( a_pchClassName );

	if ( pResultNode != m_mapClasses.End() )
	{
		return pResultNode->GetValue()->GetSecond();
	}

	return TNULL;
}

T2REDatabase::ClassInfo* T2REDatabase::AddClass( const TCHAR* a_pchClassName )
{
	ClassInfo* pInfo = FindClass( a_pchClassName );

	if ( !pInfo )
	{
		// Create new class info instance since it's not in the map yet
		pInfo = new ClassInfo( a_pchClassName );
		m_mapClasses.Insert( a_pchClassName, pInfo );
	}

	return pInfo;
}

void T2REDatabase::Dump()
{
	TINFO("Hello!");
}

T2REDatabase::ClassInfo::ClassInfo( const TCHAR* a_pchName )
{
	m_pchName = a_pchName;
}

T2REDatabase::ClassMemberInfo* T2REDatabase::ClassInfo::FindMember( const TCHAR* a_pchName )
{
	T2_FOREACH( m_Members, it )
	{
		if ( !T2String8::Compare( it->pchName, a_pchName ) )
		{
			return it;
		}
	}

	return TNULL;
}

T2REDatabase::ClassMemberInfo* T2REDatabase::ClassInfo::AddMember( const TCHAR* a_pchName, const TCHAR* a_pchType, TSIZE a_uiSize, TSIZE a_uiOffset )
{
	ClassMemberInfo* pInfo = FindMember( a_pchName );

	if ( !pInfo )
	{
		pInfo = new ClassMemberInfo();
		pInfo->pchName = a_pchName;
		pInfo->pchType = a_pchType;
		pInfo->uiSize = a_uiSize;
		pInfo->uiOffset = a_uiOffset;

		m_Members.PushBack( pInfo );
	}

	return pInfo;
}

struct test
{
	int a;
};

REDB_MEMBER( test, int, a );

TOSHI_NAMESPACE_END
