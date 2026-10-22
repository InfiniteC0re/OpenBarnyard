#pragma once
#include "GUI/AGUI2TextBox.h"
#include "FSM/AFunctionTable.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

class AModelInstance;
class AParticleEffectHandle;
class AUnit;

//-----------------------------------------------------------------------------
// Purpose: the hints of the sim. Pip's hint notes are scattered over the farm
// (the "hintNN" terrain locators) and the player finds them with the two
// floating "hint_note" models placed at the closest ones. It also places the
// sparkles that point at quest items (eggs, launcher parts, keys...) and the
// zone markers (save, sleep, doors), and shows the "out of milk" message.
// Created by AGameSystemManager, the sim (ASimState) activates it.
// TClass 00780b40 (size 0x334), vtable 00718ea8, singleton DAT_00780b00.
//-----------------------------------------------------------------------------
class MODAPI AHintManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AHintManager>
{
public:
	TDECLARE_CLASS( AHintManager, Toshi::TTask );

	static constexpr TINT MAX_HINTS   = 64;
	static constexpr TINT MAX_RECIPES = 40;

	// The markers the hint manager shows
	using MARKER = TINT;
	enum MARKER_ : MARKER
	{
		MARKER_NONE,
		MARKER_GOLDRUSH,             // "coinspawnerstump" units, while the player carries a goldrush_token
		MARKER_MILKWEED,             // "mweedNN" areas without milkweed, while the player carries a milkweed_seed
		MARKER_GATE,                 // the gate of the key the player carries
		MARKER_TOMATO_LAUNCHER_PART, // "Ben Tomato Launcher" quest
		MARKER_EGG,                  // "Hanna Missing Eggs" quest
		MARKER_JUNKYARD_PART,        // "Miles Junkyard Scores" quest
		MARKER_FRUIT,                // "Pig Apple Pie" quest
		MARKER_SAVE_ZONE,
		MARKER_SLEEP_ZONE,
		MARKER_BARN_ENTRANCE,
		MARKER_KITCHEN_ENTRANCE,
		MARKER_WALNUT_ENTRANCE,
		MARKER_MOCKTAIL_ENTRANCE,
		MARKER_BARN_EXIT,
		MARKER_GOPHER_ENTRANCE,
		MARKER_NUMOF
	};

	// Effects the markers are drawn with
	using EFFECT = TINT;
	enum EFFECT_ : EFFECT
	{
		EFFECT_SPARKLE_WHITE,
		EFFECT_SPARKLE_GREEN,
		EFFECT_SAVE_ZONE,
		EFFECT_SLEEP_ZONE,
		EFFECT_DOOR_ZONE,
		EFFECT_NUMOF
	};

	// Progress of the hints, saved with the game (0x00780b70)
	struct State
	{
		struct Hint
		{
			TINT   iNumLeft;   // 0x00, notes left at the locator
			TFLOAT fHideTime;  // 0x04, the note is hidden while > 0
			TBOOL  bAvailable; // 0x08
		};

		State();

		Hint  aHints[ MAX_HINTS ];      // 0x000
		TINT  iNumHints;                // 0x300, notes scattered over the farm
		TINT  iNumCollected;            // 0x304
		TINT  iNumRecipes;              // 0x308
		TBOOL abRecipes[ MAX_RECIPES ]; // 0x30c
	};

public:
	AHintManager();
	~AHintManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() override;                      // +0x18
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x1c
	virtual void  OnActivate() override;                    // +0x30
	virtual void  OnDeactivate() override;                  // +0x34

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable();                     // +0x38
	virtual void            Unload();                               // +0x3c
	virtual void            RenderOverlay( TFLOAT a_fDeltaTime ) {} // +0x40, DOES_NOTHING
	virtual void            Render( TFLOAT a_fDeltaTime );          // +0x44
	virtual void            PostRender( TFLOAT a_fDeltaTime ) {}    // +0x48, DOES_NOTHING

	// Loads the hint locators and the marker models (ASimState::OnInsertion)
	void Load();

	// Shows the "out of milk" message (AMilkSquirtProjectileLauncher)
	void ShowOutOfMilkMessage();

