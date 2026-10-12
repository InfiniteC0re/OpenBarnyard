#pragma once

// Loads the main game terrain (EnvMain) unless it's already loaded with the section a_iStartVISGroup (AGameLoader.cpp, used by ABikeRaceMicroGame)
TBOOL Terrain_SetMain( TBOOL a_bStreamCollision, TINT a_iStartVISGroup, TBOOL a_bLoadLater );

//-----------------------------------------------------------------------------
// Purpose: manages the whole game and it's state
//-----------------------------------------------------------------------------
class AFunctionTable;

class AGameLoader
{
public:
	// NOTE: AGameLoader has a TClass in the original (child of TObject, never instantiated, no GetClass)
	// that only owns the natives table (DAT_0078110c), which is registered without any functions.
	static Toshi::TClass  ms_oClass;         // 0x00781140
	static AFunctionTable ms_oFunctionTable; // 0x0078110c

	static void InitialiseClass();
	static void DeinitialiseClass();

	using Terrain = TUINT32;
	enum Terrain_ : Terrain
	{
		Terrain_EnvMain,
		Terrain_GateCrash,
		Terrain_EnvBarnInt,
		Terrain_Golf1,
		Terrain_Golf2,
		Terrain_Golf3,
		Terrain_Golf4,
		Terrain_Golf5,
		Terrain_Golf6,
		Terrain_Golf7,
		Terrain_Golf8,
		Terrain_Golf9,
		Terrain_Golf10,
		Terrain_Golf11,
		Terrain_Golf12,
		Terrain_Golf13,
		Terrain_Golf14,
		Terrain_Golf15,
		Terrain_Golf16,
		Terrain_Golf17,
		Terrain_Golf18,
		Terrain_PeckRange,
		Terrain_FrontEnd,
		Terrain_Statue,
		Terrain_EnvBeadyInt,
		Terrain_EnvWoods,
		Terrain_EnvBarnIntDay,
		Terrain_EnvBlackMarket,
		Terrain_CowTipping,
		Terrain_VeggieDef,
		Terrain_EnvPoolTable,
		Terrain_EnvPoolBoat,
		Terrain_EnvWacARac,
		Terrain_Tappers,
		Terrain_EnvCarRace,
		Terrain_Kitchen,
		Terrain_Cocktails,
		Terrain_ChasingChicks2,
		Terrain_EnvBensHill,
		Terrain_EnvBeadyFarm,
		Terrain_EnvCoopInt,
		Terrain_EnvFarmerInt,
		Terrain_EnvDartsLarge,
		Terrain_EnvSSDarts,
		Terrain_test1,
		Terrain_ss01,
		Terrain_ss02,
		Terrain_ss03,
		Terrain_ss04,
		Terrain_ss05,
		Terrain_ss06,
		Terrain_ss07a,
		Terrain_ss07b,
		Terrain_ss08,
		Terrain_ss09,
		Terrain_ss10,
		Terrain_ss11,
#ifdef BARNYARD_LATEST
		Terrain_ss06merged, // Wii: SS06 with the SSCUO shots on the hilltop (SS06_storyMerged)
#endif
		Terrain_NUMOF
	};

	struct TerrainInfo
	{
		const TCHAR* szName;
		TUINT32      uiUnk;
	};

	inline static constexpr TerrainInfo ms_aTerrains[ Terrain_NUMOF ] = {
		{ "EnvMain", 0x400000 },
		{ "gatecrash", 0x500000 },
		{ "envbarnint", 0x400000 },
		{ "golf1", 0x600000 },
		{ "golf2", 0x600000 },
		{ "golf3", 0x600000 },
		{ "golf4", 0x600000 },
		{ "golf5", 0x600000 },
		{ "golf6", 0x600000 },
		{ "golf7", 0x600000 },
		{ "golf8", 0x600000 },
		{ "golf9", 0x600000 },
		{ "golf10", 0x600000 },
		{ "golf11", 0x600000 },
		{ "golf12", 0x600000 },
		{ "golf13", 0x600000 },
		{ "golf14", 0x600000 },
		{ "golf15", 0x600000 },
		{ "golf16", 0x600000 },
		{ "golf17", 0x600000 },
		{ "golf18", 0x600000 },
		{ "peckrange", 0x900000 },
		{ "frontend", 0x300000 },
		{ "statue", 0x400000 },
		{ "env_beadyint", 0x400000 },
		{ "envwoods", 0x800000 },
		{ "envbarnintday", 0x400000 },
		{ "envblackmarket", 0x100000 },
		{ "cowtipping", 0x600000 },
		{ "veggiedef", 0x600000 },
		{ "EnvPoolTable", 0x400000 },
		{ "envpoolboat", 0x400000 },
		{ "EnvWacARac", 0x100000 },
		{ "tappers", 0x500000 },
		{ "envcarrace", 0xB00000 },
		{ "kitchen", 0x400000 },
		{ "cocktail", 0x400000 },
		{ "chasingchicks2", 0x800000 },
		{ "envbenshill", 0x800000 },
		{ "envbeadyfarm", 0x800000 },
		{ "envcoopint", 0x400000 },
		{ "envfarmerint", 0x400000 },
		{ "envdarts_large", 0x400000 },
		{ "EnvSSDarts", 0x400000 },
		{ "test1", 0x400000 },
		{ "ss01", 0x400000 },
		{ "ss02", 0x400000 },
		{ "ss03", 0x400000 },
		{ "ss04", 0x400000 },
		{ "ss05", 0x400000 },
		{ "ss06", 0x400000 },
		{ "ss07a", 0x400000 },
		{ "ss07b", 0x400000 },
		{ "ss08", 0x400000 },
		{ "ss09", 0x400000 },
		{ "ss10", 0x400000 },
		{ "ss11", 0x400000 },
#ifdef BARNYARD_LATEST
		{ "ss06merged", 0x800000 },
#endif
	};

public:
	// Returns TTRUE if tasks are completed
	static TBOOL UpdateTransitionTask();

