#pragma once
#include "GameInterface/ASimState.h"
#include "Render/ASkinSwap.h"
#include "FSM/AFunctionTable.h"

#include <Toshi/T2Vector.h>
#include <Math/TMatrix44.h>

class AAnimatableObject;
class ACollisionObjectMesh;
class AInteriorGoal;
class AModelInstance;
class ABarnFanHelper;
class ABarnHelper;
class ABarnSeatedHelper;
class ABarnChatHelper;

//-----------------------------------------------------------------------------
// Purpose: the inside of the barn, by day (the empty barn, EnvBarn) and by
// night (the party, EnvBarnNight). The sim minus the farm: the barn items the
// player bought from the gopher shop are placed, the animals of the party are
// spawned at the "<type>_loc" bones of the barn models (barndata FSMMap and
// AreaMap say what they do), the music plays from the radio/wurlitzer, and the
// barn is split in three floors (the loft, the ground floor and the basement)
// of which only the one the player is on is animated and rendered.
// Started from the sim by the FSM ("StartGameState"), left with "FinishGameState".
// Size 0x153a8, vtable 0x0071f0d0.
//-----------------------------------------------------------------------------
class ABarnExploreMinigameState : public ASimState
{
public:
	TDECLARE_CLASS( ABarnExploreMinigameState, ASimState );

	// The floor of the barn the player is on (ms_iFloor)
	using FLOOR = TINT;
	enum FLOOR_ : FLOOR
	{
		FLOOR_LOFT     = 0, // above y = 3.246
		FLOOR_GROUND   = 1,
		FLOOR_BASEMENT = 2, // below y = -4
	};

	// Flags of an FSMMap entry
	using FSMFLAGS = TUINT32;
	enum FSMFLAGS_ : FSMFLAGS
	{
		FSMFLAGS_NONE         = 0,
		FSMFLAGS_DRINK        = BITFLAG( 0 ), // a milkshake in the hand (drink_loc for nb_otis, cb_llocator otherwise)
		FSMFLAGS_ALWAYS_SHOWN = BITFLAG( 1 ), // the unit isn't hidden with its floor (also a bigger shadow)
		FSMFLAGS_SHADOW       = BITFLAG( 2 ),
		FSMFLAGS_NOT_PARTY    = BITFLAG( 3 ), // not spawned on a party night
		FSMFLAGS_PARTY        = BITFLAG( 4 ), // only spawned on a party night (and not the band entries on a band night)
		FSMFLAGS_FIDDLE       = BITFLAG( 5 ),
		FSMFLAGS_DRUMS        = BITFLAG( 6 ),
		FSMFLAGS_BASS         = BITFLAG( 7 ),
		FSMFLAGS_SKINSWAP     = BITFLAG( 8 ), // gets a skin that isn't the one of the player
		FSMFLAGS_HENCARDS     = BITFLAG( 9 ),
	};

	// barndata "FSMMap": what the unit spawned at a bone does
	struct FSMMapEntry
	{
		const TCHAR* szParent;   // 0x00, type of the barn object the bone belongs to, TNULL for any
		const TCHAR* szBone;     // 0x04, the bone ("<type>_loc") or the unit type
		const TCHAR* szActivity; // 0x08, "seated", "pool", "band", ... or the FSM command
		FSMFLAGS     uiFlags;    // 0x0c
		TINT         iMinTier;   // 0x10, absolute quest tier
		TINT         iMaxTier;   // 0x14, 0 if there's no limit
	};

	// barndata "AreaMap": the units spawned at the bone are placed in the area
	struct AreaMapEntry
	{
		const TCHAR* szParent; // 0x00
		const TCHAR* szBone;   // 0x04
		const TCHAR* szArea;   // 0x08
	};

	//-----------------------------------------------------------------------------
	// An item of the barn: a model (or a numbered set of models) that is placed
	// when the item has been bought from the gopher shop (iItemId), or the one
	// (strSubModel) that stands there until it has been bought.
	//-----------------------------------------------------------------------------
	struct Item
	{
		static constexpr TINT MAX_NUM_OBJECTS = 15;

		Item();
		~Item();

		Item& operator=( const Item& a_rcOther );

		void SetAddModel( Toshi::TPString8 a_strModel, TINT a_iItemId );
		void SetSubModel( Toshi::TPString8 a_strModel );
		void SetName( Toshi::TPString8 a_strName );
		void SetReplaces( const Toshi::TPString8& a_rcName );

		Toshi::TPString8 GetName() const;
		Toshi::TPString8 GetReplaces() const;

		// $Barnyard: FUNCTION 004a3910
		TBOOL HasSubModel() const { return bHasSubModel; }

