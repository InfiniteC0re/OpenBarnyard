#include "pch.h"
#include "AFSMManager.h"
#include "AFSMParser.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005ffb70
AFSMManager::AFSMManager()
{
}

// $Barnyard: FUNCTION 005ffc20
// $Barnyard: FUNCTION 005ffab0
AFSMManager::~AFSMManager()
{
}

// $Barnyard: FUNCTION 005ffb20
TBOOL AFSMManager::OnCreate()
{
	ParseFSMs( 0, "allfsms" );
	return TTRUE;
}

// $Barnyard: FUNCTION 005ffc90
AFSM* AFSMManager::CreateStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName )
{
	TASSERT( a_iFSMFileIndex < MAX_FILES );

	AFSM* pFSM = FindStateMachine( a_iFSMFileIndex, a_strName );
	if ( pFSM == NULL )
	{
		TINFO( "Creating new FSM (name=%s)\n", a_strName.GetString() );

		pFSM = new AFSM( a_strName );
		m_aMachineLists[ a_iFSMFileIndex ].InsertTail( pFSM );
	}
	else if ( !pFSM->IsEmpty() )
		return TNULL;

	return pFSM;
}

// $Barnyard: FUNCTION 005ffc90
AFSM* AFSMManager::FindStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName )
{
	TASSERT( a_iFSMFileIndex < MAX_FILES );
	T2_FOREACH( m_aMachineLists[ a_iFSMFileIndex ], it )
	{
		if ( it->GetName() == a_strName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005ffc90
void AFSMManager::DestroyStateMachine( AFSM* a_pFSM )
{
	TVALIDPTR( a_pFSM );
	a_pFSM->Remove();
	delete a_pFSM;
}

// $Barnyard: FUNCTION 005ff9d0
void AFSMManager::ParseFSMs( TINT a_iFSMFileIndex, const TCHAR* a_pchFileName )
{
	TString8 strFSMsFilePath;
	TString8 strFSMsFilePathDebug;

	strFSMsFilePath.Format( "Data/Units/%s.trb", a_pchFileName );

#ifdef TOSHI_DEBUG
	strFSMsFilePathDebug.Format( "Data/Units/%sD.trb", a_pchFileName );
#endif

	AFSMParser oParser;
	oParser.LoadFSMFile(
	    this,
	    &strFSMsFilePath,
#ifdef TOSHI_DEBUG
	    &strFSMsFilePathDebug,
#else
	    TNULL,
#endif
	    a_iFSMFileIndex
	);
}