	//-----------------------------------------------------------------------------
	// GameState
	//-----------------------------------------------------------------------------
	static void SetTransitionToMiniGame( TINT a_iMiniGame, TBOOL a_bStartLoading );

	// SetTransitionToMiniGame with the minigame called a_strName
	static void SetTransitionToMiniGame( Toshi::TPString8 a_strName, TBOOL a_bStartLoading );

	// Starts loading the main game (lib_maingame, EnvMain, the sim player and ASimState), see LoadMainGameImpl.
	// a_bLoadedGame is TTRUE when coming from a loaded save (or the character select), TFALSE for a new game.
	static void SetTransitionToMainGame( TBOOL a_bLoadedGame );

	// Restarts the current minigame. If a_bImmediately is TFALSE, only asks AGameStateController to do it after fading out
	static void RestartMiniGame( TBOOL a_bImmediately );

	// Leaves the current minigame: back to the story (story mode) or to the frontend.
	// If a_bImmediately is TFALSE, only asks AGameStateController to do it after fading out
	static void LeaveMiniGame( TBOOL a_bImmediately );

	// Loads the asset library named a_rcName and the game state class of the same name (now or once the screen has faded out)
	static void SetTransitionToGameState( const Toshi::TPString8& a_rcName, TBOOL a_bStartLoading );

	// Spawns the player of the sim (unit type "player", named "simplayer")
	static void SpawnSimPlayer();

	// Unloads the current game state/minigame and loads the frontend, then the state called a_szStateName (if not TNULL).
	// If a_bImmediately is TFALSE, only asks AGameStateController to do it after fading out
	static void ReturnToFrontEnd( const TCHAR* a_szStateName, TBOOL a_bResetPlayerSet, TBOOL a_bImmediately );

	// Loads the current save slot of APlayerProgress and starts the main game from it.
	// Without a_bNow it's done once the screen has faded out (AGameStateController::UpdateTransition).
	static void LoadSavedGame( TBOOL a_bNow );

	// Fills the 4 ints a save slot keeps about the player (gender, costume, act, loading pictures)
	static TBOOL SaveProgressInfo( TINT* a_pOutInfo );

	// Starts loading the scripted sequence a_iSequence (index of s_aScriptedSequences), see LoadScriptedSequenceImpl.
	// The state to go back to afterwards is the current one (unless it's AFrontEndDebugSSeqState).
	static void SetTransitionToScriptedSequence( TINT a_iSequence );

	// Sets the state to go back to once the scripted sequence is over (DAT_007810ec, inlined in retail)
	static void SetPrevGameStateClass( const Toshi::TClass* a_pClass );

	// Starts a new game: the SS01 intro sequence, then the main game (AGameStateController, FLAGS_UNK5)
	static void StartNewGame();

	// Leaves the scripted sequence: back to the state it was started from, to the main game or to the frontend.
	// If a_bImmediately is TFALSE, only asks AGameStateController to do it after fading out
	static void LeaveScriptedSequence( TBOOL a_bImmediately );

	// Resets the game and unloads the current level and assets before the application quits
	static void UnloadForExit();

	// Enables only the first sim loading screen picture (a new game)
	static void ResetSimLoadingPictures();

	// Localised name of the sim loading screen picture a_iPicture
	static Toshi::TPString8& GetSimLoadingPicture( Toshi::TPString8& a_rOutName, TINT a_iPicture );

