#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Gives "count" items of "itemtype" from the inventories of the units of the
// category to the "receiver" (or generates them with "genitem"). With
// "showreward" the item reward screen is shown when the receiver is the
// category itself. "screenname" renames the item type.
//-----------------------------------------------------------------------------
class AGameState;

class AGiftActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AGiftActivity, AInteractionActivity );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE       = 0,
		FLAGS_GENITEM    = BITFLAG( 0 ), // "genitem", the items don't come from an inventory
		FLAGS_SHOWREWARD = BITFLAG( 1 ), // "showreward"
	};

public:
	AGiftActivity();
	~AGiftActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual void   GetDataSize( TINT* a_pSize ) OVERRIDE;                 // +0x10
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c

private:
	// TRUE if the units of the category have at least m_iCount items of the type.
	TBOOL HasEnoughItems( AInteractionSequence* a_pSequence );

	RESULT PushRewardState( AGameState** a_ppRewardState );

private:
	Toshi::TPString8               m_strItemType;          // 0x08, "itemtype"
	Toshi::TPString8               m_strInventoryCategory; // 0x0c, "inventorycatagory" (sic)
	TINT                           m_iCount;               // 0x10, "count"
	AInteractionSequence::CATEGORY m_eReceiver;            // 0x14, "receiver"
	TINT                           m_iScreenName;          // 0x18, "screenname", locale string id, -1 if not set
	FLAGS                          m_uiFlags;              // 0x1c
	TINT                           m_iUnk20;               // 0x20, never used
};

TSTATICASSERT( sizeof( AGiftActivity ) == 0x24 );