		// $Barnyard: FUNCTION 004a3930
		TINT GetNumObjects() const { return vecObjects.Size(); }

		// $Barnyard: FUNCTION 004a3940
		AAnimatableObject* GetObject( TINT a_iIndex ) { return vecObjects[ a_iIndex ]; }

		// Creates the objects of the item: the bought ones, then the sub models of the rest
		void CreateObjects();

		// Creates the animatable object a_strType (loading its type if needed) and its collision
		void CreateObject( Toshi::TPString8 a_strType, TBOOL a_bCollision );

		// Destroys the objects (and their types) and the collisions
		void DestroyObjects();

		// Destroys the objects of the sub model, if there are any
		void DestroySubObjects();

		Toshi::T2Vector<ACollisionObjectMesh*, MAX_NUM_OBJECTS> vecCollisions; // 0x00
		TINT                                                    iNumModels;    // 0x40, the numbered models ("bn_table_1_add".."bn_table_3_add")
		Toshi::TPString8                                        strName;       // 0x44, "in_*" or the shop model
		Toshi::TPString8                                        strAddModel;   // 0x48
		Toshi::TPString8                                        strSubModel;   // 0x4c
		TBOOL                                                   bHasSubModel;  // 0x50
		TINT                                                    iItemId;       // 0x54, the gopher shop item, -1 for the tables
		TINT                                                    iNumBought;    // 0x58
		Toshi::T2Vector<AAnimatableObject*, MAX_NUM_OBJECTS>    vecObjects;    // 0x5c
		Toshi::TPString8                                        strReplaces;   // 0x9c, the item (strName) this one replaces once bought
		TBOOL                                                   bEnabled;      // 0xa0
	};

	// Spots the cooked dishes of the cooking minigame are placed at
	struct FoodSpots
	{
		static constexpr TINT MAX_NUM_SPOTS = 17;

		struct Spot
		{
			Toshi::TVector4 vecPosition; // 0x00
			TBOOL           bFree;       // 0x10
			TINT            iUnk14;      // 0x14
		};

		FoodSpots();

		TINT iNumSpots;               // 0x00
		Spot aSpots[ MAX_NUM_SPOTS ]; // 0x04
	};

	// The snoring cows of the day barn ("cowzzzz")
	struct Snorer
	{
		Toshi::TVector4 vecPosition; // 0x00
		TFLOAT          fTimer;      // 0x10
	};

	static constexpr TINT MAX_NUM_ITEMS      = 100;
	static constexpr TINT MAX_NUM_SKIN_SWAPS = 5;
	static constexpr TINT MAX_NUM_SNORERS    = 5;
	static constexpr TINT MAX_NUM_COLLISIONS = 15;

	using UnitList   = Toshi::T2Vector<AUnit*, 100>;
	using ObjectList = Toshi::T2Vector<AAnimatableObject*, 200>;

public:
	ABarnExploreMinigameState();
	~ABarnExploreMinigameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE; // +0x0c
	virtual void     OnLoaded() OVERRIDE;                                                              // +0x24
	virtual TBOOL    ShouldPlacePlayerAtStartGoal() OVERRIDE;                                          // +0x28
	virtual ASoundId GetSound() OVERRIDE { return ASOUNDID_INVALID; }                                  // +0x2c, folded with AGameState::GetSound (004ee130)
	virtual TBOOL    CanDismissInstructions() OVERRIDE { return TTRUE; }                               // +0x30, RETURNS_TRUE 0046cde0
	virtual TBOOL    OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                         // +0x4c
	virtual void     OnInsertion() OVERRIDE;                                                           // +0x50
	virtual void     OnRemoval() OVERRIDE;                                                             // +0x54
	virtual void     OnActivate() OVERRIDE;                                                            // +0x60
	virtual void     OnDeactivate() OVERRIDE;                                                          // +0x64

