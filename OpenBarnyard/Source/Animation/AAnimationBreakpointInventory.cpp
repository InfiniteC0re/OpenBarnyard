#include "pch.h"
#include "AAnimationBreakpointInventory.h"
#include "AAnimatableObject.h"
#include "Units/AUnit.h"
#include "Units/AInventoryGroupComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00584860, CreateObject 00584870, CreateObjectInPlace 005848e0 (the constructor is inlined in both).
// The destructor is folded with other empty TObject destructors (0057fd50).
TDEFINE_CLASS( AAnimationBreakpointInventory );

TPSTRING8_DECLARE( Mode );
TPSTRING8_DECLARE( HandToInventory );
TPSTRING8_DECLARE( InventoryToHand );

// 0x007750b4
static TPString8* s_apModeNames[ AAnimationBreakpointInventory::MODE_NUMOF ] = {
	&TPS8( HandToInventory ),
	&TPS8( InventoryToHand ),
};

AAnimationBreakpointInventory::AAnimationBreakpointInventory()
{
}

AAnimationBreakpointInventory::~AAnimationBreakpointInventory()
{
}

// $Barnyard: FUNCTION 00584930
TBOOL AAnimationBreakpointInventory::Create( const PBProperties* a_pProperties )
{
	TBOOL bResult = AAnimationBreakpoint::Create( a_pProperties );

	TPString8 strMode;
	a_pProperties->GetOptionalPropertyValue( strMode, TPS8( Mode ) );

	TINT i = 0;
	for ( ; i < MODE_NUMOF; i++ )
	{
		if ( strMode == *s_apModeNames[ i ] )
			break;
	}

	m_eMode = i;

	return bResult;
}

// $Barnyard: FUNCTION 005849d0
void AAnimationBreakpointInventory::Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime )
{
	if ( m_eMode != MODE_HAND_TO_INVENTORY )
		return;

	// Puts every item in the "Hand" of the owner away
	AUnit*                    pOwner     = TSTATICCAST( AUnit, a_pObject->GetOwner() );
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pOwner->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	AInventoryComponent*      pHand      = pInventory->GetCategory( TPS8D( "Hand" ) );

	if ( pHand->GetNumItems() > 0 )
	{
		do
		{
			if ( pHand->GetNumItems() < 1 )
				return;
		} while ( AInventoryGroupComponent::CollectUnit( pHand->DropItem( pHand->GetItems().Begin() ), TNULL ) );
	}
}
