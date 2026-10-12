#pragma once
#include <Math/TVector4.h>
#include <Toshi/TPString8.h>

class AAnimatableObject;
class AModelInstance;

//-----------------------------------------------------------------------------
// Purpose: an entry of "CostMapping" of Data/MinigameData/GopherShop/GopherPriceList.trb
//-----------------------------------------------------------------------------
struct AGopherShopPrice
{
	const TCHAR* szModel;         // 0x00, the animatable object type (and the inventory type of category 3)
	const TCHAR* szInventoryType; // 0x04, category 3: the inventory type, otherwise the second name of the item
	const TCHAR* szName2;         // 0x08
	TINT         iCategory;       // 0x0c, see AGopherShopGame::CATEGORY
	TINT         iTextId;         // 0x10, the locale string of the name, also the id of the item (-1: 101)
	TINT         iPrice;          // 0x14
	TINT         iUnk18;          // 0x18, unused
	TFLOAT       fScale;          // 0x1c
	TFLOAT       fRotX;           // 0x20, degrees
	TFLOAT       fRotY;           // 0x24
	TFLOAT       fRotZ;           // 0x28
	const TCHAR* szQuest;         // 0x2c, the quest that unlocks the item
	const TCHAR* szName3;         // 0x30
	const TCHAR* szRequired;      // 0x34, comma separated models of which one has to be bought first
	TINT         iMaxCount;       // 0x38, -1: unlimited
};

//-----------------------------------------------------------------------------
// Purpose: a bought item as AGopherShopGame keeps it (and saves the id and the count)
// NOTE: Size 0x24. The special members are compiler generated in retail.
//-----------------------------------------------------------------------------
struct AGopherShopPurchase
{
	// NOTE: retail only nulls the strings, the integers are zeroed by the member initialisers here
	// $Barnyard: FUNCTION 00493400
	AGopherShopPurchase() {}

	// $Barnyard: FUNCTION 0049b380
	AGopherShopPurchase( const AGopherShopPurchase& a_rcOther )
	    : iId( a_rcOther.iId )
	    , iCount( a_rcOther.iCount )
	    , iCategory( a_rcOther.iCategory )
	    , iMaxCount( a_rcOther.iMaxCount )
	    , strModel( a_rcOther.strModel )
	    , strInventoryType( a_rcOther.strInventoryType )
	    , strName2( a_rcOther.strName2 )
	    , strQuest( a_rcOther.strQuest )
	    , strName3( a_rcOther.strName3 )
	{}

	// $Barnyard: FUNCTION 00493320
	~AGopherShopPurchase() {}

	// $Barnyard: FUNCTION 004938a0
	AGopherShopPurchase& operator=( const AGopherShopPurchase& a_rcOther )
	{
		iId              = a_rcOther.iId;
		iCount           = a_rcOther.iCount;
		iCategory        = a_rcOther.iCategory;
		iMaxCount        = a_rcOther.iMaxCount;
		strModel         = a_rcOther.strModel;
		strInventoryType = a_rcOther.strInventoryType;
		strName2         = a_rcOther.strName2;
		strQuest         = a_rcOther.strQuest;
		strName3         = a_rcOther.strName3;

		return *this;
	}

	TINT             iId       = 0;    // 0x00, the locale id of the name of the item
	TINT             iCount    = 0;    // 0x04
	TINT             iCategory = 0;    // 0x08
	TINT             iMaxCount = 0;    // 0x0c
	Toshi::TPString8 strModel;         // 0x10
	Toshi::TPString8 strInventoryType; // 0x14
	Toshi::TPString8 strName2;         // 0x18
	Toshi::TPString8 strQuest;         // 0x1c
	Toshi::TPString8 strName3;         // 0x20
};

//-----------------------------------------------------------------------------
// Purpose: an item of the gopher shop, shown as a 3D model by AGopherShopCategory
//-----------------------------------------------------------------------------
class AGopherShopItem
{
public:
	static constexpr TINT MAX_REQUIRED = 11;

public:
	AGopherShopItem();
	~AGopherShopItem();

	// Creates the item (and its model) described by a_rcPrice
	static AGopherShopItem* Create( const AGopherShopPrice& a_rcPrice );

	// Places the model of a_pItem: a_rcOffset rotates it (as euler angles), a_rcPosition is used if a_bSetPosition
	static void UpdateTransform( AGopherShopItem* a_pItem, TBOOL a_bSetPosition, Toshi::TVector4 a_vecPosition, Toshi::TVector4 a_vecRotation, TFLOAT a_fScale );