	//-----------------------------------------------------------------------------
	// ASimState
	//-----------------------------------------------------------------------------
	virtual void UpdateSim( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x68

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x6c

	// FSM natives
	void             ToggleMusic( TBOOL a_bWurlitzer );
	TBOOL            IsMusicPlaying();
	void             DropLadder();
	Toshi::TPString8 GetBenchGoal();
	Toshi::TPString8 GetDanceGoal();
	void             IncDanceGoal();
	void             SSEND_Detail();
	void             SSEND_Champ();

	TBOOL IsNight() const { return m_bNight; }

	// The goal root walks to: the one of the ground floor (root_barn) when he's on it, otherwise root_nightbarn_pos
	AInteriorGoal* GetRootGoal() const { return m_bRootOnGround ? m_pRootBarnGoal : m_pRootNightBarnGoal; }

	// The VIS group (terrain section) of the floor the player is on (ATerrainInterface's current VIS group callback)
	static TINT GetCurrentVISGroup();

	static TBOOL IsMusicChannelOn( TINT a_iChannel ) { return a_iChannel == 0 ? ms_bRadioOn : ms_bWurlitzerOn; }

	// Sets the volume of the music channel a_iChannel: 0 the radio, 1 the wurlitzer (NB_Wurlitzer has two streams per channel)
	void SetMusicVolume( TINT a_iChannel, TFLOAT a_fVolume );

private:
	// Fades the music between the channels after a floor change
	void UpdateMusic( TFLOAT a_fDeltaTime );

	// Changes the floor when the player's height crosses one of the borders
	void UpdateFloor();
	void EnterLoft( const Toshi::TVector4& a_rcPosition );
	void EnterGround( const Toshi::TVector4& a_rcPosition );
	void EnterBasement( const Toshi::TVector4& a_rcPosition );

	// The items of the barn
	void        AddBaseItems();
	void        AddShopItems();
	void        UpdateItemCounts();
	void        DisableReplacedItems();
	void        UpdateVases();
	Item*       FindItem( TINT a_iItemId );
	Item*       FindItem( const Toshi::TPString8& a_rcName );
	static void SetupBooth( Item& a_rItem, const TCHAR* a_szName, TINT a_iItemId );

	// barndata
	const TCHAR*       FindArea( const Toshi::TPString8& a_rcParent, const TCHAR* a_szBone );
	const FSMMapEntry* FindFSMMapEntry( const Toshi::TPString8& a_rcParent, const TCHAR* a_szBone );

	// Spawns the units and objects at the "_loc" bones of the barn objects
	void SpawnAtBones();
	void SpawnAtBones( AAnimatableObject* a_pObject );

	// Resolves the "random", "nb_gen*" and "gen_*" names of the bones to a unit type
	static TBOOL ResolveUnitType( AAnimatableObject* a_pParent, const TCHAR* a_szName, TCHAR* a_szOutName, TSIZE a_uiSize );

	void SetupUnit( AUnit* a_pUnit, AAnimatableObject* a_pParent, const FSMMapEntry* a_pEntry, TBOOL a_bExisting );
	void SetupActivity( AUnit* a_pUnit, AAnimatableObject* a_pParent, const FSMMapEntry* a_pEntry );
	void SetupProps( AUnit* a_pUnit, FSMFLAGS a_uiFlags );
	void AddSkinSwap( AAnimatableObject* a_pObject, TINT* a_pSkin, TINT a_iNumSkins );

	// Creates an animatable object attached to the a_szBone bone of a_pParent, or placed at a_vecPosition (or a_pTransform)
	AAnimatableObject* CreateObject( AAnimatableObject* a_pParent, const TCHAR* a_szType, const TCHAR* a_szBone, TBOOL a_bAttach, Toshi::TVector4 a_vecPosition, TBOOL a_bAddToFloor, const Toshi::TMatrix44* a_pTransform );
	void               AddFloorObject( AAnimatableObject* a_pObject, const Toshi::TVector4& a_rcPosition );
	void               AddCollision( AAnimatableObject* a_pObject );

	// Night
	void SetupParty();
	void FindFoodSpots();
	void SpawnCookedFood();

	// Day
	void CreateLadder();
	void SpawnWildMikeBox();
	void SpawnQuadCow( TINT a_iIndex );

	// The end of the story (Barnyard Hero)
	void SetupEnding();
	void UpdateEndOtis();
	void RemoveEndDaisyObstruction();

	void CheckEndGame();

	static void DestroyUnits( UnitList& a_rUnits, TBOOL a_bDestroy );

public:
	// DAT_00773a64, the floor of the barn the player is on (FLOOR)
	// NOTE: AInstanceManager, ARegrowthManager and ABarnFanHelper render/animate the objects of this floor
	inline static TINT ms_iFloor = FLOOR_GROUND;

	inline static TBOOL ms_bWurlitzerOn = TTRUE; // DAT_00773968, FSM "IsMusicPlaying"
	inline static TBOOL ms_bRadioOn     = TTRUE; // DAT_00773969

	static AFunctionTable ms_oFunctionTable; // DAT_00782d7c

private:
	TBOOL                 m_bNight;                             // 0x10020
	TBOOL                 m_bParty;                             // 0x10021, the dance floor night (Friday or Saturday, every item bought)
	TBOOL                 m_bBandNight;                         // 0x10022, Saturday party: the band plays
	TBOOL                 m_bInChampSequence;                   // 0x10023, SSEND_Champ was pushed
	TBOOL                 m_bInDetailSequence;                  // 0x10024, SSEND_Detail was pushed
	TINT                  m_iNumItems;                          // 0x10028
	Item                  m_aItems[ MAX_NUM_ITEMS ];            // 0x1002c
	UnitList              m_vecAlwaysShownUnits;                // 0x1403c
	UnitList              m_vecLoftUnits;                       // 0x141d0
	UnitList              m_vecGroundUnits;                     // 0x14364
	UnitList              m_vecLoftCharacters;                  // 0x144f8, y > 0.1
	UnitList              m_vecGroundCharacters;                // 0x1468c
	ObjectList            m_vecLoftObjects;                     // 0x14820
	ObjectList            m_vecGroundObjects;                   // 0x14b44
	TINT                  m_iNumSkinSwaps;                      // 0x14e68
	ASkinSwap             m_aSkinSwaps[ MAX_NUM_SKIN_SWAPS ];   // 0x14e6c
	TBOOL                 m_bDanceFloor;                        // 0x14ed0, the dance floor (item 947 or 929) is there
	TINT                  m_iNumSnorers;                        // 0x14ed4
	Snorer                m_aSnorers[ MAX_NUM_SNORERS ];        // 0x14ed8
	FoodSpots             m_oFoodSpots;                         // 0x14f3c, "fd_" locators of the terrain
	FoodSpots             m_oBarFoodSpots;                      // 0x150d8, "ct_" bones of the saloon bar (cocktails)
	ABarnFanHelper*       m_pFanHelper;                         // 0x15274
	ABarnSeatedHelper*    m_pSeatedHelper;                      // 0x15278
	ABarnHelper*          m_pHelper;                            // 0x1527c
	ABarnChatHelper*      m_pChatHelper;                        // 0x15280
	TINT                  m_iNumFSMMapEntries;                  // 0x15284
	const FSMMapEntry*    m_pFSMMap;                            // 0x15288
	TINT                  m_iNumAreaMapEntries;                 // 0x1528c
	const AreaMapEntry*   m_pAreaMap;                           // 0x15290
	TINT                  m_iNumCollisions;                     // 0x15294
	ACollisionObjectMesh* m_apCollisions[ MAX_NUM_COLLISIONS ]; // 0x15298
	TFLOAT                m_fMusicFade;                         // 0x152d4
	TBOOL                 m_bWurlitzer;                         // 0x152d8, NB_Wurlitzer is playing (two channels), NB_Radio otherwise
	TINT                  m_iMusicChannel;                      // 0x152dc, the channel faded in (1 in the loft)
	TBOOL                 m_bHasVases;                          // 0x152e0, item 958
	TINT                  m_iNumBooths2;                        // 0x152e4, 4 if item 966 is bought
	TINT                  m_iNumBooths1;                        // 0x152e8, 4 if item 965 is bought
	TINT                  m_iLoftCue;                           // 0x152ec, 692
	TINT                  m_iGroundCue;                         // 0x152f0, 694
	TINT                  m_iBasementCue;                       // 0x152f4, 693
	TINT                  m_iSnoreEffect;                       // 0x152f8, "cowzzzz"
	TBOOL                 m_bDroppingLadder;                    // 0x152fc
	TFLOAT                m_fLadderDrop;                        // 0x15300
	TFLOAT                m_fLadderSpeed;                       // 0x15304
	Toshi::TMatrix44      m_oLadderTransform;                   // 0x15308
	AAnimatableObject*    m_pLadder;                            // 0x15348
	AUnit*                m_pRoot;                              // 0x1534c
	AInteriorGoal*        m_pRootBarnGoal;                      // 0x15350, "root_barn"
	AInteriorGoal*        m_pRootNightBarnGoal;                 // 0x15354, "root_nightbarn_pos"
	TBOOL                 m_bRootOnGround;                      // 0x15358
	TBOOL                 m_bPartySetUp;                        // 0x15359
	TFLOAT                m_fWildMikeHeight;                    // 0x1535c
	TBOOL                 m_bBarnyardHero;                      // 0x15360
	AUnit*                m_pEndOtis;                           // 0x15364
	Toshi::TVector4       m_vecEndOtisPosition;                 // 0x15368
	Toshi::TVector4       m_vecEndOtisDirection;                // 0x15378
	TINT                  m_iDanceGoal;                         // 0x15388
	AModelInstance*       m_pStageModel;                        // 0x1538c, "bn_stage_add" of SSEND_Champ
	TINT                  m_iFoosballCue;                       // 0x15390
	TINT                  m_iMechManCue;                        // 0x15394
	Toshi::TVector4       m_vecMechManPosition;                 // 0x15398
};

TSTATICASSERT( sizeof( ABarnExploreMinigameState::Item ) == 0xa4 );
TSTATICASSERT( sizeof( ABarnExploreMinigameState ) == 0x153a8 );
