#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TGlobalEmitter.h>

#include "AUnitProxy.h"

class AUnit;
class AUnitManager;

// Empty event thrown through TGlobalEmitter when the id map is destroyed
// (and by AUIDMap::ThrowClearEvent). Listener list lives at 0x0077e1ec.
struct AUIDMapClearEvent
{
};

// Maps unit ids to refcounted AUnitProxy handles (method names from JPOG's exports). Fixed 2048-bucket hash
// (id & 0x7ff), proxies drawn from a fixed pool linked on a free list.
class AUIDMap : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AUIDMap, Toshi::TObject );

	static constexpr TUINT NUM_BUCKETS = 2048;
	static constexpr TUINT BUCKET_MASK = NUM_BUCKETS - 1;
	static constexpr TINT  NUM_PROXIES = 340;

public:
	AUIDMap();
	~AUIDMap();

	// NOTE: folded with other +0x04 setters in the retail binary (0066a1e0)
	void SetOwner( AUnitManager* a_pOwner ) { m_pOwner = a_pOwner; }

	// Binds a_pUnit under its own id.
	void Store( AUnit* a_pUnit, Toshi::TRef<AUnitProxy>& a_rHandle );

	// Throws AUIDMapClearEvent; always returns TTRUE.
	TBOOL ThrowClearEvent();

	// Binds id to a_pData, creating the proxy if needed, and points the handle
	// at it.
	void Store( AUnit* a_pData, TUINT a_iId, Toshi::TRef<AUnitProxy>& a_rHandle );

	// Points the handle at id's proxy, creating an empty one if it doesn't exist.
	void GetProxy( TUINT a_iId, Toshi::TRef<AUnitProxy>& a_rHandle );

private:
	friend class AUnitProxy;

	AUnitProxy* NewProxy( Toshi::TNodeList<AUnitProxy>& a_rBucket );

	// Unlinks a_pProxy from its bucket and returns it to the free list.
	void DeleteProxy( AUnitProxy* a_pProxy );

private:
	AUnitManager*                m_pOwner;                  // 0x04, set by AUnitManager::OnCreate
	AUnitProxy*                  m_pProxies;                // 0x08, pool of NUM_PROXIES entries
	Toshi::TNodeList<AUnitProxy> m_aBuckets[ NUM_BUCKETS ]; // 0x0c
	Toshi::TNodeList<AUnitProxy> m_FreeList;                // 0x800c
};
