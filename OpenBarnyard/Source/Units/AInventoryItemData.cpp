#include "pch.h"
#include "AInventoryItemData.h"
#include "AInventoryItem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( Name );
TPSTRING8_DECLARE( Count );

// $Barnyard: FUNCTION 00626d40
AInventoryItemData::AInventoryItemData()
    : m_iCount( 0 )
{
}

// $Barnyard: FUNCTION 00626d60
// $Barnyard: FUNCTION 00627350
AInventoryItemData::~AInventoryItemData()
{
}

// $Barnyard: FUNCTION 00626820
TBOOL AInventoryItemData::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strName, TPS8( Name ) );
	a_pProperties->GetOptionalPropertyValue( m_iCount, TPS8( Count ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 00627370
TBOOL AInventoryItemData::Apply( AInventoryItem* a_pItem, AUnit* a_pHolder )
{
	return a_pItem->Set( m_strName, a_pHolder, m_iCount );
}
