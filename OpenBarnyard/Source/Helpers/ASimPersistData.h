#pragma once
#include "GameInterface/AHackedQuickSave.h"
#include "HUD/AItemCountHudElement.h"

#include <Toshi/TSingleton.h>

// Sim state that has to survive leaving the sim (mini games, the barn, the frontend) and is saved
// with the profile (AHackedQuickSave::SaveProfile): story act, the ladder beans, the item count HUD...
class ASimPersistData
    : public Toshi::TObject
    , public Toshi::TSingleton<ASimPersistData>
{
public:
	TDECLARE_CLASS( ASimPersistData, Toshi::TObject );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE               = 0,
		FLAGS_MAINGAME_LOADED    = BITFLAG( 0 ), // set once the main game has been loaded (AGameLoader::LoadMainGameImpl)
		FLAGS_BARN_DAY_VISITED   = BITFLAG( 1 ), // the regrowth of the barn has grown on the first visit of the day (ABarnExploreMinigameState)
		FLAGS_BARN_NIGHT_VISITED = BITFLAG( 2 ), // the regrowth of the barn has grown on the first visit of the night
		FLAGS_CHAMP_SEQUENCE     = BITFLAG( 3 ), // SSEND_Champ has been played (ABarnExploreMinigameState::OnLoaded)
		FLAGS_REGROWTH_NEWDAY    = BITFLAG( 4 ), // set by ARegrowthManager::NewDay
		FLAGS_REGROWTH_NEWDAY2   = BITFLAG( 5 ), // set by ARegrowthManager::NewDay
	};

	// State of one of the ladder beans (ALadderBeansGame), 0x10 bytes
	struct LadderBeans
	{
		using LADDERFLAGS = TUINT8;
		enum LADDERFLAGS_ : LADDERFLAGS
		{
			LADDERFLAGS_NONE    = 0,
			LADDERFLAGS_GROWING = BITFLAG( 1 ), // the model is moving towards fTargetHeight (ALadderBeansGame::Update)
		};

		TINT8       iStage;        // 0x00, 0: not planted, 1-8: planted, 8: fully grown
		LADDERFLAGS uiFlags;       // 0x01
		TFLOAT      fHeight;       // 0x04, current Y offset of the milkweed model (the Y axis points down)
		TFLOAT      fTargetHeight; // 0x08, Y offset of the current stage
		TFLOAT      fMaxHeight;    // 0x0c, Y offset of the last stage
	};

	// Static data of the ladder beans (0x30 bytes per ladder, 0x00773f70)
	struct LadderBeansInfo
	{
		TINT   iId;                // 0x00, digit at the end of the locator name
		TFLOAT afStageHeight[ 9 ]; // 0x04
		TFLOAT fStartOffset;       // 0x28, horizontal offset of the model from the area towards its special point 1 at the first stage
		TFLOAT fEndOffset;         // 0x2c, same at the last stage
	};

	static constexpr TINT NUM_LADDER_BEANS = 8;

	// The 8 ladder beans (0x80 bytes)
	struct LadderBeansSet
	{
		LadderBeans aLadders[ NUM_LADDER_BEANS ];

		// Sets every ladder back to the first stage
		void Reset();

		TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer );
		TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer );
	};

	static const LadderBeansInfo ms_aLadderBeansInfo[ NUM_LADDER_BEANS ];

public:
	ASimPersistData();
	~ASimPersistData();

	virtual TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer ); // +0x0c
	virtual TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer ); // +0x10

	// Allocates the ladder beans (called after the manager is created)
	TBOOL CreateLadderBeans();
	void  DestroyLadderBeans();

	// Resets the data for a new game
	void Reset();

	TBOOL IsPipFollowingPlayer() const { return m_bPipFollowsPlayer; }
	void  SetPipFollowingPlayer( TBOOL a_bFollows ) { m_bPipFollowsPlayer = a_bFollows; }

	FLAGS GetFlags() const { return m_eFlags; }
	TBOOL HasFlags( FLAGS a_eFlags ) const { return ( m_eFlags & a_eFlags ) != 0; }
	void  SetFlags( FLAGS a_eFlags ) { m_eFlags |= a_eFlags; }
	void  ClearFlags( FLAGS a_eFlags ) { m_eFlags &= ~a_eFlags; }

	TINT GetTerrainVIS() const { return m_iTerrainVIS; }
	void SetTerrainVIS( TINT a_iVIS ) { m_iTerrainVIS = a_iVIS; }

	TINT GetAct() const { return m_iAct; }
	void SetAct( TINT a_iAct ) { m_iAct = a_iAct; }

	TINT GetRank() const { return m_iRank; }
	void SetRank( TINT a_iRank ) { m_iRank = a_iRank; }

	// The day (day + 30 * month) the farmer last left / came back (ASimState::UpdateDailyEvents)
	TINT GetFarmerDepartDay() const { return m_iFarmerDepartDay; }
	void SetFarmerDepartDay( TINT a_iDay ) { m_iFarmerDepartDay = a_iDay; }
	TINT GetFarmerArriveDay() const { return m_iFarmerArriveDay; }
	void SetFarmerArriveDay( TINT a_iDay ) { m_iFarmerArriveDay = a_iDay; }

	LadderBeansSet*              GetLadderBeans() const { return m_pLadderBeans; }
	AItemCountHudElement::State& GetItemCountHudState() { return m_oItemCountHudState; }

private:
	TINT                        m_iTerrainVIS;        // 0x04, VIS group of the terrain section the player was saved in (-1 if none)
	FLAGS                       m_eFlags;             // 0x08
	LadderBeansSet*             m_pLadderBeans;       // 0x0c
	AItemCountHudElement::State m_oItemCountHudState; // 0x10
	TBOOL                       m_bPipFollowsPlayer;  // 0x60
	TINT                        m_iAct;               // 0x64, act of the story
	TINT                        m_iFarmerDepartDay;   // 0x68, day + 30 * month the farmer last left
	TINT                        m_iFarmerArriveDay;   // 0x6c, day + 30 * month the farmer last came back
	TINT                        m_iRank;              // 0x70, 1 if the player is the champion (EVENT_PLAYER_IS_CHAMPION), -1 if none
};

TSTATICASSERT( sizeof( ASimPersistData::LadderBeans ) == 0x10 );
TSTATICASSERT( sizeof( ASimPersistData::LadderBeansSet ) == 0x80 );
TSTATICASSERT( sizeof( ASimPersistData::LadderBeansInfo ) == 0x30 );
