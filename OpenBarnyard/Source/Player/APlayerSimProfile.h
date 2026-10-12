#pragma once
#include "APlayerProfile.h"
#include "Helpers/AGameTimeManager.h"

#include <Toshi/TDList.h>
#include <Toshi/TString8.h>
#include <Toshi/TString16.h>
#include <Toshi/TPString8.h>

// Profile of the player in the sim (free roam) part of the game: gender, costume,
// stamina, the pick up counter shown on ASimPlayerPickUpHudElement and a few item lists.
class APlayerSimProfile : public APlayerProfile
{
public:
	// NOTE: retail registers the class with TObject as its parent, not APlayerProfile
	TDECLARE_CLASS( APlayerSimProfile, Toshi::TObject );

	// Entry of the m_llUsedItems / m_llPendingItems lists (12 of them, m_llFreeItems holds the unused ones)
	struct Item : public Toshi::TDList<Item>::TNode
	{
		// $Barnyard: FUNCTION 00621bf0
		Item()
		    : m_iText( 0 )
		    , m_iTextId( -1 )
		{}

		// $Barnyard: FUNCTION 00621350
		~Item() {}

		TINT             m_iText;   // 0x08
		TINT             m_iTextId; // 0x0c, -1 when unused
		Toshi::TPString8 m_strName; // 0x10
	};

	static constexpr TINT NUM_ITEMS          = 12;
	static constexpr TINT NUM_SQUIRTED_UNITS = 20;
	static constexpr TINT MAX_PICKUPS        = 9999;

public:
	APlayerSimProfile();
	~APlayerSimProfile();

	//-----------------------------------------------------------------------------
	// APlayerProfile
	//-----------------------------------------------------------------------------
	virtual void  Reset() OVERRIDE;                                     // +0x0c
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;               // +0x10
	virtual TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x14
	virtual TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x18

	// Adds a_fStamina (can be negative) to the stamina, clamped to [0, m_fMaxStamina]. Does nothing while m_bUpdateStamina is TFALSE.
	void AddStamina( TFLOAT a_fStamina );

	// Sets the stamina, clamped to [0, m_fMaxStamina].
	void SetStamina( TFLOAT a_fStamina );

	// Sets the max stamina, clamped to [1, 5], and lowers the stamina to it.
	void SetMaxStamina( TFLOAT a_fMaxStamina );

	// Adds a_fMaxStamina to the max stamina (see SetMaxStamina)
	void AddMaxStamina( TFLOAT a_fMaxStamina );

	// Lowers the max stamina by one every new day (called by APlayerProfileManager).
	TBOOL OnDateChanged( const AGameTimeManager::DateChangeEvent* a_pEvent );

	// Sets the pick ups (clamped to [0, MAX_PICKUPS]). With a_bUpdateHud and ASimPlayerPickUpHudElement around,
	// a_bCountUp lets the displayed value count up to it, otherwise the counter is set right away.
	void SetPickups( TINT a_iPickups, TBOOL a_bCountUp, TBOOL a_bUpdateHud );

	// Adds a_iPickups (can be negative) to the displayed pick ups, the counter counts up to it if ASimPlayerPickUpHudElement is around.
	void AddPickups( TINT a_iPickups );

	// AddPickups that adds to the value the counter is counting up to while it's counting.
	void AddPickupsToTarget( TINT a_iPickups );

	// Adds a_iPickups to m_iMiniGamePickups and shows it on ASimPlayerPickUpHudElement (collected mini game pick ups).
	void AddMiniGamePickups( TINT a_iPickups );

	// Returns the a_iIndex'th (1-based) item of m_llUsedItems or TNULL.
	Item* GetUsedItem( TINT a_iIndex );

	// Copies m_iText and the name of the a_iIndex'th (1-based) item of m_llUsedItems.
	void GetUsedItemInfo( TINT a_iIndex, TINT& a_rOutText, Toshi::TPString8& a_rOutName );

	// Moves every item back to m_llFreeItems.
	void ReleaseAllItems();

	// Moves the item named a_strName from both m_llUsedItems and m_llPendingItems back to m_llFreeItems.
	void ReleaseItem( Toshi::TPString8 a_strName );

	TINT GetNumUsedItems() { return m_llUsedItems.Count(); }

	// Takes a free item and appends it to m_llUsedItems (the quests the player can do), a_bShowHud flashes the sim HUD.
	void AddUsedItem( const TWCHAR* a_wszText, Toshi::TPString8 a_strName, TINT a_iTextId, TBOOL a_bShowHud );

	// Takes a free item and appends it to m_llPendingItems replacing the one called a_strName (used by AQuestManager).
	void AddPendingItem( const TWCHAR* a_wszText, Toshi::TPString8 a_strName, TINT a_iTextId );

	void  ClearSquirtedUnits();
	TBOOL HasSquirtedUnit( Toshi::TPString8 a_strName );
	TINT  GetNumSquirtedUnits();

	// Remembers a_strName (the name of a unit squirted by the player's milk, see ASquirtRequirement)
	// unless it's already there or it's one of the chicks ("chick_01" to "chick_07") or empty.
	void AddSquirtedUnit( Toshi::TPString8 a_strName );

	// Returns the name of the texture section of the player's hide ("Hide_Angus", "Hide_Girl_Angus", ...).
	// a_iCostume -1 and a_eGender GENDER_NONE use the profile's costume and gender.
	const TCHAR* GetHideTextureName( TINT a_iCostume, GENDER a_eGender ) const;

