#pragma once
#include <Toshi/T2SList.h>
#include <Toshi/TPString8.h>

#include <Plugins/PPropertyParser/PBProperties.h>

class AUnit;
class AInventoryItem;

//-----------------------------------------------------------------------------
// A starting item of an AInventoryComponentData ("Item" block): "Count"
// items of the unit type "Name".
// NOTE: vtable 0x00737d18, it isn't a TObject.
//-----------------------------------------------------------------------------
class AInventoryItemData
    : public Toshi::T2SList<AInventoryItemData>::Node
{
public:
	// NOTE: retail leaves m_iCount uninitialised
	AInventoryItemData();
	virtual ~AInventoryItemData(); // +0x00

	virtual TBOOL Create( const PBProperties* a_pProperties );        // +0x04
	virtual TBOOL Apply( AInventoryItem* a_pItem, AUnit* a_pHolder ); // +0x08, fills the slot with the item

	const Toshi::TPString8& GetName() const { return m_strName; }
	TINT                    GetCount() const { return m_iCount; }

private:
	TINT             m_iCount;  // 0x08, "Count"
	Toshi::TPString8 m_strName; // 0x0c, "Name", the unit type
};

TSTATICASSERT( sizeof( AInventoryItemData ) == 0x10 );
