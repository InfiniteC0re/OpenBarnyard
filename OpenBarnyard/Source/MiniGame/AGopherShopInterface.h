#pragma once
#include "FSM/AFunctionTable.h"
#include "GameInterface/AHackedQuickSave.h"

#include <Toshi/TObject.h>

//-----------------------------------------------------------------------------
// Purpose: the purchases of the gopher shop (AGopherShopGame) and Joey's offers.
// The shop saves the purchases to its buffer of the quick save
// (AHackedQuickSave::BUFFER_OFFERS), they're read back from there:
//  TUINT8 Joey's offer (0xff if none), TINT number of items, TBOOL, TINT,
//  then the id and the count of every bought item.
// Script natives "agophershopinterface::".
// TClass 00782d18, vtable 0071ec00, size 4.
//-----------------------------------------------------------------------------
class AGopherShopInterface
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AGopherShopInterface, Toshi::TObject );

	// Script natives of the class, for the FSM
	// $Barnyard: FUNCTION 00499a20
	virtual AFunctionTable* GetFunctionTable() { return &ms_oFunctionTable; } // +0x0c

	// Number of items the percentage of GetPercentBought is out of
	static constexpr TINT NUM_ITEMS = 47;

	// An offer of Joey: a message and the item he sells cheaper
	struct JoeyOffer
	{
		TINT iTextId; // locale id of the message
		TINT iItemId;
	};

	static constexpr TINT NUM_JOEY_OFFERS = 15;

public:
	// $Barnyard: FUNCTION 00499a30
	// $Barnyard: FUNCTION 00499af0
	AGopherShopInterface() = default;

	// Returns the number of a_iItemId bought from the shop
	static TINT GetNumBought( TINT a_iItemId );

	// Whether a_iItemId was bought from the shop
	static TBOOL HasBought( TINT a_iItemId );

	// Returns the percentage of the items of the shop that were bought
	static TINT GetPercentBought();

	// Clears the purchases
	static TBOOL RemoveAllBoughtItems();

	static JoeyOffer GetJoeyOffer( TUINT8 a_uiIndex );

	// $Barnyard: FUNCTION 00499ab0
	static TINT GetNumJoeyOffers() { return NUM_JOEY_OFFERS; }

	// $Barnyard: FUNCTION 00499ac0
	// The message of Joey's current offer
	static TWCHAR* GetJoeyOfferText() { return ms_wszJoeyOfferText; }

	// The buffer of the quick save the shop is saved to
	static AHackedQuickSave::Buffer* GetShopBuffer();

public:
	static AFunctionTable ms_oFunctionTable; // 0x00782d48

private:
	static JoeyOffer     ms_aJoeyOffers[ NUM_JOEY_OFFERS ]; // 0x0071eb88
	inline static TWCHAR ms_wszJoeyOfferText[ 128 ];        // 0x00782c18
};
