#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TQList.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TPString8.h>

#include "AUnitProxy.h"
#include "GameInterface/AHackedQuickSave.h"

class AUnit;
class AUnitType;

//-----------------------------------------------------------------------------
// A slot of an AInventoryComponent: the (hidden) unit that is kept in the
// inventory and how many of its type the slot holds.
//-----------------------------------------------------------------------------
class AInventoryItem
    : public Toshi::TObject
    , public Toshi::TQList<AInventoryItem>::TNode
{
public:
	TDECLARE_CLASS( AInventoryItem, Toshi::TObject );

	static constexpr TINT MAX_COUNT           = 20;
	static constexpr TINT MAX_SCREEN_NAME_LEN = 32;

public:
	AInventoryItem();
	~AInventoryItem();

	virtual TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer ); // +0x0c
	virtual TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer ); // +0x10, respawns the unit

	// Puts a_pUnit in the slot, held by a_pHolder, and hides it.
	TBOOL Set( AUnit* a_pUnit, AUnit* a_pHolder, TINT a_iCount );

	// Spawns a unit of the type a_rcTypeName (or "apple" if it can't be spawned) and puts it in the slot.
	TBOOL Set( const Toshi::TPString8& a_rcTypeName, AUnit* a_pHolder, TINT a_iCount );

	// Puts a_pUnit in the slot instead of the current unit without changing the count and
	// disables it (without hiding it). The old unit is left as it is.
	void ReplaceUnit( AUnit* a_pUnit, AUnit* a_pHolder );

	// Releases the unit and empties the slot.
	void Reset();

	// Takes the unit out of the world: stops its AI, movement and queries. Also makes
	// it invisible if a_bHide is set.
	void DisableUnit( TBOOL a_bHide );

	void AddCount( TINT a_iCount );
	void RemoveCount( TINT a_iCount );
	void SetCount( TINT a_iCount );

	// The screen name of the unit or, if it has none, the name of its type.
	const TWCHAR* GetScreenName();

	// The name of the type of the unit, empty if the slot is empty.
	Toshi::TPString8 GetTypeName() const;

	AUnit*     GetUnit() const;
	AUnitType* GetUnitType() const;
	TINT       GetCount() const { return m_iCount; }

private:
	Toshi::TRef<AUnitProxy> m_oUnit;                                // 0x0c
	TINT                    m_iCount;                               // 0x10, at most MAX_COUNT
	TWCHAR                  m_wszScreenName[ MAX_SCREEN_NAME_LEN ]; // 0x14, see GetScreenName
};

TSTATICASSERT( sizeof( AInventoryItem ) == 0x54 );
