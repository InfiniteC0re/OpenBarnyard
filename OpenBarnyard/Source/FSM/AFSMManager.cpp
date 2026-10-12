#include "pch.h"
#include "AFSMManager.h"
#include "AFSMParser.h"
#include "AFunctionStub.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ff9c0, CreateObject 005ffdc0, CreateObjectInPlace 005ffe20
TDEFINE_CLASS( AFSMManager );

// $Barnyard: FUNCTION 005ffb70
AFSMManager::AFSMManager()
{
	m_pMiscFunctions = new AMiscFunctions();
}

// $Barnyard: FUNCTION 005ffc20
// $Barnyard: FUNCTION 005ffab0
AFSMManager::~AFSMManager()
{
}

// $Barnyard: FUNCTION 005ffb20
TBOOL AFSMManager::OnCreate()
{
	// NOTE: not in retail, the scripts can only be parsed if all natives they use are registered
	AFunctionStub::RegisterStubTables();

	ParseFSMs( 0, "allfsms" );
	return TTask::OnCreate();
}

// $Barnyard: FUNCTION 005ffc40
void AFSMManager::OnDestroy()
{
	if ( m_pMiscFunctions )
		delete m_pMiscFunctions;

	T2_FOREACH_ARRAY( m_aMachineLists, i )
	{
		while ( !m_aMachineLists[ i ].IsEmpty() )
		{
			AFSM* pFSM = m_aMachineLists[ i ].Head();
			pFSM->Remove();
			delete pFSM;
		}
	}

	// NOTE: not in retail
	AFunctionStub::UnregisterStubTables();

	TTask::OnDestroy();
}

// $Barnyard: FUNCTION 005ffe60
AFSM* AFSMManager::CreateStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName )
{
	AFSM* pFSM = FindStateMachine( a_iFSMFileIndex, a_strName );

	if ( pFSM )
	{
		// It's been created by a call from another machine but not parsed yet
		if ( pFSM->HasStates() )
			return TNULL;
	}
	else
	{
		pFSM = new AFSM( a_strName );
		m_aMachineLists[ a_iFSMFileIndex ].InsertTail( pFSM );
	}

	return pFSM;
}

// $Barnyard: FUNCTION 005ffce0
AFSM* AFSMManager::FindOrCreateStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName )
{
	AFSM* pFSM = FindStateMachine( a_iFSMFileIndex, a_strName );

	if ( !pFSM )
	{
		pFSM = new AFSM( a_strName );
		m_aMachineLists[ a_iFSMFileIndex ].InsertTail( pFSM );
	}

	return pFSM;
}

// $Barnyard: FUNCTION 005ffc90
AFSM* AFSMManager::FindStateMachine( TINT a_iFSMFileIndex, const Toshi::TPString8& a_strName )
{
	T2_FOREACH( m_aMachineLists[ a_iFSMFileIndex ], it )
	{
		if ( it->GetName() == a_strName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005ffb40
void AFSMManager::DestroyStateMachine( AFSM* a_pFSM )
{
	a_pFSM->Remove();

	if ( a_pFSM )
		delete a_pFSM;
}

// $Barnyard: FUNCTION 005ffd70
void AFSMManager::ResetStateMachines()
{
	T2_FOREACH( m_aMachineLists[ 0 ], it )
	{
		it->Reset();
	}
}

// $Barnyard: FUNCTION 005ff9d0
void AFSMManager::ParseFSMs( TINT a_iFSMFileIndex, const TCHAR* a_pchFileName )
{
	TString8 strFSMsFilePath;
	TString8 strFSMsFilePathDebug;

	strFSMsFilePath.Format( "Data/Units/%s.trb", a_pchFileName );

#ifdef TOSHI_DEBUG
	// NOTE: retail never formats the debug path, the line numbers are only loaded here to have them in the errors
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
