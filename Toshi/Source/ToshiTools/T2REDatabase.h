#pragma once
#include "Toshi/TSingleton.h"
#include "Toshi/T2String.h"
#include "Toshi/T2ManagedPtr.h"
#include "Toshi/T2Map.h"
#include "Toshi/T2DList.h"

#ifdef TOSHI_DEBUG

TOSHI_NAMESPACE_START

extern class T2REDatabase* g_pREDatabase;

#define REDB_CLASS()
#define REDB_MEMBER(CLASS, TYPE, NAME) inline static T2REDBClassMemberInitialiser REDBFIELD_##NAME { #CLASS, #NAME, #TYPE, sizeof(##CLASS::##NAME), offsetof(##CLASS, ##NAME) };

//-----------------------------------------------------------------------------
// Purpose: Manages info about different methods, classes that can be
// exported in a file in order to do any kind of comparisons with the original
// code.
//-----------------------------------------------------------------------------
class T2REDatabase :
	public TSingleton<T2REDatabase>
{
public:
	struct ClassMemberInfo :
		public T2DList<ClassMemberInfo>::Node
	{
		const TCHAR* pchName;
		const TCHAR* pchType;
		TSIZE uiSize;
		TSIZE uiOffset;
	};

	class ClassInfo
	{
	public:
		ClassInfo( const TCHAR* a_pchName );

		ClassMemberInfo* FindMember( const TCHAR* a_pchName );
		ClassMemberInfo* AddMember( const TCHAR* a_pchName, const TCHAR* a_pchType, TSIZE a_uiSize, TSIZE a_uiOffset );

	private:
		const TCHAR* m_pchName;
		T2DList<ClassMemberInfo> m_Members;
	};

public:
	ClassInfo* FindClass( const TCHAR* a_pchClassName );
	ClassInfo* AddClass( const TCHAR* a_pchClassName );

	void Dump();

private:
	T2Map<const TCHAR*, ClassInfo*, T2String8Comparator> m_mapClasses;
};

//-----------------------------------------------------------------------------
// Purpose: Helps to add member of a class/structure to the list.
//-----------------------------------------------------------------------------
class T2REDBClassMemberInitialiser
{
public:
	T2REDBClassMemberInitialiser( const TCHAR* a_pchClassName, const TCHAR* a_pchMemberName, const TCHAR* a_pchType, TSIZE a_uiSize, TSIZE a_uiOffset )
	{
		g_pREDatabase
			->AddClass( a_pchClassName )
			->AddMember( a_pchMemberName, a_pchType, a_uiSize, a_uiOffset );
	}
};

TOSHI_NAMESPACE_END

#endif // TOSHI_DEBUG