	// Returns TTRUE if a_pUnit is the player, facing a hint note within 3 metres
	TBOOL IsFacingHint( AUnit* a_pUnit );

	// Collects the hint the player is facing. Returns the index of the collected hint or -1
	TINT CollectHint();

	// Adds a recipe, returns its index or -1 if there's no more
	TINT AddRecipe();

	// Spreads a_iNumHints notes over the hint locators
	void SetNumHints( TINT a_iNumHints );

	static TBOOL __stdcall SaveState( void* a_pBuffer, void* a_pUnused );
	static TBOOL __stdcall LoadState( void* a_pBuffer, void* a_pUnused );
	static void ResetState();

	static TINT GetMaxHints() { return MAX_HINTS; }
	static TINT GetNumCollectedHints() { return ms_oState.iNumCollected; }
	// $Barnyard: FUNCTION 0041d260
	static TINT   GetNumRecipes() { return ms_oState.iNumRecipes; }
	static TBOOL* GetRecipes() { return ms_oState.abRecipes; }
	static TBOOL  HasFirstRecipe() { return ms_oState.abRecipes[ 0 ]; }


private:
	void UpdateHintNotes( TFLOAT a_fDeltaTime );
	void UpdateMarkers( TFLOAT a_fDeltaTime );
	void UpdateZoneMarker( MARKER a_eMarker );
	void BobMarker( MARKER a_eMarker, TFLOAT a_fHeight );
	void PlaceMarker( MARKER a_eMarker, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );
	void StopMarkerEffect( MARKER a_eMarker );
	void RenderMarkers( TBOOL a_bOverride );

	void UpdateGoldrushMarker();
	void UpdateMilkweedMarker();
	void UpdateGateMarker();
	void UpdateTomatoLauncherMarker();
	void UpdateEggMarker();
	void UpdateJunkyardMarker();
	void UpdateFruitMarker();

	static AHintManager* __stdcall GetInstance( AFunctionTable* a_pTable, void* a_pCaller );

public:
	static AFunctionTable ms_oFunctionTable; // 0x00780b10
	inline static State   ms_oState;         // 0x00780b70

private:
	AModelInstance*        m_pHintNote;                          // 0x20, "data\\models\\hint_note" at the closest hint
	AModelInstance*        m_pHintNote2;                         // 0x24, at the second closest one
	TINT                   m_iNumHintLocators;                   // 0x28
	TINT                   m_iFacingHint;                        // 0x2c, -1 if none
	TINT                   m_iMaxRecipes;                        // 0x30, cooking.NumberOfRecipes
	TFLOAT                 m_fBob;                               // 0x34, [0; 1], how high the markers are
	TBOOL                  m_bActive;                            // 0x38
	TBOOL                  m_bLoaded;                            // 0x39
	Toshi::TVector4        m_vPlayerPos;                         // 0x3c
	Toshi::TVector4*       m_pHintPositions;                     // 0x4c, [ MAX_HINTS ]
	Toshi::TVector4*       m_pHintDirections;                    // 0x50, [ MAX_HINTS ]
	AModelInstance*        m_apMarkerModels[ MARKER_NUMOF ];     // 0x54
	AParticleEffectHandle* m_apMarkerEffects[ MARKER_NUMOF ];    // 0x94
	Toshi::TVector4        m_avMarkerPositions[ MARKER_NUMOF ];  // 0xd4
	TINT                   m_aiMarkerEffectIds[ MARKER_NUMOF ];  // 0x1d4, effect playing at the marker
	TBOOL                  m_abShowMarker[ MARKER_NUMOF ];       // 0x214
	Toshi::TPString8       m_astrMarkerNames[ MARKER_NUMOF ];    // 0x224, item or quest of the marker
	TFLOAT                 m_afMarkerDistanceSq[ MARKER_NUMOF ]; // 0x264, from the player
	TINT                   m_aiEffects[ EFFECT_NUMOF ];          // 0x2a4
	AGUI2TextBox           m_oOutOfMilkText;                     // 0x2b8
	TFLOAT                 m_fOutOfMilkTime;                     // 0x330
};

TSTATICASSERT( sizeof( AHintManager::State ) == 0x334 );
TSTATICASSERT( sizeof( AHintManager ) == 0x334 );
