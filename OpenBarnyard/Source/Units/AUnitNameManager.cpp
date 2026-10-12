#include "pch.h"
#include "AUnitNameManager.h"
#include "AUnitManager.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00411d70, CreateObject 00412590, CreateObjectInPlace 004125f0
TDEFINE_CLASS_INIT( AUnitNameManager, 1, 0 );

AFunctionTable AUnitNameManager::ms_oFunctionTable( &TGetClass( AUnitNameManager ), TNULL );

// NOTE: engine string hash shared by other maps (h = h * 5 + c); owner unknown.
// $Barnyard: FUNCTION 006bb240
static TUINT UnitNameHash( const TCHAR* a_szString )
{
	TINT iHash = 0;

	for ( ; *a_szString != '\0'; a_szString++ )
		iHash = iHash * 5 + *a_szString;

	return iHash;
}

// FSM fallback: AUnitNameManager natives always run on the singleton.
// $Barnyard: FUNCTION 00411da0
static TObject* __stdcall AUnitNameManagerFunctionFallback( void* a_pUnk1, void* a_pUnk2 )
{
	return AUnitNameManager::GetSingleton();
}

// $Barnyard: FUNCTION 00412630
void AUnitNameManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AUnitNameManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<AUnit*, AUnitNameManager, TPString8>::Create( "RetrieveByName", &AUnitNameManager::RetrieveByName ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<AUnit*, AUnitNameManager, TPString8>::Create( "RetrieveByType", &AUnitNameManager::RetrieveByType ) );
}

// $Barnyard: FUNCTION 00411d80
void AUnitNameManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00412020
AUnitNameManager::AUnitNameManager()
{
	m_oMap.ppBuckets = TNULL;
	m_oMap.Clear();

	m_oMap.ppBuckets   = TSTATICCAST( Entry*, TMalloc( NUM_BUCKETS * sizeof( Entry* ) ) );
	m_oMap.iNumBuckets = NUM_BUCKETS;
	m_oMap.iCount      = 0;

	for ( TINT i = 0; i < m_oMap.iNumBuckets; i++ )
		m_oMap.ppBuckets[ i ] = TNULL;
}

// $Barnyard: FUNCTION 00412680
// $Barnyard: FUNCTION 004120d0
AUnitNameManager::~AUnitNameManager()
{
	m_oMap.Clear();

	if ( m_oMap.ppBuckets )
	{
		TFree( m_oMap.ppBuckets );
		m_oMap.ppBuckets = TNULL;
	}

	m_oMap.iNumBuckets = 0;
}

// $Barnyard: FUNCTION 004120c0
AFunctionTable* AUnitNameManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00411f30
void AUnitNameManager::Clear()
{
	m_oMap.Clear();
}

// $Barnyard: FUNCTION 00412150
void AUnitNameManager::RemoveName( TPString8 a_strName )
{
	m_oMap.Remove( a_strName );
}

// $Barnyard: FUNCTION 004121b0
TINT AUnitNameManager::GetUnitId( TPString8 a_strName )
{
	TINT iId;

	if ( !m_oMap.Find( a_strName, iId ) )
		iId = -1;

	return iId;
}

// $Barnyard: FUNCTION 00412220
AUnit* AUnitNameManager::RetrieveByName( TPString8 a_strName )
{
	return AUnitManager::GetSingleton()->FindUnitById( GetUnitId( a_strName ) );
}

// $Barnyard: FUNCTION 004122a0
AUnit* AUnitNameManager::RetrieveByType( TPString8 a_strTypeName )
{
	TNodeList<AUnit>& rUnits = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = rUnits.Begin(); it != rUnits.End(); it++ )
	{
		if ( !it->IsDying() && it->GetType()->GetName() == a_strTypeName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00412360
TPString8 AUnitNameManager::GetUnitName( TINT a_iUnitId )
{
	// NOTE: retail walks the map with its (shared) bucket iterator
	for ( TINT i = 0; i < m_oMap.iNumBuckets; i++ )
	{
		for ( Entry* pEntry = m_oMap.ppBuckets[ i ]; pEntry; pEntry = pEntry->pNext )
		{
			if ( pEntry->iUnitId == a_iUnitId )
				return pEntry->strName;
		}
	}

	return TPString8();
}

// $Barnyard: FUNCTION 004127c0
void AUnitNameManager::SetUnitName( TPString8 a_strName, TINT a_iUnitId )
{
	m_oMap.Insert( a_strName, a_iUnitId );
}

// $Barnyard: FUNCTION 00412450
AUnitNameManager::Entry::~Entry()
{
	// Buckets own their whole chain
	if ( pNext )
		delete pNext;
}

// $Barnyard: FUNCTION 00411dd0
void AUnitNameManager::UnitNameMap::Clear()
{
	if ( ppBuckets )
	{
		for ( TINT i = 0; i < iNumBuckets; i++ )
		{
			if ( ppBuckets[ i ] )
			{
				delete ppBuckets[ i ];
				ppBuckets[ i ] = TNULL;
			}
		}
	}

	iCount = 0;
}

// $Barnyard: FUNCTION 00411f40
TBOOL AUnitNameManager::UnitNameMap::Find( const TPString8& a_rcName, TINT& a_rOutId ) const
{
	TUINT uiHash = UnitNameHash( a_rcName.GetString() );

	for ( Entry* pEntry = ppBuckets[ uiHash % iNumBuckets ]; pEntry; pEntry = pEntry->pNext )
	{
		if ( pEntry->strName == a_rcName )
		{
			a_rOutId = pEntry->iUnitId;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00411fa0
TBOOL AUnitNameManager::UnitNameMap::Remove( const TPString8& a_rcName )
{
	TUINT  uiBucket = UnitNameHash( a_rcName.GetString() ) % iNumBuckets;
	Entry* pPrev    = TNULL;

	for ( Entry* pEntry = ppBuckets[ uiBucket ]; pEntry; pPrev = pEntry, pEntry = pEntry->pNext )
	{
		if ( pEntry->strName == a_rcName )
		{
			if ( !pPrev )
				ppBuckets[ uiBucket ] = pEntry->pNext;
			else
				pPrev->pNext = pEntry->pNext;

			pEntry->pNext = TNULL;
			delete pEntry;
			iCount--;

			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004126a0
void AUnitNameManager::UnitNameMap::Insert( const TPString8& a_rcName, const TINT& a_rcId )
{
	Entry** ppBucket = &ppBuckets[ UnitNameHash( a_rcName.GetString() ) % iNumBuckets ];
	Entry*  pLast    = TNULL;

	for ( Entry* pEntry = *ppBucket; pEntry; pEntry = pEntry->pNext )
	{
		pLast = pEntry;

		if ( pEntry->strName == a_rcName )
		{
			pEntry->iUnitId = a_rcId;
			return;
		}
	}

	Entry* pNew   = new Entry;
	pNew->strName = a_rcName;
	pNew->iUnitId = a_rcId;
	pNew->pNext   = TNULL;

	if ( !pLast )
		*ppBucket = pNew;
	else
		pLast->pNext = pNew;

	iCount++;
}
