#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TNodeList.h>

class AUnit;
class AUIDMap;

// A pooled, refcounted handle to a unit addressed by id. While bound it sits in
// an AUIDMap bucket; while spare it sits on the map's free list.
class AUnitProxy
    : public Toshi::TObject
    , public Toshi::TRefCounted
    , public Toshi::TNodeList<AUnitProxy>::TNode
{
public:
	TDECLARE_CLASS( AUnitProxy, Toshi::TObject );

public:
	AUnitProxy();
	~AUnitProxy();

	// Returns the proxy to its owning map's free list instead of freeing it.
	virtual void Delete() OVERRIDE;

	AUnit* GetUnit() const { return m_pData; }
	void   SetUnit( AUnit* a_pUnit ) { m_pData = a_pUnit; }
	TUINT  GetId() const { return m_iId; }

private:
	friend class AUIDMap;

	AUnit*   m_pData;  // 0x14
	TUINT    m_iId;    // 0x18
	AUIDMap* m_pOwner; // 0x1c
};
