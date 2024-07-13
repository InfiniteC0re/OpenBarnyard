#include "pch.h"
#include "THookedClass.h"

void THookedClass::RecurseTree( RecurseTreeBaseBeginCb_t fBaseBegin, RecurseTreeBaseEndCb_t fBaseEnd, RecurseTreeCheck_t fCheck, void* custom )
{
	TBOOL valid = fCheck( this, custom );

	if ( valid )
	{
		if ( fBaseBegin ) fBaseBegin( this, custom );
		RecurseTree2( fBaseBegin, fBaseEnd, fCheck, custom );
		if ( fBaseEnd ) fBaseEnd( this, custom );
	}
}

void THookedClass::RecurseTree2( RecurseTreeBaseBeginCb_t fBaseBegin, RecurseTreeBaseEndCb_t fBaseEnd, RecurseTreeCheck_t fCheck, void* custom )
{
	for ( THookedClass* pClass = m_pLastChildren; pClass != TNULL; pClass = pClass->m_pPrevious )
	{
		if ( fCheck ) fCheck( pClass, custom );

		if ( pClass->m_pLastChildren )
		{
			if ( fBaseBegin ) fBaseBegin( pClass, custom );
			pClass->RecurseTree2( fBaseBegin, fBaseEnd, fCheck, custom );
			if ( fBaseEnd ) fBaseEnd( pClass, custom );
		}
	}
}

class THookedObject* THookedClass::CreateObject() const
{
	if ( m_fnCreate != TNULL )
	{
		return m_fnCreate();
	}

	return TNULL;
}

THookedClass* THookedClass::FindRecurse( const TCHAR* const& name, THookedClass* parent, TBOOL hasPrevious )
{

}
