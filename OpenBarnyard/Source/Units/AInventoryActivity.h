#pragma once
#include "AInteractionActivity.h"

class AUnitType;

//-----------------------------------------------------------------------------
// Changes the inventory of the units of the category ("action"): adds,
// removes or deletes "count" items of "itemtype" ("other" for the unit of the
// sequence), picks items up, puts away carried items or spills money.
//-----------------------------------------------------------------------------
class AInventoryActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AInventoryActivity, AInteractionActivity );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE           = 0,
		FLAGS_ADD            = BITFLAG( 0 ), // "add", see m_eItemCategory
		FLAGS_REMOVE         = BITFLAG( 1 ), // "remove"
		FLAGS_DELETE         = BITFLAG( 2 ), // "delete"
		FLAGS_PICKUP         = BITFLAG( 3 ), // "pickup"
		FLAGS_PICKUPDIRECT   = BITFLAG( 4 ), // "pickupdirect"
		FLAGS_PICKUPFOLLOW   = BITFLAG( 5 ), // "pickupfollow", the item follows "bone"
		FLAGS_PUTAWAYCARRY   = BITFLAG( 6 ), // "putawaycarry"
		FLAGS_SPILLMONEY     = BITFLAG( 7 ), // "spillmoney"
		FLAGS_SPILLMONEYRAND = BITFLAG( 8 ), // "spillmoneyrand"
		FLAGS_DELETEALL      = BITFLAG( 9 ), // "deleteall"
	};

public:
	AInventoryActivity();
	~AInventoryActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	AUnitType*                     m_pItemType;     // 0x08, type of "itemtype", TNULL if "other"
	Toshi::TPString8               m_strItemType;   // 0x0c, "itemtype"
	Toshi::TPString8               m_strBone;       // 0x10, "bone"
	TINT                           m_iCount;        // 0x14, "count"
	FLAGS                          m_uiFlags;       // 0x18, "action"
	AInteractionSequence::CATEGORY m_eItemCategory; // 0x1c, "itemcategory", used by "add"
	TFLOAT                         m_fMaxRadius;    // 0x20, "maxradius"
};

TSTATICASSERT( sizeof( AInventoryActivity ) == 0x24 );