	// The quest of the item is done (or the story is past it)
	TBOOL IsUnlocked();

	// Number of the item that can still be bought, -1 if it's unlimited
	TINT GetNumRemaining();

	// All of the items have been bought
	TBOOL IsSoldOut();

	void IncreaseCount( TINT a_iUnused );

	// $Barnyard: FUNCTION 0049b000
	void DecreaseCount( TINT a_iUnused ) { m_iCount--; }

	// $Barnyard: FUNCTION 0049b010
	TBOOL HasCount() const { return m_iCount > 0; }

	void SetName( Toshi::TPString8 a_strModel, TINT a_iTextId );

	// $Barnyard: FUNCTION 0049b2f0
	void SetModel( const Toshi::TPString8& a_rcModel ) { m_strModel = a_rcModel; }

	// $Barnyard: FUNCTION 0049b320
	Toshi::TPString8 GetModel() const { return m_strModel; }

	// $Barnyard: FUNCTION 0049b420
	void SetInventoryType( Toshi::TPString8 a_strType ) { m_strInventoryType = a_strType; }

	// $Barnyard: FUNCTION 0049b4a0
	Toshi::TPString8 GetInventoryType() const { return m_strInventoryType; }

	// $Barnyard: FUNCTION 0049b4d0
	void SetName2( Toshi::TPString8 a_strName ) { m_strName2 = a_strName; }

	// $Barnyard: FUNCTION 0049b550
	Toshi::TPString8 GetName2() const { return m_strName2; }

	// $Barnyard: FUNCTION 0049b580
	void SetQuest( const Toshi::TPString8& a_rcQuest ) { m_strQuest = a_rcQuest; }

	// NOTE: retail uses a folded getter of another class (0049fa50)
	Toshi::TPString8 GetQuest() const { return m_strQuest; }

	// $Barnyard: FUNCTION 0049b5b0
	Toshi::TPString8 GetName3() const { return m_strName3; }

	// $Barnyard: FUNCTION 004932f0
	Toshi::TPString8 GetRequired( TINT a_iIndex ) const { return m_aRequired[ a_iIndex ]; }
	TINT             GetNumRequired() const { return m_iNumRequired; }

	AModelInstance* GetModelInstance();

	// The item as it's kept by AGopherShopGame
	AGopherShopPurchase GetPurchase() const;

	TINT               GetId() const { return m_iTextId; }
	TINT               GetCategory() const { return m_iCategory; }
	TINT               GetCount() const { return m_iCount; }
	void               SetCount( TINT a_iCount ) { m_iCount = a_iCount; }
	TINT               GetPrice() const { return m_iPrice; }
	TINT               GetMaxCount() const { return m_iMaxCount; }
	const TWCHAR*      GetNameText() const { return m_wszName; }
	AAnimatableObject* GetObject() const { return m_pObject; }

public:
	TFLOAT             m_fFade;                     // 0x00, how far the model has faded in/out while scrolling
	TFLOAT             m_fOffset;                   // 0x04, x position while scrolling
	Toshi::TVector4    m_vecRotation;               // 0x08, radians
	TINT               m_iCategory;                 // 0x18
	TFLOAT             m_fScale;                    // 0x1c
	const TWCHAR*      m_wszName;                   // 0x20
	TINT               m_iTextId;                   // 0x24
	AAnimatableObject* m_pObject;                   // 0x28
	Toshi::TPString8   m_strModel;                  // 0x2c
	TINT               m_iCount;                    // 0x30, bought (category 3: in the inventory, -1 none)
	TINT               m_iPrice;                    // 0x34
	TINT               m_iMaxCount;                 // 0x38, -1: unlimited
	Toshi::TPString8   m_strInventoryType;          // 0x3c
	Toshi::TPString8   m_strName2;                  // 0x40
	Toshi::TPString8   m_strQuest;                  // 0x44
	Toshi::TPString8   m_strName3;                  // 0x48
	TINT               m_iNumRequired;              // 0x4c
	Toshi::TPString8   m_aRequired[ MAX_REQUIRED ]; // 0x50
};

TSTATICASSERT( sizeof( AGopherShopItem ) == 0x7c );
TSTATICASSERT( sizeof( AGopherShopPurchase ) == 0x24 );
TSTATICASSERT( sizeof( AGopherShopPrice ) == 0x3c );
