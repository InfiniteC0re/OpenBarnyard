#pragma once
#include "AUnitStaticData.h"
#include "AInventoryItemData.h"

#include <Toshi/T2SList.h>
#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Static data of a unit type that keeps items ("AInventoryComponentData"
// block), creates an AInventoryComponent for every unit of the type. Also a
// category of an AInventoryGroupComponentData ("CategoryType" block).
//
// TClass 0x007b4a20 (version 1.0, size 0x1c, runtime creatable), vtable 0x00737be0.
//-----------------------------------------------------------------------------
class AInventoryComponentData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AInventoryComponentData, AUnitStaticData );

public:
	// NOTE: retail leaves m_iMaxItems and m_iScreenName uninitialised, Create sets them
	AInventoryComponentData();
	~AInventoryComponentData();

	//-----------------------------------------------------------------------------
	// AUnitStaticData
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

	TINT                                      GetMaxItems() const { return m_iMaxItems; }
	const Toshi::TPString8&                   GetCategoryName() const { return m_strCategoryName; }
	TINT                                      GetScreenName() const { return m_iScreenName; }
	const Toshi::T2SList<AInventoryItemData>& GetItems() const { return m_llItems; }

private:
	TINT                               m_iMaxItems;       // 0x0c, "MaxItems" (5 by default), number of slots
	Toshi::TPString8                   m_strCategoryName; // 0x10, "CategoryName"
	TINT                               m_iScreenName;     // 0x14, "ScreenName", locale string id, -1 if not set
	Toshi::T2SList<AInventoryItemData> m_llItems;         // 0x18, "Item" blocks, the starting items
};

TSTATICASSERT( sizeof( AInventoryComponentData ) == 0x1c );