	TFLOAT GetStamina() const { return m_fStamina; }
	TFLOAT GetMaxStamina() const { return m_fMaxStamina; }
	TFLOAT GetStaminaRaiseSpeed() const { return m_fStaminaRaiseSpeed; }
	TFLOAT GetStaminaWasteSpeed() const { return m_fStaminaWasteSpeed; }
	void   SetStaminaWasteSpeed( TFLOAT a_fSpeed ) { m_fStaminaWasteSpeed = a_fSpeed; }
	TINT   GetCostume() const { return m_iCostume; }

	const Toshi::TString16& GetName() const { return m_strName; }
	void                    SetName( const TWCHAR* a_wszName ) { m_strName = a_wszName; }

	// $Barnyard: FUNCTION 00620eb0
	void SetName( const Toshi::TString16& a_rcName ) { m_strName.Copy( a_rcName ); }
	void SetCostume( TINT a_iCostume ) { m_iCostume = a_iCostume; }
	void SetEnableUpdates( TBOOL a_bEnable ) { m_bEnableUpdates = a_bEnable; }
	void SetUpdateStamina( TBOOL a_bUpdate ) { m_bUpdateStamina = a_bUpdate; }
	TINT GetMiniGamePickups() const { return m_iMiniGamePickups; }
	TINT GetSavedAct() const { return m_iSavedAct; }
	TINT GetSavedTier() const { return m_iSavedTier; }
	TINT GetDisplayedPickups() const { return m_iDisplayedPickups; }
	void SetMiniGamePickups( TINT a_iValue ) { m_iMiniGamePickups = a_iValue; }

	// Remembers the act and tier of AQuestManager
	void StoreQuestTier();

	TINT  GetNumHoneycombs() const { return m_iNumHoneycombs; }
	void  SetNumHoneycombs( TINT a_iNumHoneycombs ) { m_iNumHoneycombs = a_iNumHoneycombs; }
	TBOOL HasRoyalJelly() const { return m_bRoyalJelly; }
	void  SetRoyalJelly( TBOOL a_bRoyalJelly ) { m_bRoyalJelly = a_bRoyalJelly; }
	TINT  GetHoneyReplay( TINT a_iHive ) const { return m_aiHoneyReplay[ a_iHive ]; }
	void  SetHoneyReplay( TINT a_iHive, TINT a_iHoney ) { m_aiHoneyReplay[ a_iHive ] = a_iHoney; }

	const Toshi::TPString8& GetInventoryItem() const { return m_strInventoryItem; }
	void                    SetInventoryItem( const Toshi::TPString8& a_rcType ) { m_strInventoryItem = a_rcType; }

private:
	// Moves the item named a_strName from a_rList back to m_llFreeItems.
	void ReleaseItemFromList( Toshi::TDList<Item>& a_rList, Toshi::TPString8 a_strName );

private:
	TINT                m_iMiniGamePickups;                     // 0x10
	TINT                m_iUnk14;                               // 0x14
	TINT                m_iDisplayedPickups;                    // 0x18, value shown on ASimPlayerPickUpHudElement, counts towards m_iPickups
	TINT                m_iPickups;                             // 0x1c
	TFLOAT              m_fPickupsCountTime;                    // 0x20
	TFLOAT              m_fStamina;                             // 0x24
	TFLOAT              m_fMaxStamina;                          // 0x28
	TFLOAT              m_fStaminaRaiseSpeed;                   // 0x2c, per second
	TFLOAT              m_fStaminaWasteSpeed;                   // 0x30, per second while sprinting
	TBOOL               m_bUpdateStamina;                       // 0x34
	Toshi::TString16    m_strName;                              // 0x38, the name of the player
	TINT                m_iCostume;                             // 0x44
	Item                m_aItems[ NUM_ITEMS ];                  // 0x48
	Toshi::TDList<Item> m_llFreeItems;                          // 0x138
	Toshi::TDList<Item> m_llUsedItems;                          // 0x140
	Toshi::TDList<Item> m_llPendingItems;                       // 0x148
	Toshi::TPString8    m_aSquirtedUnits[ NUM_SQUIRTED_UNITS ]; // 0x150
	TFLOAT              m_fPendingItemsDelay;                   // 0x1a0
	TINT                m_iPickupsSound;                        // 0x1a4, cue of the looping counting sound
	TBOOL               m_bEnableUpdates;                       // 0x1a8
	TINT                m_iUnk1AC;                              // 0x1ac
	TINT                m_iNumHoneycombs;                       // 0x1b0, honeycombs kept while a save is loaded (AHackedQuickSave::LoadProfile)
	TBOOL               m_bRoyalJelly;                          // 0x1b4
	TCHAR               PADDING[ 0x1b8 - 0x1b5 ];               // 0x1b5
	TINT                m_aiHoneyReplay[ 3 ];                   // 0x1b8, honey of the hives restored into AReplayHoneyCollecting (AGameLoader::LoadMainGameImpl)
	TINT                m_iSavedAct;                            // 0x1c4, AQuestManager act (StoreQuestTier)
	TINT                m_iSavedTier;                           // 0x1c8, AQuestManager tier (StoreQuestTier)
	Toshi::TPString8    m_strInventoryItem;                     // 0x1cc, type of the item last selected in the "BackPack" of the inventory
};

TSTATICASSERT( sizeof( APlayerSimProfile::Item ) == 0x14 );
TSTATICASSERT( sizeof( APlayerSimProfile ) == 0x1d0 );