	// NOTE: inlined in retail (ABarnRewardState), the picture can then be shown by the sim loading screen
	static TBOOL IsSimLoadingPictureUnlocked( TINT a_iPicture );
	static void  UnlockSimLoadingPicture( TINT a_iPicture );

	// Unlocks the sim loading screen picture called a_rcName, returns TFALSE if there's no such picture
	static TBOOL UnlockSimLoadingPicture( const Toshi::TPString8& a_rcName );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-minigame <GameStateName>" skips the startup screens and starts the minigame once the frontend is up
	static TBOOL Dev_IsMiniGameRequested();
	static void  Dev_UpdateMiniGameRequest();

	// Development: "-simstate" skips the startup screens and loads the main game (ASimState) once the frontend is up
	static TBOOL Dev_IsSimStateRequested();
	static void  Dev_UpdateSimStateRequest();

	// Development: plays the scripted sequence "-devtier" asked for while the main game was loading
	static void Dev_UpdateDevTierSequence();

	// Development: with "-devsavefile <path>" every save / load uses that file from the start (the save loaded at
	// boot and OpenBarnyard.cfg included), so a test run never reads or writes Save\savedata.by. TTRUE if it's given
	static TBOOL Dev_SetupSaveFileAtBoot();
#endif
	// NOTE: not in PC retail, set by AFrontEndPlayersState (recovered from the original source)
	// Set while a multiplayer Antics session is going on
	inline static TBOOL s_bAntics = TFALSE;

	static TINT GetCurrMiniGame();
	static TINT GetNextMiniGame();

	static void LoadFrontEnd();

	// Destroys the current terrain (and the golf flags and bike race gates of the previous level)
	static void DestroyLevel();

	// Destroys the level, the world managers (trees, instances...) and unloads every model and library (engine shutdown)
	static void DestroyAll();

	// Remembers where the bike the player left in the sim is (when the sim is left)
	static void StoreSimBike();

	// Puts the bike back where the player left it (when the sim is entered again)
	static void RestoreSimBike();

	//-----------------------------------------------------------------------------
	// Terrain
	//-----------------------------------------------------------------------------
	static void Terrain_Set( TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bStreamModels, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup );
	static void Terrain_Load();

	static TBOOL IsChangingTerrain();
	static TBOOL IsBarnExploreMiniGame();

	// Set by ABarnExploreMinigameState while it exists (DAT_007810e6)
	static void SetBarnExploreMiniGame( TBOOL a_bBarnExploreMiniGame );

	// The name of the class of the state the minigame or the game state was started from
	static Toshi::TPString8 GetPrevGameStateName();

	// TTRUE while the sim player has to be placed at the start goal once the game state is loaded
	static TBOOL ShouldPlacePlayerAtStartGoal();

	// The goal the sim player is placed at once the next game state (the barn, the sim) is loaded (ASpawnActivity)
	static void  SetPlayerStartGoal( const Toshi::TPString8& a_rcGoal );
	static TBOOL IsStoryMode();

	// NOTE: inlined in retail (e.g. ACowTippingMiniGameState::OnInsertion)
	static void  SetStoryMode( TBOOL a_bStoryMode );
	static TBOOL IsMainGameLoaded();

	// TTRUE while the library of a game state (minigame, scripted sequence...) is loaded (DAT_007810e3)
	static TBOOL IsGameStateLibraryLoaded();

	// TTRUE until ReturnToFrontEndImpl has run (DAT_00781105)
	static TBOOL IsReturningToFrontEnd();

	// TTRUE once a minigame state has been pushed by LoadMiniGameImpl, until the minigame is left
	static TBOOL IsInMiniGame();

	// TTRUE if the state loaded before the current minigame was ABarnExploreMinigameState
	static TBOOL WasBarnExploreMiniGame();
	static void  ResetWasBarnExploreMiniGame();

	// The last scripted sequence that was started (-1 once ASimState has handled it)
	static TINT GetLastScriptedSequence();
	static void ResetLastScriptedSequence();

	// TTRUE while the main game is being loaded from a save
	static TBOOL IsLoadingSavedGame();

	// Index of the scripted sequence (ScriptedSequence/SSxx) that is loaded, -1 if there's none
	static TINT GetScriptedSequence();
	static TINT GetTerrainVIS();

	static constexpr const TCHAR* GetTerrainName( Terrain a_eTerrain ) { return ms_aTerrains[ a_eTerrain ].szName; }

	//-----------------------------------------------------------------------------
	// Utils
	//-----------------------------------------------------------------------------
	static void Debug_VerifyModelsInAssetPack( Toshi::TTRB* a_pTRB );
};
