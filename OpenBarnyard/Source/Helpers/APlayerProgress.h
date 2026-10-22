#pragma once
#include "GameInterface/AHackedQuickSave.h"

#include <Toshi/TSingleton.h>

// Story progress and the save data: a header and NUM_SAVE_SLOTS save slots, kept in one block of memory
// that SaveLoadSKU writes to / reads from the save file.
class MODAPI APlayerProgress
    : public Toshi::TSingleton<APlayerProgress>
{
public:
	static constexpr TINT NUM_SAVE_SLOTS = 4;

	// Version and date the header has to have to be loaded
	static constexpr TUINT16 SAVE_VERSION = 51;
	static constexpr TUINT32 SAVE_DATE    = 733546;

	// Header of the save data (0x44 bytes)
	struct Header
	{
		TCHAR   szMagic[ 2 ];     // 0x00, "BY"
		TUINT16 uiVersion;        // 0x02, SAVE_VERSION
		TUINT32 uiDate;           // 0x04, SAVE_DATE
		TUINT32 uiSize;           // 0x08, sizeof( SaveData )
		TUINT32 uiChecksum;       // 0x0c, CRC32 of the whole save (with this set to 0)
		TUINT32 uiUnk10;          // 0x10, m_uiUnk24
		TUINT32 uiUnlockedFlags;  // 0x14, m_eMiniGameFlags
		TUINT8  aOptions[ 0x2c ]; // 0x18, AOptions::SaveSettings
	};

	// The unlocked antics (a bit per antic, see IsMiniGameAvailable) and the art and movies of the
	// art gallery (a bit per item from UNLOCKFLAGS_ART0 and UNLOCKFLAGS_MOVIE0)
	using UNLOCKFLAGS = TUINT;
	enum UNLOCKFLAGS_ : UNLOCKFLAGS
	{
		UNLOCKFLAGS_NONE           = 0,
		UNLOCKFLAGS_UNK0           = BITFLAG( 0 ),  // an antic unlocked from the start
		UNLOCKFLAGS_UNK1           = BITFLAG( 1 ),  // an antic unlocked from the start
		UNLOCKFLAGS_UNK2           = BITFLAG( 2 ),  // an antic unlocked from the start
		UNLOCKFLAGS_UNK3           = BITFLAG( 3 ),  // an antic unlocked from the start
		UNLOCKFLAGS_MUD_JUMP       = BITFLAG( 4 ),  // unlocked by the quests or the cheat
		UNLOCKFLAGS_BARN_POOL      = BITFLAG( 5 ),  // unlocked by the quests or the cheat
		UNLOCKFLAGS_BIKE_RACE      = BITFLAG( 6 ),  // unlocked by the quests or the cheat
		UNLOCKFLAGS_CHASING_CHICKS = BITFLAG( 7 ),  // unlocked by the quests or the cheat
		UNLOCKFLAGS_VEGGIE_PATCH   = BITFLAG( 8 ),  // unlocked by the quests or the cheat
		UNLOCKFLAGS_COW_TIPPING    = BITFLAG( 9 ),  // unlocked by the quests or the cheat
		UNLOCKFLAGS_BARNTENDER     = BITFLAG( 10 ), // unlocked by the quests or the cheat
		UNLOCKFLAGS_UNK11          = BITFLAG( 11 ), // an antic unlocked from the start
		UNLOCKFLAGS_UNK12          = BITFLAG( 12 ), // an antic unlocked from the start
		UNLOCKFLAGS_ART0           = BITFLAG( 13 ), // the first art of the gallery, the ladder beans rewards unlock the next ones
		UNLOCKFLAGS_MOVIE0         = BITFLAG( 22 ), // the first movie of the gallery, the ladder beans rewards unlock the next ones
	};

	using SLOTFLAGS = TUINT8;
	enum SLOTFLAGS_ : SLOTFLAGS
	{
		SLOTFLAGS_NONE       = 0,
		SLOTFLAGS_HERO       = BITFLAG( 0 ), // EVENT_PLAYER_IS_HERO (ASpecialEventRequirement)
		SLOTFLAGS_CHAMPION   = BITFLAG( 1 ), // the player is the barn champion, set when SSEND_Champ plays (ABarnExploreMinigameState, 004a3780)
		SLOTFLAGS_BARNDETAIL = BITFLAG( 2 ), // AMiscFunctions::SaveFinishedBarnDetail
		SLOTFLAGS_NEWUNLOCK  = BITFLAG( 3 ), // set when one of the flags above is set the first time
#ifdef BARNYARD_LATEST
		// NOTE: the latest version (Wii) keeps the difficulty of the save here, set (Easy) or cleared (Normal) by
		// ADifficultyOptionState when a new game starts. Never written here: saves must stay compatible with PC
		// retail, so the difficulty is a setting of OpenBarnyard.cfg instead (see IsEasyDifficulty).
		SLOTFLAGS_EASY = BITFLAG( 4 ),
#endif
	};

	// One save slot (0xd170 bytes)
	struct SaveSlot
	{
		TUINT8    bOccupied;                                      // 0x0000
		TUINT8    aQuickSave[ AHackedQuickSave::SAVE_DATA_SIZE ]; // 0x0001, AHackedQuickSave::SaveToMemory
		TUINT8    aHints[ 0x334 ];                                // 0xb639, AHintManager::SaveState
		TUINT8    aHighScores[ 0x17e8 ];                          // 0xb96d, AHighScoreManager::Save
		SLOTFLAGS eFlags;                                         // 0xd155
		TUINT8    PADDING[ 2 ];                                   // 0xd156
		TINT      iGender;                                        // 0xd158
		TINT      iCostume;                                       // 0xd15c
		TINT      iAct;                                           // 0xd160
		TUINT     uiSimLoadingPictures;                           // 0xd164
		TUINT32   uiDate;                                         // 0xd168, day | month << 8 | year << 16
		TUINT32   uiTime;                                         // 0xd16c, second << 8 | minute << 16 | hour << 24
	};

	struct SaveData
	{
		Header   oHeader;                  // 0x00
		SaveSlot aSlots[ NUM_SAVE_SLOTS ]; // 0x44
	};

public:
	APlayerProgress( TINT a_iSizeAlignment, TINT a_iAlignment, TINT a_iHeaderOffset, TINT a_iExtraSize );
	~APlayerProgress();

	// Fills the header in (magic, version, date and size)
	void SetupHeader();

	// Clears every slot
	void ClearAllSlots();

	// Clears the current slot
	void ClearCurrentSlot();

	// Updates the checksum of the header
	TUINT32 UpdateChecksum();

	// Checks the header and the checksum, clears every slot and returns TFALSE if they are wrong
	TBOOL Validate();

	// Unlocks the minigames and bonus items of the quest tier a_iTier / a_iQuest
	void UnlockQuestTierFlags( TINT a_iTier, TINT a_iQuest );

	// Unlocks the bonus of the ladder beans, a_iNumGrown is the number of grown ladders.
	// Returns TTRUE if anything was unlocked.
	TBOOL UnlockLadderBeansFlags( TINT8 a_iNumGrown );

	// Sets SLOTFLAGS_CHAMPION (and SLOTFLAGS_NEWUNLOCK the first time) on the current slot
	SaveSlot* SetCurrentSlotChampion();

	// Is SLOTFLAGS_HERO set on the current slot
	TBOOL IsHero() const;

	// Sets SLOTFLAGS_HERO on the current slot (Cow Tipping)
	SaveSlot* SetCurrentSlotHero();

	// Is SLOTFLAGS_CHAMPION set on the current slot
	TBOOL IsChampion() const;

	// Sets SLOTFLAGS_BARNDETAIL on the current slot (and SLOTFLAGS_NEWUNLOCK the first time)
	SaveSlot* SetCurrentSlotBarnDetail();

	// Is SLOTFLAGS_BARNDETAIL set on the current slot
	TBOOL HasCurrentSlotBarnDetail() const;

	TBOOL IsMiniGameAvailable( TINT a_iIndex ) const { return ( m_eMiniGameFlags & ( 1 << a_iIndex ) ) != 0; }

	// Sets a_uiFlags in m_eMiniGameFlags (the unlocked antics and bonus items)
	void UnlockFlags( UNLOCKFLAGS a_uiFlags ) { m_eMiniGameFlags |= a_uiFlags; }

	UNLOCKFLAGS GetUnlockedFlags() const { return m_eMiniGameFlags; }
	void        SetUnlockedFlags( UNLOCKFLAGS a_uiFlags ) { m_eMiniGameFlags = a_uiFlags; }
	TUINT       GetUnk24() const { return m_uiUnk24; }
	void        SetUnk24( TUINT a_uiValue ) { m_uiUnk24 = a_uiValue; }

	TINT GetCurrentSlotIndex() const { return m_iCurrentSlot; }
	void SetCurrentSlotIndex( TINT a_iSlot ) { m_iCurrentSlot = a_iSlot; }

	SaveSlot* GetSlot( TINT a_iSlot ) const { return &m_pSaveData->aSlots[ a_iSlot ]; }
	// $Barnyard: FUNCTION 00423290
	SaveSlot* GetCurrentSlot() const { return m_iCurrentSlot == -1 ? TNULL : &m_pSaveData->aSlots[ m_iCurrentSlot ]; }

#ifdef BARNYARD_LATEST
	// TTRUE if the game is played on Easy (ADifficultyOptionState). Read by the minigames to tune themselves
	TBOOL IsEasyDifficulty() const;
#endif

	SaveData* GetSaveData() const
	{
		return m_pSaveData;
	}
	void* GetSaveBuffer() const { return m_pBuffer; }
	TUINT GetSaveSize() const { return m_uiSaveSize; }

private:
	void*       m_pUnk00;         // 0x00
	TINT        m_iUnk04;         // 0x04
	void*       m_pUnk08;         // 0x08
	TINT        m_iUnk0C;         // 0x0c
	TUINT       m_uiSaveSize;     // 0x10, what is written to the save file
	TUINT       m_uiBufferSize;   // 0x14
	TUINT8*     m_pBuffer;        // 0x18
	SaveData*   m_pSaveData;      // 0x1c
	TINT        m_iCurrentSlot;   // 0x20, -1 if none
	TUINT       m_uiUnk24;        // 0x24
	UNLOCKFLAGS m_eMiniGameFlags; // 0x28
};

TSINGLETON_DECLARE_ALIAS( APlayerProgress, PlayerProgress );

TSTATICASSERT( sizeof( APlayerProgress::Header ) == 0x44 );
TSTATICASSERT( sizeof( APlayerProgress::SaveSlot ) == 0xd170 );
TSTATICASSERT( sizeof( APlayerProgress::SaveData ) == 0x34604 );
