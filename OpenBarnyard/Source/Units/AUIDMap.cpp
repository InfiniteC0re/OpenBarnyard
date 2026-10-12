#include "pch.h"
#include "AUIDMap.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0040d7f0, CreateObject 0040e1b0, CreateObjectInPlace 0040e210
TDEFINE_CLASS( AUIDMap );

// $Barnyard: FUNCTION 0040dcb0
AUIDMap::AUIDMap()
{
	m_pProxies = new AUnitProxy[ NUM_PROXIES ];

	for ( TINT i = 0; i < NUM_PROXIES; i++ )
		m_FreeList.InsertHead( &m_pProxies[ i ] );
}

// $Barnyard: FUNCTION 0040e250
// $Barnyard: FUNCTION 0040de30
AUIDMap::~AUIDMap()
{
	TGlobalEmitter<AUIDMapClearEvent>::Throw( AUIDMapClearEvent() );

	// Proxies live in m_pProxies, so unlink them before freeing the pool.
	m_FreeList.RemoveAll();

	if ( TSystemManager::Quitted() )
	{
		for ( TUINT i = 0; i < NUM_BUCKETS; i++ )
			m_aBuckets[ i ].RemoveAll();
	}

	delete[] m_pProxies;
}

// $Barnyard: FUNCTION 0040df10
void AUIDMap::Store( AUnit* a_pData, TUINT a_iId, TRef<AUnitProxy>& a_rHandle )
{
	TNodeList<AUnitProxy>& rBucket = m_aBuckets[ a_iId & BUCKET_MASK ];

	AUnitProxy* pProxy = TNULL;
	for ( auto it = rBucket.Begin(); it != rBucket.End(); it++ )
	{
		if ( it->GetId() == a_iId )
		{
			pProxy    = it;
			a_rHandle = pProxy;
			break;
		}
	}

	if ( !pProxy )
	{
		pProxy = NewProxy( rBucket );

		if ( !pProxy )
		{
			// Pool is full - drop whatever the handle was holding.
			a_rHandle = TNULL;
			return;
		}

		pProxy->m_pOwner = this;
		pProxy->m_iId    = a_iId;
		a_rHandle        = pProxy;
	}

	if ( a_rHandle )
		a_rHandle->m_pData = a_pData;
}

// $Barnyard: FUNCTION 0040e270
void AUIDMap::Store( AUnit* a_pUnit, TRef<AUnitProxy>& a_rHandle )
{
	Store( a_pUnit, a_pUnit->GetId(), a_rHandle );
}

// $Barnyard: FUNCTION 0040dbc0
TBOOL AUIDMap::ThrowClearEvent()
{
	TGlobalEmitter<AUIDMapClearEvent>::Throw( AUIDMapClearEvent() );
	return TTRUE;
}

// $Barnyard: FUNCTION 0040dff0
void AUIDMap::GetProxy( TUINT a_iId, TRef<AUnitProxy>& a_rHandle )
{
	TNodeList<AUnitProxy>& rBucket = m_aBuckets[ a_iId & BUCKET_MASK ];

	for ( auto it = rBucket.Begin(); it != rBucket.End(); it++ )
	{
		if ( it->GetId() == a_iId )
		{
			a_rHandle = it.Get();
			return;
		}
	}

	// Nothing bound yet - make an empty proxy for the id.
	Store( TNULL, a_iId, a_rHandle );
}

// $Barnyard: FUNCTION 0040dbd0
AUnitProxy* AUIDMap::NewProxy( TNodeList<AUnitProxy>& a_rBucket )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original doesn't check for an exhausted pool and dereferences
	// TNULL below. Store already handles a TNULL result.
	if ( m_FreeList.IsEmpty() )
		return TNULL;
#endif

	AUnitProxy* pProxy = m_FreeList.RemoveHead();
	a_rBucket.InsertTail( pProxy );

	return pProxy;
}

// $Barnyard: FUNCTION 0040dc40
void AUIDMap::DeleteProxy( AUnitProxy* a_pProxy )
{
	a_pProxy->Remove();
	m_FreeList.InsertHead( a_pProxy );
}
