#pragma once
#include "AUnitStaticData.h"
#include "AInventoryComponentData.h"

//-----------------------------------------------------------------------------
// Static data of a unit type whose items are split into categories
// ("AInventoryGroupComponentData" block, one "CategoryType" block per
// category), creates an AInventoryGroupComponent for every unit of the type.
//
// TClass 0x007b49c0 (version 2.0, size 0x14, runtime creatable), vtable 0x00737b3c.
//-----------------------------------------------------------------------------
class AInventoryGroupComponentData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AInventoryGroupComponentData, AUnitStaticData );

public:
	AInventoryGroupComponentData();
	~AInventoryGroupComponentData();

	//-----------------------------------------------------------------------------
	// AUnitStaticData
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

	TINT                     GetNumCategories() const { return m_iNumCategories; }
	AInventoryComponentData* GetCategory( TINT a_iIndex ) const { return &m_pCategories[ a_iIndex ]; }

private:
	TINT                     m_iNumCategories; // 0x0c
	AInventoryComponentData* m_pCategories;    // 0x10, "CategoryType" blocks
};

TSTATICASSERT( sizeof( AInventoryGroupComponentData ) == 0x14 );
