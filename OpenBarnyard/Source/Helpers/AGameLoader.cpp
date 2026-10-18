#include "pch.h"
#include "AGameLoader.h"
#include "AOptions.h"
#include "FSM/AFunctionTable.h"
#include "ALoadScreen.h"
#include "Tasks/ARootTask.h"
#include "Tasks/AGameSystemManager.h"
#include "Helpers/AHighScoreManager.h"
#include "MiniGame/AGopherShopInterface.h"
#include "MiniGame/AGopherShopGame.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/ABikeHelpers.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Cameras/ABoneAttachCameraHelper.h"
#include "Player/APlayerManager.h"
#include "Units/AInventoryGroupComponent.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "Helpers/AInstanceManager.h"
#include "Units/AScriptedSequenceActivity.h"
#include "Helpers/AGameTimeManager.h"
#include "Physics/ACollisionManager.h"
#include "HUD/AHudManager.h"
#include "FX/AAniEffectManager.h"
#include "GUI/AGUISlideshow.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "MiniGame/AMiniGameManager.h"
#include "MiniGame/AShakeTreeMicroGame.h"
#include "MiniGame/ABikeRaceMicroGame.h"
#include "Render/AShadowManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "GameInterface/AFrontEndMainMenuState2.h"
#include "GameInterface/AFrontEndAttractState.h"
#include "GameInterface/AMiniGameInstructionState.h"
#include "GameInterface/AMessagePopupState.h"
#include "Player/APlayerProfileManager.h"
#include "Input/AInputHandler.h"
#include "Input/AInputMapManager.h"
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceMouse.h>
#include "GUI/AGUI2.h"

#ifdef BARNYARD_LATEST
#  include "GUI/AGUISystem.h"
#  include "GUI/XUI/AGUI2XUIText.h"
#endif
#include "Units/AAnimalPopulationManager.h"
#include "Units/AInteractionManager.h"
#include "Render/ARainRenderer.h"
#include "FX/AGameTimeFXManager.h"
#include "Helpers/AWeatherManager.h"
#include "GameInterface/ASimState.h"
#include "Helpers/ALadderBeansGame.h"
#include "Units/AUnitPlayer.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ALaunchAmmo.h"
#include "Helpers/APlayerProgress.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "GameInterface/AInventoryState2.h"
#include "GameInterface/AInteriorNetworkPlacementState.h"
#include "Cameras/ACamera.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitNameManager.h"
#include "Units/AUnitTransform.h"
#include "Units/AAnimatedUnit.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/APlayerControlComponent.h"
#include "Units/ACharacterUnit.h"
#include "GameInterface/AAnimalConversationState.h"
#include <Toshi/TScheduler.h>
#include "Assets/AModelLoader.h"
#include "Animation/AAnimatableObject.h"
#include "ARandom.h"
#include "Units/AUnitType.h"
#include "Quests/AQuestManager.h"
#include "Helpers/AGateManager.h"
#include "Helpers/ATreeManager.h"
#include "Animation/AModelRepos.h"
#include "Helpers/ARegrowthManager.h"
#include "Helpers/ATreasureManager.h"
#include "Helpers/ASimPersistData.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/AHintManager.h"
#include "GameInterface/SaveLoadSKU.h"
#include "GameInterface/AHackedQuickSave.h"
#include "GameInterface/AGameStateController.h"
#include "Units/ABikeControlComponent.h"
#include "Helpers/ACoinSpawner.h"
#include "MiniGame/ACoinPickupMicroGame.h"
#include "Units/ACoinPickupCoinItem.h"
#include "Units/AInventoryComponentInterface.h"
#include "MiniGame/ACookingRecipe.h"
#include "Assets/AAssetLoader.h"
#include "Helpers/AReplayManager.h"
#include "GameInterface/AReplayVariantSelectState.h"
#include "MiniGame/ATreasueHuntMicroGame.h"
#include "GameInterface/ASSeqState.h"
#include "Render/ARenderer.h"
#include "MiniGame/ABarnExploreMinigameState.h"
#include "Sound/AMultiplexStream.h"
#include "Quests/AQuest.h"
#include "Units/AInteractionEventManager.h"
#include "Units/AInteractionQuestEvent.h"
#include "Units/AUnitRef.h"
#include "FSM/AMiscFunctions.h"
#include "ACheatActivator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( honey_collecting_replay );

//-----------------------------------------------------------------------------
// GameState Management
//-----------------------------------------------------------------------------
static TINT          s_iCurrMiniGame       = -1; // 0x00772804
static TINT          s_iNextMiniGame       = -1;
static TBOOL         s_bLoadingMiniGame    = TFALSE; // 0x00781102
static const TClass* s_pPrevGameStateClass = TNULL;

// Set while the story (sim world) is being played, minigames started from it return to it.
static TBOOL s_bStoryMode = TFALSE; // 0x007810e1

// Set once a minigame state has been pushed by LoadMiniGameImpl.
static TBOOL s_bInMiniGame = TFALSE; // 0x007810e2

// Set while the asset library named after s_strCurrentStateName is loaded (see LoadGameStateImpl).
static TBOOL s_bGameStateLibraryLoaded = TFALSE; // 0x007810e3

// Set while lib_maingame is loaded.
static TBOOL s_bMainGameLoaded = TFALSE; // 0x007810e4

// Set by LoadMiniGameImpl if the state that was loaded before the minigame is ABarnExploreMinigameState.
static TBOOL s_bWasBarnExploreMiniGame = TFALSE; // 0x007810e7

// Index of the scripted sequence (ScriptedSequence/SSxx) that is loaded, -1 if there's none.
static TINT s_iScriptedSequence = -1; // 0x0077280c

// Name of the game state class loaded by LoadGameStateImpl, set by SetTransitionToGameState.
static TPString8 s_strCurrentStateName; // 0x0078113c

//-----------------------------------------------------------------------------
// Terrain Management
//-----------------------------------------------------------------------------
static TINT                 s_iTerrainVISIndex = -1;
static TBOOL                s_bChangingTerrain = TFALSE; // 0x00781103
static ATerrainInterface*   s_pCurrentTerrain  = TNULL;
static AGameLoader::Terrain s_eCurrentLevel    = AGameLoader::Terrain_EnvBeadyFarm;

static TBOOL s_bLoadingGameState    = TFALSE; // 0x00781104, set when LoadGameStateImpl (00426b70) has to run
static TBOOL s_bReturningToFrontEnd = TFALSE; // 0x00781105, set when ReturnToFrontEndImpl (004271f0) has to run

// Set when the sim player has to be placed at s_strPlayerStartGoal (see LoadMainGameImpl and SetPlayerStartGoal), cleared by ReturnToFrontEnd
// NOTE: 0x00781101 next to it is s_bSimBikeStored
static TBOOL s_bPlacePlayerAtStartGoal = TFALSE; // 0x00781100

static TBOOL s_bLastTransitionUpdateResult; // 0x00781724

// Set while the barn explore minigame is loaded; the player gets no launchers then.
// Set by LoadGameStateImpl when the state is "ABarnExploreMinigameState", and by its constructor and OnRemoval
static TBOOL s_bBarnExploreMiniGame = TFALSE; // 0x007810e6

// Set while the main game is loaded from a save (LoadSavedGame), cleared once LoadMainGameImpl / LoadGameStateImpl have run.
static TBOOL s_bLoadingSavedGame = TFALSE; // 0x00781106

// Goal the sim player is placed at once the main game is loaded (SetPlayerStartGoal sets it), used while s_bPlacePlayerAtStartGoal is set.
static TPString8 s_strPlayerStartGoal; // 0x00781710

#ifdef BARNYARD_LATEST
// Latest version: see IsPigApplePieReturn
static TBOOL s_bPigApplePieReturn = TFALSE; // 0x806b4aa1 (Wii)
#endif

// NOTE: CreateObject 006dc070, CreateObjectInPlace 0054d8b0 (RETURNS_ZERO)
Toshi::TClass AGameLoader::ms_oClass( "AGameLoader", &TGetClass( TObject ), TNULL, TNULL, AGameLoader::InitialiseClass, AGameLoader::DeinitialiseClass, 0, 1, 4, 1 );

// DAT_0078110c
AFunctionTable AGameLoader::ms_oFunctionTable( &AGameLoader::ms_oClass, TNULL );

// $Barnyard: FUNCTION 004232b0
void AGameLoader::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );
}

// $Barnyard: FUNCTION 00423d50
void AGameLoader::DeinitialiseClass()
{
	s_strCurrentStateName = TPString8();
	s_strPlayerStartGoal  = TPString8();
	ms_oFunctionTable.Unregister();
}

// VIS group the main game terrain starts streaming from (-1 for the saved one).
// NOTE: nothing in retail writes it, so it's always -1
static TINT s_iMainGameStartVIS = -1; // 0x00772818

// A bit for every sim loading screen picture (s_aSimLoadingPictures) that can be shown.
// Unlocked by ABarnRewardState and the FSM native EnableInstructionScreen (UnlockSimLoadingPicture)
static TUINT s_uiSimLoadingPictures = 0; // 0x007817ec

#ifdef BARNYARD_LATEST

// Wii: loads the XUI scene a_rcName (Data/XUI/<language>/<name>.trb, its material library and the
// button icons of its texts) and shows it as the loading screen, in front of AGUISystem's background
// picture. a_bLoadResource is TFALSE when the resource is already loaded (RestartMiniGame).
// NOTE: inlined into the loaders of the Wii build (LoadMiniGameImpl 8012a518, LoadGameStateImpl 8012b160,
// LoadMainGameImpl 80129a2c and RestartMiniGame 80128590)
static TBOOL ShowXUILoadScreen( const TPString8& a_rcName, TBOOL a_bLoadResource )
{
	AGUI2* pGUI = AGUI2::GetSingleton();

#  ifdef BARNYARD_COMMUNITY_PATCH
	// The Wii data is optional on PC: without the scene, the loader shows the PC picture
	if ( a_bLoadResource )
	{
		TString8 strPath;
		strPath.Format( "Data/XUI/%s/%s.trb", AGUI2::GetLanguage(), a_rcName.GetString() );

		TFile* pFile = TFile::Create( strPath, TFILEMODE_READ );

		if ( !pFile )
			return TFALSE;

		pFile->Destroy();
	}
	else if ( !pGUI->GetXUI().FindResource( a_rcName.GetString() ) )
	{
		return TFALSE;
	}
#  endif

	if ( a_bLoadResource )
	{
		pGUI->LoadXUIFont( "HGPSoeiKakupoptai_24" );
		pGUI->LoadXUIFont( "HGPSoeiKakupoptai_48" );
		pGUI->LoadXUIMaterialLibrary( "wii_icons128" );

		// NOTE: the Wii build creates it every time (the material of the previous load screen is destroyed
		// with the asset pack that is unloaded before, see AAssetLoader::DestroyAssetPackOfLibrary)
		if ( !AGUI2XUIText::ms_pIconMaterial )
			AGUI2XUIText::ms_pIconMaterial = T2GUI::GetRenderer()->CreateMaterial( "..\\..\\..\\Texture\\wii_icons128.tga" );

		pGUI->LoadXUIResource( a_rcName.GetString() );
		pGUI->LoadXUIMaterialLibrary( a_rcName.GetString() );
	}

	g_oLoadScreen.ShowXUIBackground( TTRUE, TFALSE );
	g_oLoadScreen.SetXUIScene( a_rcName.GetString(), 0 );
	ARenderer::GetSingleton()->ForceUpdate30FPS();

	return TTRUE;
}

#endif // BARNYARD_LATEST

//-----------------------------------------------------------------------------
// Scripted sequences
//-----------------------------------------------------------------------------

// The scripted sequences (0x007791f0), indexed by s_iScriptedSequence. The table is shared with AScriptedSequenceActivity
static const AScriptedSequenceActivity::SequenceInfo* const s_aScriptedSequences = AScriptedSequenceActivity::ms_aSequences;

// Destroys the asset pack of the scripted sequence that is loaded
static void UnloadScriptedSequence()
{
	TString8 strLibrary( "ScriptedSequence/" );
	strLibrary += s_aScriptedSequences[ s_iScriptedSequence ].szName;

	AAssetLoader::DestroyAssetPackOfLibrary( strLibrary.GetString() );
	s_iScriptedSequence = -1;
}

// VIS group the terrain of the scripted sequence is forced to ("Dummy" if it has one)
static TINT s_iSSeqVISGroup = 0; // 0x007727fc

// Type of the gates the scripted sequence opened or closed (-1 if none) and whether they were open before
static TINT  s_iSSeqGateType = -1;     // 0x00772810
static TBOOL s_bSSeqGateOpen = TFALSE; // 0x00781107

// Read by ASimState::OnUpdate once the sequence is over, the sequence 9 makes it save the game
static TINT s_iLastScriptedSequence = -1; // 0x00772814

static TBOOL LoadScriptedSequenceImpl();

#ifdef BARNYARD_COMMUNITY_PATCH
// Development: the scripted sequence "-devtier" asked for while the main game was loading (see LoadMainGameImpl),
// played by Dev_UpdateDevTierSequence once the sim is up
static TINT          s_iDevTierSequence          = -1;
static const TClass* s_pDevTierSequencePrevState = TNULL;
static TPString8     s_strDevTierSequenceGoal;

static void  Dev_StartDevTierGame();
static TBOOL Dev_ApplyDevTier();
static void  Dev_PopulateDevTier();
#endif

// $Barnyard: FUNCTION 004232c0
static TINT GetSSeqVISGroup()
{
	return s_iSSeqVISGroup;
}

TPSTRING8_DECLARE( bkg_SIM_how_to_move );
TPSTRING8_DECLARE( bkg_SIM_inventory );
TPSTRING8_DECLARE( bkg_SIM_missions );
TPSTRING8_DECLARE( sunglasses );
TPSTRING8_DECLARE( bikekey );
TPSTRING8_DECLARE( bkg_SIM_text_messages );
TPSTRING8_DECLARE( bkg_SIM_barn_champ );
TPSTRING8_DECLARE( bkg_SIM_bike );
TPSTRING8_DECLARE( bkg_SIM_goldrush );
TPSTRING8_DECLARE( bkg_SIM_gopher );
TPSTRING8_DECLARE( bkg_SIM_metal );
TPSTRING8_DECLARE( bkg_SIM_milkweed );
TPSTRING8_DECLARE( bkg_SIM_party );
TPSTRING8_DECLARE( bkg_SIM_recipe );
TPSTRING8_DECLARE( bkg_SIM_sunglasses );
TPSTRING8_DECLARE( player );
TPSTRING8_DECLARE( simplayer );
TPSTRING8_DECLARE( park2 );
TPSTRING8_DECLARE( park2_goals_playerstart );
#ifdef BARNYARD_LATEST
TPSTRING8_DECLARE( pigpiespawn );
TPSTRING8_DECLARE( apple_pie );
#endif
TPSTRING8_DECLARE( waveflag );

TPSTRING8_DECLARE( ui );
TPSTRING8_DECLARE( music );
TPSTRING8_DECLARE( sim_golf_flag );
TPSTRING8_DECLARE( bkg_Showdown );
TPSTRING8_DECLARE( ACowTippingMiniGameState );
TPSTRING8_DECLARE( AGopherShopGame );
TPSTRING8_DECLARE( ACookingMiniGame );
TPSTRING8_DECLARE( ABarnExploreMinigameState );

//-----------------------------------------------------------------------------
// Instance model lists (AInstanceManager::LoadModels), indexed by AMiniGame's instances type
//-----------------------------------------------------------------------------
static TUINT s_aInstanceModels0[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
	0x10, 0x11, 0x12, 0x18, 0x1D, 0x1E, 0x1F, 0x20,
	0x24, 0x25, 0x26, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
	0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x34, 0x35, 0x36,
	0x38, 0x3B, 0x3C, 0x3D, 0x3E, 0x41, 0x40, 0x42,
	0x44, 0x45, 0x46, 0x48, 0x49, 0x4B, 0x4C, 0x4D,
	0x75, 0x76, 0x4F, 0x5A, 0x59, 0x80, 0x81, 0x82,
	0x83, 0x84, 0x85, 0x86
};

// Instance type 1, the frontend
static TUINT s_aFrontEndInstanceModels[] = {
	0x13, 0x14, 0x15, 0x16,
	0x4A, 0x4B, 0x84, 0x47,
	0x38, 0x3F, 0x37, 0x46,
	0x0, 0x1, 0x2, 0x3,
	0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xA, 0xB,
	0xC, 0xD, 0xE, 0xF,
	0x10, 0x11, 0x12, 0x18,
	0x1D, 0x1E, 0x1F, 0x20,
	0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B,
	0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x34, 0x35,
	0x36, 0x39, 0x3A, 0x3B,
	0x3C, 0x3D, 0x3E, 0x41,
	0x40, 0x42, 0x43, 0x44,
	0x45, 0x48, 0x49, 0x4C,
	0x4D, 0x75, 0x76, 0x4F,
	0x5A, 0x59, 0x80, 0x81,
	0x82, 0x83, 0x85, 0x86
};
static constexpr TINT s_iNumFrontEndInstanceModels = TARRAYSIZE( s_aFrontEndInstanceModels );
static constexpr TINT s_iNumFrontEndCollModelNodes = 720;

static TUINT s_aInstanceModels2[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x18,
	0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
	0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
	0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x34, 0x35, 0x36,
	0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x41, 0x40,
	0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4A, 0x4B, 0x4C, 0x4D, 0x75, 0x76, 0x4F, 0x5A,
	0x59, 0x80, 0x81, 0x82, 0x83, 0x38, 0x3F, 0x37
};

static TUINT s_aInstanceModels3[] = {
	0x12, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55,
	0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D,
	0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65,
	0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D,
	0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x28,
	0x2C
};

static TUINT s_aInstanceModels4[] = {
	0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x7, 0x8,
	0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10,
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
	0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55,
	0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D,
	0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65,
	0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D,
	0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75,
	0x76, 0x80
};

static TUINT s_aInstanceModels5[] = {
	0x0, 0x2, 0x4, 0x8, 0x9, 0xA, 0xB, 0x1A,
	0x1B, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x26, 0x29,
	0x2A, 0x12, 0x16, 0x70
};

static TUINT s_aInstanceModels6[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x6, 0x8, 0x9,
	0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11,
	0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x1D,
	0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
	0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
	0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
	0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,
	0x3E, 0x3F, 0x41, 0x40, 0x42, 0x43, 0x44, 0x45,
	0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D,
	0x75, 0x76, 0x4F, 0x5A, 0x59, 0x80
};

// Instance type 7, Mud Jumpers (gatecrash) and other outdoor minigames
static TUINT s_aInstanceModels7[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x41, 0x40, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88
};

struct InstanceModelList
{
	TUINT* pModels;                 // 0x0
	TINT   iNumModels;              // 0x4
	TBOOL  bCreateCollision;        // 0x8
	TINT   iNumCollisionModelNodes; // 0xc
};

static InstanceModelList s_aInstanceModelLists[] = {
	{ s_aInstanceModels0, TARRAYSIZE( s_aInstanceModels0 ), TTRUE, 720 },
	{ s_aFrontEndInstanceModels, s_iNumFrontEndInstanceModels, TFALSE, s_iNumFrontEndCollModelNodes },
	{ s_aInstanceModels2, TARRAYSIZE( s_aInstanceModels2 ), TFALSE, 720 },
	{ s_aInstanceModels3, TARRAYSIZE( s_aInstanceModels3 ), TTRUE, 4000 },
	{ s_aInstanceModels4, TARRAYSIZE( s_aInstanceModels4 ), TFALSE, 720 },
	{ s_aInstanceModels5, TARRAYSIZE( s_aInstanceModels5 ), TFALSE, 720 },
	{ s_aInstanceModels6, TARRAYSIZE( s_aInstanceModels6 ), TTRUE, 720 },
	{ s_aInstanceModels7, TARRAYSIZE( s_aInstanceModels7 ), TTRUE, 720 },
	{ TNULL, 0, TFALSE, 720 },
}; // 0x00773170

// $Barnyard: FUNCTION 00423440
void AGameLoader::DestroyLevel()
{
	if ( AAnimatableObjectManager::IsSingletonCreated() )
	{
		AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

		if ( AAnimatableObjectType* pFlagType = pAnimObjManager->FindType( TPS8( sim_golf_flag ) ) )
		{
			pAnimObjManager->DestroyAnimatableObjectsOfType( pFlagType );
			pAnimObjManager->DestroyType( TPS8( sim_golf_flag ) );
			AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPS8( sim_golf_flag ) );
			AMaterialLibraryManager::GetSingleton()->UnrefLibrary( TPS8( sim_golf_flag ), TTRUE );
		}
	}

	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );

	if ( s_pCurrentTerrain )
		delete s_pCurrentTerrain;

	s_pCurrentTerrain = TNULL;
	s_eCurrentLevel   = Terrain_NUMOF;
	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );

	if ( AGateManager::IsSingletonCreated() )
		delete AGateManager::GetSingleton();
}

// $Barnyard: FUNCTION 00423310
void AGameLoader::DestroyAll()
{
	g_oLoadScreen.RemoveElements();
	AGameStateController::GetSingleton()->ResetStack();

	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );

	if ( s_pCurrentTerrain )
		delete s_pCurrentTerrain;

	s_pCurrentTerrain = TNULL;
	s_eCurrentLevel   = Terrain_NUMOF;
	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );

	if ( AGateManager::IsSingletonCreated() )
		delete AGateManager::GetSingleton();

	if ( ATreeManager::IsSingletonCreated() )
		delete ATreeManager::GetSingleton();

	if ( AInstanceManager::IsSingletonCreated() )
		delete AInstanceManager::GetSingleton();

	if ( ARegrowthManager::IsSingletonCreated() )
		delete ARegrowthManager::GetSingleton();

	if ( ATreasureManager::IsSingletonCreated() )
		delete ATreasureManager::GetSingleton();

	ARenderer::GetSingleton()->GetSkyDome()->UnloadStars();

	AModelRepos* pModelRepos = AModelRepos::GetSingleton();
	pModelRepos->UnloadAllModels();
	pModelRepos->MarkAllModelsUnused();
	pModelRepos->UnloadUnusedModels();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	AKeyFrameLibraryManager::GetSingleton()->UnloadAllLibraries();
	AMaterialLibraryManager::GetSingleton()->DestroyAllLibraries( TTRUE );
	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();
}

// The bike the player left in the sim (see StoreSimBike)
static TBOOL    s_bSimBikeStored = TFALSE; // 0x00781101
static TVector4 s_vecSimBikePosition;      // 0x00781170
static TVector4 s_vecSimBikeDirection;     // 0x00781714

// $Barnyard: FUNCTION 00423590
void AGameLoader::StoreSimBike()
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pPlayer )
		return;

	ABikeManager* pBikeManager = ABikeManager::GetSingleton();
	AUnit*        pBike        = TNULL;
	TINT          iBikeType    = 0;
	pBikeManager->FindClosestBike( *pPlayer->GetSteer()->GetPosition(), pBike, iBikeType );

	if ( pBike && !pBikeManager->IsBikeParked( pBike ) )
	{
		TVector4 vecPosition  = *pBike->GetSteer()->GetPosition();
		TVector4 vecDirection = *pBike->GetSteer()->GetDirection();

		if ( pBike->GetSteer()->GetClass()->IsA( &TGetClass( ABikeSteer ) ) )
			ABikeHelpers::AlignToGround( vecPosition, vecDirection, 2.0f, 40.0f );

		s_vecSimBikePosition  = vecPosition;
		s_vecSimBikeDirection = vecDirection;
		s_bSimBikeStored      = TTRUE;
	}
}

// $Barnyard: FUNCTION 004234f0
void AGameLoader::RestoreSimBike()
{
	if ( !s_bSimBikeStored )
		return;

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( pPlayer )
	{
		ABikeManager* pBikeManager = ABikeManager::GetSingleton();
		AUnit*        pBike        = TNULL;
		TINT          iBikeType    = 0;
		pBikeManager->FindClosestBike( *pPlayer->GetSteer()->GetPosition(), pBike, iBikeType );

		if ( pBike )
		{
			pBike->GetSteer()->SetPosition( s_vecSimBikePosition );
			pBike->GetSteer()->SetDirection( s_vecSimBikeDirection );
			pBikeManager->SetBikeParked( pBike, TFALSE );
		}
	}

	s_bSimBikeStored = TFALSE;
}

// $Barnyard: FUNCTION 00426370
// $BarnyardWiiJP: FUNCTION 8012a518
static TBOOL LoadMiniGameImpl()
{
	AGameTimeFXManager::GetSingleton()->StopAmbientEffects();

	s_bWasBarnExploreMiniGame = TStringManager::String8Compare( s_strCurrentStateName.GetString8().GetString(), "ABarnExploreMinigameState", -1 ) == 0;

	if ( s_bStoryMode && s_iScriptedSequence == -1 )
	{
		// Leaving the story to play a minigame, save the world state first
		APlayerProfileManager::GetSingleton()->GetMainProfile()->StoreQuestTier();

		if ( s_bGameStateLibraryLoaded == TTRUE )
		{
			AHackedQuickSave::GetSingleton()->SaveTime();
			AHackedQuickSave::GetSingleton()->SaveProfile();
		}
		else
		{
			AHackedQuickSave::GetSingleton()->SaveGame();
		}

		AMiniGameManager::GetSingleton()->ClearNextMiniGame();
		g_oLoadScreen.Update();
	}

	AGameStateController::GetSingleton()->ResetStack();
	AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	g_oLoadScreen.Update();

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMusicManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->Reset();
	pSoundManager->UnloadWaveBankSamples( TPS8( music ) );

	// Remove old level
	AGameLoader::DestroyLevel();
	g_oLoadScreen.Update();

	// Remove old instances
	AInstanceManager::GetSingleton()->Reset();
	g_oLoadScreen.Update();

	TRenderInterface::GetSingleton()->FlushDyingResources();

	if ( AAniEffectManager::IsSingletonCreated() )
	{
		delete AAniEffectManager::GetSingleton();
		g_oLoadScreen.Update();
	}

	ARegrowthManager::GetSingleton()->Deactivate();
	g_oLoadScreen.Update();

	AGameSystemManager::GetSingleton()->Reset();
	g_oLoadScreen.Update();

	// NOTE: the original calls DOES_NOTHING (005acd80) on the singleton at 0078a738 here

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	ACameraHelper*  pCameraHelper  = pCameraManager->GetCurrentCameraHelper();

	if ( pCameraHelper && pCameraHelper->IsA( &TGetClass( ABoneAttachCameraHelper ) ) )
	{
		TSTATICCAST( ABoneAttachCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_BONEATTACH ) )->Reset();
	}

	g_oLoadScreen.Update();

	// Unload assets of the previous state
	if ( s_bStoryMode && !s_bGameStateLibraryLoaded )
	{
		if ( s_iScriptedSequence == -1 )
		{
			ATreasureManager::GetSingleton()->Disable();
			g_oLoadScreen.Update();
			AAssetLoader::DestroyAssetPackOfLibrary( "lib_maingame" );
			s_bMainGameLoaded = TFALSE;
		}
		else
		{
			UnloadScriptedSequence();
		}
	}
	else if ( s_bStoryMode && s_bGameStateLibraryLoaded )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( s_strCurrentStateName.GetString8().GetString() );
		s_bGameStateLibraryLoaded = TFALSE;
		s_strCurrentStateName     = TPString8();
	}
	else if ( s_iScriptedSequence != -1 )
	{
		UnloadScriptedSequence();
	}
	else if ( !s_bStoryMode && !s_bGameStateLibraryLoaded )
	{
		// Coming from the frontend
		AAssetLoader::DestroyAssetPackOfLibrary( "lib_frontend" );
		TRenderInterface::GetSingleton()->FlushDyingResources();
		TRenderInterface::GetSingleton()->FlushDyingResources();
	}

	AMiniGameManager* pMiniGameManager = AMiniGameManager::GetSingleton();

	// Loading screen picture
	TPString8 strStateClassName;
	TBOOL     bIsShowdown = pMiniGameManager->GetStateClassName( strStateClassName, s_iCurrMiniGame ) == TPS8( ACowTippingMiniGameState ) && pMiniGameManager->GetMiniGameVariant() == 2;

#ifdef BARNYARD_LATEST
	// Wii: the XUI load screen of the minigame replaces the picture
	// NOTE: the Wii build also loads the fonts and the button icons when the minigame only has a Wii Remote
	// instruction scene (ARevInstructionState, not recovered)
	TPString8 strXUILoadScreen = bIsShowdown ? TPString8( "CoyoteShowdown_LoadScreen" ) : pMiniGameManager->GetXUILoadScreenName( s_iCurrMiniGame );

	if ( strXUILoadScreen.IsEmpty() || !ShowXUILoadScreen( strXUILoadScreen, TTRUE ) )
#endif
	{
		TPString8 strPicture;
		if ( bIsShowdown )
			AGUISlideshow::LocaliseBackgroundFileName( strPicture, TPS8( bkg_Showdown ) );
		else
			pMiniGameManager->GetLoadingScreenName( strPicture, s_iCurrMiniGame );

		g_oLoadScreen.SetPicture( strPicture, TTRUE );
	}

	pMiniGameManager->SetNextMiniGameIndex( s_iCurrMiniGame );
	AGameTimeManager::GetSingleton()->PushUpdateState();
	APlayerManager::GetSingleton()->SetCurrentPlayerSet( APlayerManager::PLAYERSET_MINIGAME );
	g_oLoadScreen.Update();

	AGameSystemManager::GetSingleton()->SetUnk104( 2 );

	// Load assets of the minigame
	AAssetLoader::LoadAssetPackOfLibrary( pMiniGameManager->GetLibraryName( s_iCurrMiniGame ), TTRUE );
	g_oLoadScreen.Update();

	const TINT iInstancesType = pMiniGameManager->GetMiniGame( s_iCurrMiniGame )->GetInstancesType();
	ACollisionManager::GetSingleton()->CreateObjectHashMain( ACollisionManager::HashType( iInstancesType ) );
	g_oLoadScreen.Update();

	if ( iInstancesType != -1 )
	{
		if ( pMiniGameManager->GetMiniGame( s_iCurrMiniGame )->GetLoadFlags() & AMiniGame::LOADFLAGS_REGROWTH )
		{
			ARegrowthManager::GetSingleton()->Activate( TTRUE );
			ARegrowthManager::GetSingleton()->LoadModels();
		}

		const InstanceModelList& rModelList = s_aInstanceModelLists[ iInstancesType ];
		AInstanceManager::GetSingleton()->LoadModels( iInstancesType, rModelList.iNumModels, rModelList.pModels, rModelList.bCreateCollision, rModelList.bCreateCollision, rModelList.iNumCollisionModelNodes );
	}

	g_oLoadScreen.Update();
	AAssetLoader::CreateAssetsFromLibrary( pMiniGameManager->GetLibraryName( s_iCurrMiniGame ) );

	// Load music wavebank samples
	ASoundManager::ms_bShouldUpdateLoadingScreen = TTRUE;
	pSoundManager->LoadWaveBankSamples( TPS8( music ), AWaveBank::LOADFLAGS_NONE, -1 );
	ASoundManager::ms_bShouldUpdateLoadingScreen = TFALSE;

	// Load music samples
	pSoundManager->LoadSoundBankSamples( TPS8( music ) );
	g_oLoadScreen.Update();

	// NOTE: the original calls DOES_NOTHING (005acd80) on AShadowManager here
	// NOTE: the Wii build loads the Wii Remote instruction scene of the minigame here (ARevInstructionState, not recovered)

	// Create and push the minigame state
	AGameState* pMiniGameState = TSTATICCAST( AGameState, pMiniGameManager->GetStateClass( s_iCurrMiniGame )->CreateObject() );
	AGameStateController::GetSingleton()->PushState( pMiniGameState );
	g_oLoadScreen.Update();

	s_bInMiniGame = TTRUE;
	pMiniGameManager->GetNextMiniGame();

	if ( pMiniGameManager->GetMiniGame( s_iCurrMiniGame )->GetLoadFlags() & AMiniGame::LOADFLAGS_GATES )
	{
		if ( !AGateManager::IsSingletonCreated() )
		{
			new AGateManager();
			g_oLoadScreen.Update();

			// The gates of the bike races
			if ( pMiniGameManager->GetStateClass( s_iCurrMiniGame ) == &TGetClass( ABikeRaceMicroGame ) )
			{
				AGateManager* pGateManager = AGateManager::GetSingleton();

				switch ( pMiniGameManager->GetMiniGameVariant() )
				{
					case 2:
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_GOLF, TFALSE );
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_SOUTHPATH, TFALSE );
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_WOODS, TFALSE );
						pGateManager->FindGate( AGateManager::GATETYPE_GOLF, 3 )->Close();
						break;
					case 3:
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_OLDMILL, TFALSE );
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_WOODS, TFALSE );
						break;
					case 4:
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_GULCH, TFALSE );
						break;
					case 5:
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_GOLF, TFALSE );
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_OLDMILL, TFALSE );
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_GULCH, TFALSE );
						pGateManager->OpenGatesOfType( AGateManager::GATETYPE_SOUTHPATH, TFALSE );
						break;
					default:
						break;
				}
			}

			g_oLoadScreen.Update();
		}
	}

	const TFLOAT fFOV = pMiniGameState->GetFOV();
	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( fFOV );
	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );

	TPString8 strLoadingScreen;
	if ( !pMiniGameManager->GetLoadingScreenName( strLoadingScreen, s_iCurrMiniGame ).IsEmpty() )
	{
		// Keeps showing the loading picture until the player dismisses it, then starts the minigame
		AMiniGameInstructionState* pInstructionState = new AMiniGameInstructionState();
		pInstructionState->SetMiniGame( pMiniGameState );
		AGameStateController::GetSingleton()->PushState( pInstructionState );
	}
	else
	{
		g_oLoadScreen.Reset();
		g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );
	}

	g_oLoadScreen.ResetIndicators( TTRUE );

	AWeatherManager::GetSingleton()->LockToCurrentWeatherType( TFALSE );

	return TTRUE;
}

// $Barnyard: FUNCTION 00423720
static void OnCollisionModelLoaded( ATerrainSection::ModelNode* a_pCollisionModel )
{
	TINT iGroupId = ATerrainInterface::GetSingleton()->GetCollisionModelSectionIndex( a_pCollisionModel );

	if ( AInteriorNetworkManager::IsSingletonCreated() )
		AInteriorNetworkManager::GetSingleton()->UpdateGoalCollisionGroup( a_pCollisionModel, iGroupId );

	AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( pCurrentState )
		pCurrentState->OnCollisionModelLoaded( a_pCollisionModel, (void*)iGroupId );

	AShadowManager::GetSingleton()->OnTerrainSectionChanged( (TINT)a_pCollisionModel, iGroupId );
}

// $Barnyard: FUNCTION 00423770
static void OnVISGroupChanged( ATerrainSection* a_pCurrent, ATerrainSection* a_pPrevious )
{
	AGameTimeFXManager::GetSingleton()->OnVISGroupChanged( a_pCurrent, a_pPrevious );

	AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( pCurrentState )
		pCurrentState->OnVISGroupChanged( a_pCurrent, a_pPrevious );
}

// $Barnyard: FUNCTION 004271f0
// $BarnyardWiiJP: FUNCTION 8012ba9c
static TBOOL ReturnToFrontEndImpl()
{
	AMiniGameManager::ms_bHasFade = TFALSE;
	AGameStateController::GetSingleton()->ResetStack();
	AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	g_oLoadScreen.Update();

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMusicManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->Reset();
	pSoundManager->UnloadWaveBankSamples( TPS8( music ) );
	g_oLoadScreen.Update();

	AHackedQuickSave::GetSingleton()->ResetBuffers( TTRUE );

	if ( AAniEffectManager::IsSingletonCreated() )
		delete AAniEffectManager::GetSingleton();

	g_oLoadScreen.Update();

	// Remove the level
	AGameLoader::DestroyLevel();
	AInstanceManager::GetSingleton()->Reset();
	g_oLoadScreen.Update();

	TRenderInterface::GetSingleton()->FlushDyingResources();
	ARegrowthManager::GetSingleton()->Deactivate();
	g_oLoadScreen.Update();

	AGameSystemManager::GetSingleton()->Reset();
	g_oLoadScreen.Update();

	// NOTE: the original calls DOES_NOTHING (005acd80) on the singleton at 0078a738 here

	AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	g_oLoadScreen.Update();

#ifdef BARNYARD_LATEST
	AGUI2* pGUI = AGUI2::GetSingleton();
	pGUI->DeleteXUIScenes();
	pGUI->DeleteXUIResources();
	pGUI->UnloadXUIMaterialLibraries();
	pGUI->CloseXUIFonts();
#endif

	// Unload the assets of the state that is left
	if ( s_bInMiniGame == TTRUE )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( AMiniGameManager::GetSingleton()->GetLibraryName( s_iCurrMiniGame ) );
		s_bInMiniGame = TFALSE;
	}
	else if ( s_bGameStateLibraryLoaded == TTRUE )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( s_strCurrentStateName.GetString8().GetString() );
		s_bGameStateLibraryLoaded = TFALSE;
		s_strCurrentStateName     = TPString8();
	}
	else if ( s_iScriptedSequence == -1 )
	{
		ATreasureManager::GetSingleton()->Disable();
		AAssetLoader::DestroyAssetPackOfLibrary( "lib_maingame" );
		s_bMainGameLoaded = TFALSE;
	}
	else
	{
		UnloadScriptedSequence();
	}

	g_oLoadScreen.Update();

	// Load the frontend
	AAssetLoader::LoadAssetPackOfLibrary( "lib_frontend", TTRUE );
	g_oLoadScreen.Update();

	const InstanceModelList& rModelList = s_aInstanceModelLists[ 1 ];
	ACollisionManager::GetSingleton()->CreateObjectHashMain( ACollisionManager::HashType_UNK2 );
	AInstanceManager::GetSingleton()->LoadModels( 1, rModelList.iNumModels, rModelList.pModels, rModelList.bCreateCollision, rModelList.bCreateCollision, rModelList.iNumCollisionModelNodes );
	g_oLoadScreen.Update();

	AGameLoader::Terrain_Set( AGameLoader::Terrain_FrontEnd, TFALSE, TTRUE, 0, 0, 0, 0 );

	// Load the sections the frontend shows, the last one forced is the barn
	static const TCHAR* s_aszFrontEndSections[] = {
		"Barn",
		"Barnnorth",
		"BGbarneast",
		"BGbarnwest",
		"Hillbase",
		"Hilltop",
		"Meadowsouth",
	}; // 0x007733c8

	for ( TINT i = TARRAYSIZE( s_aszFrontEndSections ) - 1; i >= 0; i-- )
	{
		ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
		pTerrain->ForceSection( pTerrain->GetSectionIndex( s_aszFrontEndSections[ i ] ), TTRUE );
	}

	AAssetLoader::CreateAssetsFromLibrary( "lib_frontend" );

	// Load music wavebank samples
	ASoundManager::ms_bShouldUpdateLoadingScreen = TTRUE;
	pSoundManager->LoadWaveBankSamples( TPS8( music ), AWaveBank::LOADFLAGS_NONE, -1 );
	ASoundManager::ms_bShouldUpdateLoadingScreen = TFALSE;

	// Load music samples
	pSoundManager->LoadSoundBankSamples( TPS8( music ) );
	g_oLoadScreen.Update();

	ACameraManager::GetSingleton()->DetachCameraHelpers();
	AFrontEndMainMenuState2::SetupCamera();
	AGameTimeManager::GetSingleton()->ChangePhaseOfDay( AGameTimeManager::DAYPHASE( 1 ), TTRUE, TTRUE );
	g_oLoadScreen.Update();

	// Push the main menu and the state that was asked for
	AGameStateController::GetSingleton()->PushState( new AFrontEndMainMenuState2() );

	if ( !s_strCurrentStateName.IsEmpty() )
	{
		const TClass* pClass = TClass::Find( s_strCurrentStateName.GetString8().GetString(), &TGetClass( AGameState ) );
		AGameStateController::GetSingleton()->PushState( TSTATICCAST( AGameState, pClass->CreateObject() ) );
	}

	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
	g_oLoadScreen.ResetIndicators( TTRUE );
	AGameTimeManager::GetSingleton()->PopUpdateState();

	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );
	AWeatherManager::GetSingleton()->LockToCurrentWeatherType( TTRUE );

	// The barn doors of the frontend are open
	if ( !AGateManager::IsSingletonCreated() )
		new AGateManager();

	AGateManager::GetSingleton()->OpenGatesOfType( AGateManager::GATETYPE_BARN, TFALSE );
	AGateManager::GetSingleton()->OpenGatesOfType( AGateManager::GATETYPE_NIGHTBARN, TFALSE );

	return TTRUE;
}

//-----------------------------------------------------------------------------
// Main game
//-----------------------------------------------------------------------------

// NOTE: the original keeps this table (and the two functions below) next to AGameStateController
static TPString8* s_aSimLoadingPictures[] = {
	&TPS8( bkg_SIM_how_to_move ),
	&TPS8( bkg_SIM_inventory ),
	&TPS8( bkg_SIM_missions ),
	&TPS8( bkg_SIM_text_messages ),
	&TPS8( bkg_SIM_barn_champ ),
	&TPS8( bkg_SIM_bike ),
	&TPS8( bkg_SIM_goldrush ),
	&TPS8( bkg_SIM_gopher ),
	&TPS8( bkg_SIM_metal ),
	&TPS8( bkg_SIM_milkweed ),
	&TPS8( bkg_SIM_party ),
	&TPS8( bkg_SIM_recipe ),
	&TPS8( bkg_SIM_sunglasses ),
}; // 0x007733f8

#ifdef BARNYARD_LATEST

// Indices of s_aSimLoadingPictures
static constexpr TINT SIMLOADINGPICTURE_GOPHER = 7;
static constexpr TINT SIMLOADINGPICTURE_RECIPE = 11;

// Wii: the XUI help scenes shown instead of s_aSimLoadingPictures (the same order)
// NOTE: the Wii table (80505260) has one more scene, Wii_Gameplay_Help (how to use the Wii Remote motion
// tutorials of ARevInstructionState), as its 5th entry, so from bkg_SIM_barn_champ on the scenes are one
// entry further than the pictures. The Wii build still unlocks the pictures by name with the PC indices
// (EnableInstructionScreen, 80132ee4), so there bkg_SIM_barn_champ unlocks Wii_Gameplay_Help and bkg_SIM_party
// unlocks Wii_Milkweed (it also always unlocks Wii_Gameplay_Help); ABarnRewardState uses the Wii indices.
// The PC indices are kept here, so saves stay compatible.
// NOTE: the help pages that teach the Wii Remote and Nunchuk controls aren't used (TNULL): their PC picture,
// with the PC controls, is shown instead.
static const TCHAR* s_aSimLoadingXUIScenes[] = {
	TNULL, // Wii_Move_Around
	TNULL, // Wii_Inventory
	"Wii_Complete_Missions",
	TNULL, // Wii_Text_Messages
	"Wii_Barnyard_Champion",
	TNULL, // Wii_Bike
	TNULL, // Wii_Gold_Rush
	"Wii_The_Gopher_Shop",
	TNULL, // Wii_Metal_Detector
	TNULL, // Wii_Milkweed
	"Wii_Party_Animal",
	TNULL, // Wii_Recipes
	TNULL, // Wii_Sunglasses
};

TSTATICASSERT( TARRAYSIZE( s_aSimLoadingXUIScenes ) == TARRAYSIZE( s_aSimLoadingPictures ) );

// Shows the XUI help scene of the sim loading picture a_iPicture, returns TFALSE if it has none
// NOTE: inlined into LoadMainGameImpl and LoadGameStateImpl in the Wii build (the scene names come from 80133088)
static TBOOL ShowSimLoadingXUIScene( TINT a_iPicture )
{
	if ( !s_aSimLoadingXUIScenes[ a_iPicture ] )
		return TFALSE;

	return ShowXUILoadScreen( TPString8( s_aSimLoadingXUIScenes[ a_iPicture ] ), TTRUE );
}

#endif // BARNYARD_LATEST

// Picks a random picture out of the ones enabled in s_uiSimLoadingPictures, returns -1 if there's none
// $Barnyard: FUNCTION 0042ab90
static TINT GetRandomSimLoadingPicture()
{
	TINT iNumPictures = 0;

	for ( TINT i = 0; i < 13; i++ )
		iNumPictures += ( s_uiSimLoadingPictures & ( 1 << i ) ) ? 1 : 0;

	if ( iNumPictures == 0 )
		return -1;

	// Skip the pictures that aren't enabled
	TINT iPicture = ARandom::GetSingleton()->m_oRandom.GetInt( iNumPictures );

	for ( TINT i = 0; i <= iPicture; i++ )
	{
		if ( !( s_uiSimLoadingPictures & ( 1 << i ) ) )
			iPicture++;
	}

	return iPicture;
}

// $Barnyard: FUNCTION 0042ac00
TPString8& AGameLoader::GetSimLoadingPicture( TPString8& a_rOutName, TINT a_iPicture )
{
	AGUISlideshow::LocaliseBackgroundFileName( a_rOutName, *s_aSimLoadingPictures[ a_iPicture ] );
	return a_rOutName;
}

TBOOL AGameLoader::IsSimLoadingPictureUnlocked( TINT a_iPicture )
{
	return ( s_uiSimLoadingPictures & ( 1 << a_iPicture ) ) != 0;
}

void AGameLoader::UnlockSimLoadingPicture( TINT a_iPicture )
{
	s_uiSimLoadingPictures |= 1 << a_iPicture;
}

// $Barnyard: FUNCTION 0042ab40
TBOOL AGameLoader::UnlockSimLoadingPicture( const TPString8& a_rcName )
{
	TINT i = 0;

	while ( *s_aSimLoadingPictures[ i ] != a_rcName )
	{
		if ( ++i >= TARRAYSIZE( s_aSimLoadingPictures ) )
			return TFALSE;
	}

	if ( i <= -1 )
		return TFALSE;

	s_uiSimLoadingPictures |= 1 << i;
	return TTRUE;
}

// Loads the main game terrain (EnvMain) unless it's already loaded with the section a_iStartVISGroup, and creates the golf flags
// $Barnyard: FUNCTION 00424280
TBOOL Terrain_SetMain( TBOOL a_bStreamCollision, TINT a_iStartVISGroup, TBOOL a_bLoadLater )
{
	if ( s_eCurrentLevel == AGameLoader::Terrain_EnvMain && a_bStreamCollision && a_iStartVISGroup >= 0 &&
	     s_pCurrentTerrain->GetSection( a_iStartVISGroup )->IsLODLoaded( ATerrainLODType_High ) )
	{
		// The section is already there
		return TTRUE;
	}

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	if ( AAnimatableObjectType* pGolfFlagType = pAnimObjManager->FindType( TPS8( sim_golf_flag ) ) )
	{
		pAnimObjManager->DestroyAnimatableObjectsOfType( pGolfFlagType );
		pAnimObjManager->DestroyType( TPS8( sim_golf_flag ) );
		AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPS8( sim_golf_flag ) );
		AMaterialLibraryManager::GetSingleton()->UnrefLibrary( TPS8( sim_golf_flag ), TTRUE );
	}

	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );

	if ( s_pCurrentTerrain )
		delete s_pCurrentTerrain;

	s_pCurrentTerrain = TNULL;
	s_eCurrentLevel   = AGameLoader::Terrain_NUMOF;
	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
	TRenderInterface::GetSingleton()->FlushDyingResources();

	TINT iSavedVIS     = ASimPersistData::GetSingleton()->GetTerrainVIS();
	s_iTerrainVISIndex = ( iSavedVIS == -1 ) ? 0 : iSavedVIS;

	if ( a_iStartVISGroup >= 0 )
		s_iTerrainVISIndex = a_iStartVISGroup;

	s_pCurrentTerrain = new ATerrainInterface( 4, 0x500000, 0x700000, s_iTerrainVISIndex );

	TCHAR szTerrainFile[ 128 ];
	TStringManager::String8Format( szTerrainFile, sizeof( szTerrainFile ), "Data\\Terrain\\%s\\%s.trb", AGameLoader::GetTerrainName( AGameLoader::Terrain_EnvMain ), AGameLoader::GetTerrainName( AGameLoader::Terrain_EnvMain ) );

	s_pCurrentTerrain->LoadFromFile( szTerrainFile, TTRUE, a_bStreamCollision );
	s_eCurrentLevel = AGameLoader::Terrain_EnvMain;

	if ( !a_bLoadLater )
		AGameLoader::Terrain_Load();

	// Create the golf flags at the golf_flag locators
	TTRB* pAssetPackTRB = AAssetLoader::GetAssetTRB( AAssetType_AssetPack );
	AKeyFrameLibraryManager::GetSingleton()->LoadLibrary( TPS8( sim_golf_flag ), pAssetPackTRB );
	AMaterialLibraryManager::GetSingleton()->LoadLibrary( TPS8( sim_golf_flag ), pAssetPackTRB, TTRUE );
	pAnimObjManager->LoadAnimObjType( TPS8( sim_golf_flag ), TNULL, TFALSE );

	AAnimatableObjectType* pGolfFlagType = pAnimObjManager->FindType( TPS8( sim_golf_flag ) );
	ATerrainLocatorList*   pLocatorList  = s_pCurrentTerrain->GetLocatorList();

	for ( TINT i = 0; i < pLocatorList->GetNumLocators(); i++ )
	{
		if ( TStringManager::String8Compare( pLocatorList->GetLocatorName( i ), "golf_flag", 10 ) != 0 )
			continue;

		AAnimatableObject* pGolfFlag = pAnimObjManager->CreateAnimatableObject( pGolfFlagType, TNULL, 0 );

		TMatrix44 mLocator;
		pLocatorList->GetLocator( i )->GetMatrix( mLocator );

		TMatrix44 mWorld;
		pLocatorList->GetWorldTransform().GetLocalMatrixImp( mWorld );

		TMatrix44 mTransform;
		mTransform.Multiply( mWorld, mLocator );

		pGolfFlag->GetModelInstance()->SetTransform( mTransform );
		pGolfFlag->SetSkeletonUpdating( TTRUE, TTRUE );
		pGolfFlag->PlayAnimation( TPS8( waveflag ) );
	}

	AGameTimeFXManager::GetSingleton()->Reset();

	return TTRUE;
}

// Spawns the player of the sim (unit type "player", named "simplayer")
// $Barnyard: FUNCTION 004246a0
void AGameLoader::SpawnSimPlayer()
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	APlayerSet*     pPlayerSet     = pPlayerManager->GetPlayerSet( APlayerManager::PLAYERSET_SIM );

	pPlayerSet->Reset();

	pPlayerSet->AddHumanPlayer( AInputHandler::GetSingleton()->GetMainController(), -1 );
	pPlayerManager->SetCurrentPlayerSet( APlayerManager::PLAYERSET_SIM );

	// Skin of the model: the male or female player cow (the barn explore minigame uses the other two)
	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();
	const TBOOL            bIsFemale       = pProfileManager->GetMainGender() == APlayerProfile::GENDER_FEMALE;
	const TINT             iSkin           = s_bBarnExploreMiniGame ? ( bIsFemale ? 3 : 2 ) : ( bIsFemale ? 1 : 0 );

	AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, iSkin, -1, TTRUE );
	AUnit*         pPlayer = AUnitManager::GetSingleton()->SpawnUnit( TPS8( player ), &oTransform );

	AAnimatedUnit* pAnimatedPlayer = TSTATICCAST( AAnimatedUnit, pPlayer );
	pAnimatedPlayer->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );

	pAnimatedPlayer->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

	AUnitNameManager::GetSingleton()->SetUnitName( TPS8( simplayer ), pPlayer->GetId() );
}

// Loads the main game: lib_maingame, EnvMain, the sim player, then pushes ASimState
// $Barnyard: FUNCTION 00425bc0
// $BarnyardWiiJP: FUNCTION 80129a2c
static TBOOL LoadMainGameImpl()
{
	s_pPrevGameStateClass = TNULL;

	AGameStateController::GetSingleton()->ResetStack();
	AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMusicManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->Reset();
	pSoundManager->UnloadWaveBankSamples( TPS8( music ) );

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	ACameraHelper*  pCameraHelper  = pCameraManager->GetCurrentCameraHelper();

	if ( pCameraHelper && pCameraHelper->IsA( &TGetClass( ABoneAttachCameraHelper ) ) )
	{
		ABoneAttachCameraHelper* pBoneAttachHelper = TSTATICCAST( ABoneAttachCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_BONEATTACH ) );
		pBoneAttachHelper->Reset();
		pBoneAttachHelper->OnDetach();
	}

	// Remove the old level
	AGameLoader::DestroyLevel();
	AInstanceManager::GetSingleton()->Reset();
	AGameSystemManager::GetSingleton()->Reset();

	// NOTE: the original calls DOES_NOTHING (005acd80) on the singleton at 0078a738 here

	TRenderInterface::GetSingleton()->FlushDyingResources();

	if ( AAniEffectManager::IsSingletonCreated() )
		delete AAniEffectManager::GetSingleton();

	ARegrowthManager::GetSingleton()->Deactivate();
	g_oLoadScreen.Update();

	// Unload the assets of the previous state
	if ( s_bInMiniGame == TTRUE )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( AMiniGameManager::GetSingleton()->GetLibraryName( s_iCurrMiniGame ) );
		s_bInMiniGame = TFALSE;
	}
	else if ( s_bGameStateLibraryLoaded == TTRUE )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( s_strCurrentStateName.GetString8().GetString() );
		g_oLoadScreen.Update();
		s_bGameStateLibraryLoaded = TFALSE;
		s_bWasBarnExploreMiniGame = TStringManager::String8Compare( s_strCurrentStateName.GetString8().GetString(), "ABarnExploreMinigameState", -1 ) == 0;
		s_strCurrentStateName     = TPString8();
	}
	else if ( s_iScriptedSequence == -1 )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( "lib_frontend" );
	}
	else
	{
		UnloadScriptedSequence();
	}

	// Loading screen picture
	const TINT iPicture = GetRandomSimLoadingPicture();

	if ( iPicture != -1 )
	{
#ifdef BARNYARD_LATEST
		// Wii: the XUI help scene of the picture
		if ( !ShowSimLoadingXUIScene( iPicture ) )
#endif
		{
			TPString8 strPicture;
			g_oLoadScreen.SetPicture( AGameLoader::GetSimLoadingPicture( strPicture, iPicture ), TTRUE );
		}
	}

	// Load the assets of the main game
	AAssetLoader::LoadAssetPackOfLibrary( "lib_maingame", TTRUE );
	g_oLoadScreen.Update();

	ACollisionManager::GetSingleton()->CreateObjectHashMain( ACollisionManager::HashType( 0 ) );

	ARegrowthManager::GetSingleton()->Activate( TTRUE );
	ARegrowthManager::GetSingleton()->LoadModels();
	ARegrowthManager::GetSingleton()->SetInUse( TTRUE );

	const InstanceModelList& rModelList = s_aInstanceModelLists[ 0 ];
	AInstanceManager::GetSingleton()->LoadModels( 0, rModelList.iNumModels, rModelList.pModels, rModelList.bCreateCollision, rModelList.bCreateCollision, rModelList.iNumCollisionModelNodes );
	TRenderInterface::GetSingleton()->FlushDyingResources();

	Terrain_SetMain( TTRUE, s_iMainGameStartVIS, TFALSE );
	AGameLoader::Terrain_Load();

	new AAniEffectManager();

	AGameSystemManager::GetSingleton()->SetUnk104( 1 );
	AAssetLoader::CreateAssetsFromLibrary( "lib_maingame" );

	// Bikes that are unlocked
	// The bikes 1-5 are the items 309-313 of the gopher shop
	for ( TINT i = 0; i < 5; i++ )
	{
		if ( AGopherShopInterface::HasBought( i + 309 ) )
			ABikeManager::GetSingleton()->SetBikeTypeUnlocked( ABikeManager::GetBikeTypeFlag( i ), TTRUE );
	}

	ABikeManager::GetSingleton()->LoadBikeAssets();

	AModelLoader::ResolveTextures();

	// NOTE: the original calls DOES_NOTHING (005acd80) on AShadowManager here

	AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( TPS8( park2 ) );
	g_oLoadScreen.Update();

	if ( !AGateManager::IsSingletonCreated() )
		new AGateManager();

	ATreasureManager::GetSingleton()->Enable();

	ASimpleProjectileManager::GetSingleton()->CreateDefaultPools();

	AGameTimeFXManager::GetSingleton()->ClearForcedFX();
	AWeatherManager::GetSingleton()->LockToCurrentWeatherType( TFALSE );

	AGameTimeManager::GetSingleton()->ChangePhaseOfDay( AGameTimeManager::DAYPHASE_DAWN, TTRUE, TTRUE );
	AGameLoader::SpawnSimPlayer();

	AHackedQuickSave::GetSingleton()->LoadTime();

	if ( s_bStoryMode == TTRUE || s_bLoadingSavedGame == TTRUE )
	{
		// Restore the saved world
		AHackedQuickSave::GetSingleton()->LoadGame();

		// Restore the honey of the hives kept by the honey collecting game
		if ( AReplayManager::ms_bRestoreHoneyCollecting == TTRUE )
		{
			AReplayHoneyCollecting* pHoneyReplay = TSTATICCAST( AReplayHoneyCollecting, AReplayManager::GetSingleton()->FindReplay( TPS8( honey_collecting_replay ) ) );
			APlayerSimProfile*      pProfile     = APlayerProfileManager::GetSingleton()->GetMainProfile();

			for ( TINT i = 0; i < AReplayHoneyCollecting::NUM_HIVES; i++ )
				pHoneyReplay->m_aiHoney[ i ] = pProfile->GetHoneyReplay( i );

			AReplayManager::ms_bRestoreHoneyCollecting = TFALSE;
		}

		// Spawn the animals of the time of day
		AAnimalPopulationManager::GetSingleton()->Populate( AGameTimeManager::GetSingleton()->IsItNight() ? 6 : 5, TFALSE );

		if ( s_bLoadingSavedGame )
		{
			s_strPlayerStartGoal      = TPS8( park2_goals_playerstart );
			s_bPlacePlayerAtStartGoal = TTRUE;
		}
#ifdef BARNYARD_LATEST
		else if ( AQuestManager::GetSingleton()->GetNumAssignedQuests() > 0 )
		{
			// Latest version: back from fetching the apple pie of "Pig Apple Pie", the player starts at the Pig
			AQuestInstance* pQuest = AQuestManager::GetSingleton()->GetAssignedQuest( 0 );

			if ( pQuest->GetQuest()->GetName() == TPS8D( "Pig Apple Pie" ) )
			{
				AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetHumanPlayerUnit( 0 );
				AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

#  ifdef BARNYARD_COMMUNITY_PATCH
				// NOTE: the Wii also does it when the quest can't be done at this time of the day (at night): the Pig then
				// can't be talked to, so the screen stays black and the controls are held until the morning
				if ( pInventory->FindItemByType( TPS8( apple_pie ) ) && !AQuestManager::GetSingleton()->IsQuestUnavailable( pQuest->GetQuest() ) )
#  else
				if ( pInventory->FindItemByType( TPS8( apple_pie ) ) )
#  endif
				{
					s_strPlayerStartGoal      = TPS8( pigpiespawn );
					s_bPlacePlayerAtStartGoal = TTRUE;

					ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
					pTerrain->ForceSection( pTerrain->GetSectionIndex( "barnnorth" ), TTRUE );
					AGameLoader::Terrain_Load();

					s_bPigApplePieReturn = TTRUE;
				}
			}
		}
#endif

		// NOTE: the original doesn't check for a missing slot here
		APlayerProgress::SaveSlot* pSlot = APlayerProgress::GetSingleton()->GetCurrentSlot();

		if ( pSlot->eFlags & APlayerProgress::SLOTFLAGS_HERO )
		{
			AQuestManager* pQuestManager = AQuestManager::GetSingleton();

			if ( pQuestManager->GetAct() == 3 && pQuestManager->GetTier() == 16 )
			{
				// The game has been finished, remove the quests of the last tier
				// NOTE: the original walks the vector while RemoveQuest erases from it, the quests are copied first here
				AQuestInstance* apQuests[ AQuestManager::MAX_NUM_ASSIGNED_QUESTS ];
				TINT            iNumQuests = pQuestManager->GetNumAssignedQuests();

				for ( TINT i = 0; i < iNumQuests; i++ )
					apQuests[ i ] = pQuestManager->GetAssignedQuest( i );

				for ( TINT i = 0; i < iNumQuests; i++ )
				{
					TINT iAct  = -1;
					TINT iTier = -1;
					pQuestManager->FindQuestTier( apQuests[ i ], &iAct, &iTier );

					if ( iAct == 3 && iTier == 16 )
						pQuestManager->RemoveQuest( apQuests[ i ], TTRUE, TTRUE );
				}
			}
		}
	}
	else
	{
		// New game
		s_strPlayerStartGoal      = TPS8( park2_goals_playerstart );
		s_bPlacePlayerAtStartGoal = TTRUE;

#ifdef BARNYARD_COMMUNITY_PATCH
		// Development: "-devtier <act> <tier>" starts the story at that tier. Before the animals are spawned, the
		// same as a loaded save (the population depends on the tier, the act's assets only have the act's units)
		const TBOOL bDevTier = Dev_ApplyDevTier();

		// NOTE: SetQuestTier (called by ApplyDevTier) requests the tier's scripted sequence in the middle of the
		// transition, where it would never load and would leave s_iScriptedSequence set. Retail only gets here for
		// tier 1 (no sequence), so the request is deferred until the sim is up
		if ( bDevTier && s_iScriptedSequence != -1 )
		{
			s_iDevTierSequence          = s_iScriptedSequence;
			s_pDevTierSequencePrevState = s_pPrevGameStateClass;
			s_strDevTierSequenceGoal    = s_strPlayerStartGoal;
			s_iScriptedSequence         = -1;
			s_iLastScriptedSequence     = -1;
		}
#else
		const TBOOL bDevTier = TFALSE;
#endif

		// Spawn the animals of the time of day
		AAnimalPopulationManager::GetSingleton()->Populate( AGameTimeManager::GetSingleton()->IsItNight() ? 6 : 5, TFALSE );

		if ( !bDevTier )
			AQuestManager::GetSingleton()->SetQuestTier( 1, 1, TFALSE );
#ifdef BARNYARD_COMMUNITY_PATCH
		else
			Dev_PopulateDevTier();
#endif

		AGameTimeManager::GetSingleton()->SetTime( 10, 0, 0, TFALSE, TTRUE );
	}

	AGameTimeManager::GetSingleton()->PopUpdateState();

	if ( s_bPlacePlayerAtStartGoal )
	{
		// Place the player at the start goal
		s_bPlacePlayerAtStartGoal = TFALSE;

		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetPlayerUnit( 0 );
#ifdef BARNYARD_LATEST
		APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, pPlayer->GetSteer() );

		if ( s_strPlayerStartGoal == TPS8( pigpiespawn ) && !TSTATICCAST( AInteriorNetwork, pSteer->GetCurrentNetwork() )->HasGoal( s_strPlayerStartGoal ) )
		{
			// NOTE: "pigpiespawn" is only in the Wii park2 network (the Wii edits the network, PC retail files are never
			// replaced), so the player is put at the place and facing of the Wii goal, next to the pig sty
			AInteriorGoal oGoal;
			oGoal.m_vForward   = TVector4( 0.387119f, 0.0f, 0.92203f, 1.0f );
			oGoal.m_fPositionX = 14.827578f;
			oGoal.m_fPositionY = -8.396326f;
			oGoal.m_fPositionZ = 16.571123f;
			pSteer->PlaceAtGoal( &oGoal, TTRUE, TFALSE );
		}
		else
		{
			pSteer->PlaceAtNamedGoal( s_strPlayerStartGoal, TTRUE, TTRUE );
		}
#else
		TSTATICCAST( APlayerSteer, pPlayer->GetSteer() )->PlaceAtNamedGoal( s_strPlayerStartGoal, TTRUE, TTRUE );
#endif
	}

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( ACamera::sm_fCurrentFOV );
	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );

	s_bMainGameLoaded = TTRUE;

	// Create and push the sim
	ASimState* pSimState = new ASimState();
	AGameStateController::GetSingleton()->PushState( pSimState );

	AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
	AMusicManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	s_bLoadingSavedGame = TFALSE;
	s_bStoryMode        = TTRUE;
	pSimState->OnLoaded();

	ASimPersistData::GetSingleton()->SetFlags( ASimPersistData::FLAGS_MAINGAME_LOADED );

	g_oLoadScreen.ResetIndicators( TTRUE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00424c30
TPString8 AGameLoader::GetPrevGameStateName()
{
	if ( s_pPrevGameStateClass )
		return TPString8( s_pPrevGameStateClass->GetName() );

	return TPString8();
}

// $Barnyard: FUNCTION 00426b70
// $BarnyardWiiJP: FUNCTION 8012b160
static TBOOL LoadGameStateImpl()
{
	if ( s_bMainGameLoaded == TTRUE )
	{
		AHackedQuickSave::GetSingleton()->SaveGame();
	}
	else if ( s_bGameStateLibraryLoaded == TTRUE )
	{
		AHackedQuickSave::GetSingleton()->SaveTime();
		AHackedQuickSave::GetSingleton()->SaveProfile();
	}

	AGameStateController::GetSingleton()->ResetStack();
	AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMusicManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->Reset();
	pSoundManager->UnloadWaveBankSamples( TPS8( music ) );
	g_oLoadScreen.Update();

	// Remove the old level
	AGameLoader::DestroyLevel();
	g_oLoadScreen.Update();

	AInstanceManager::GetSingleton()->Reset();
	g_oLoadScreen.Update();

	if ( AAniEffectManager::IsSingletonCreated() )
	{
		delete AAniEffectManager::GetSingleton();
		g_oLoadScreen.Update();
	}

	ARegrowthManager::GetSingleton()->Deactivate();
	g_oLoadScreen.Update();

	AGameSystemManager::GetSingleton()->Reset();

	// NOTE: the original calls DOES_NOTHING (005acd80) on AGUISystem here

	TRenderInterface::GetSingleton()->FlushDyingResources();
	g_oLoadScreen.Update();

	// Unload the assets of the previous state
	if ( s_bInMiniGame == TTRUE )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( AMiniGameManager::GetSingleton()->GetLibraryName( s_iCurrMiniGame ) );
		s_bInMiniGame = TFALSE;
	}
	else if ( s_bMainGameLoaded == TTRUE )
	{
		ATreasureManager::GetSingleton()->Disable();
		AAssetLoader::DestroyAssetPackOfLibrary( "lib_maingame" );
		s_bMainGameLoaded = TFALSE;
	}
	else if ( s_bStoryMode == TTRUE && s_bGameStateLibraryLoaded == TTRUE )
	{
		TPString8 strPrevState = AGameLoader::GetPrevGameStateName();
		AAssetLoader::DestroyAssetPackOfLibrary( strPrevState.GetString8().GetString() );
	}
	else if ( s_iScriptedSequence == -1 )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( "lib_frontend" );
	}
	else
	{
		UnloadScriptedSequence();
	}

	// Loading screen picture
	TPString8 strPicture;

#ifdef BARNYARD_LATEST
	// Wii: the XUI help scene of the picture. The gopher shop and the cooking show their own one.
	TINT iPicture;

	if ( s_strCurrentStateName == TPS8( AGopherShopGame ) )
		iPicture = SIMLOADINGPICTURE_GOPHER;
	else if ( s_strCurrentStateName == TPS8( ACookingMiniGame ) )
		iPicture = SIMLOADINGPICTURE_RECIPE;
	else
		iPicture = GetRandomSimLoadingPicture();

	if ( iPicture != -1 && !ShowSimLoadingXUIScene( iPicture ) )
		g_oLoadScreen.SetPicture( AGameLoader::GetSimLoadingPicture( strPicture, iPicture ), TTRUE );
#else
	if ( s_strCurrentStateName == TPS8( AGopherShopGame ) )
	{
		AGUISlideshow::LocaliseBackgroundFileName( strPicture, TPS8( bkg_SIM_gopher ) );
		g_oLoadScreen.SetPicture( strPicture, TTRUE );
	}
	else if ( s_strCurrentStateName == TPS8( ACookingMiniGame ) )
	{
		AGUISlideshow::LocaliseBackgroundFileName( strPicture, TPS8( bkg_SIM_recipe ) );
		g_oLoadScreen.SetPicture( strPicture, TTRUE );
	}
	else
	{
		const TINT iPicture = GetRandomSimLoadingPicture();

		if ( iPicture != -1 )
			g_oLoadScreen.SetPicture( AGameLoader::GetSimLoadingPicture( strPicture, iPicture ), TTRUE );
	}
#endif

	// Load the assets of the state (the library is named after the class)
	AAssetLoader::LoadAssetPackOfLibrary( s_strCurrentStateName.GetString8().GetString(), TTRUE );
	g_oLoadScreen.Update();

	ACollisionManager::GetSingleton()->CreateObjectHashMain( ACollisionManager::HashType( 0 ) );
	g_oLoadScreen.Update();

	if ( TStringManager::String8CompareNoCase( s_strCurrentStateName.GetString8().GetString(), "ACookingMiniGame", -1 ) != 0 &&
	     TStringManager::String8CompareNoCase( s_strCurrentStateName.GetString8().GetString(), "AGopherShopGame", -1 ) != 0 &&
	     TStringManager::String8CompareNoCase( s_strCurrentStateName.GetString8().GetString(), "AEndGameState", -1 ) != 0 )
	{
		const InstanceModelList& rModelList = s_aInstanceModelLists[ 0 ];
		AInstanceManager::GetSingleton()->LoadModels( 0, rModelList.iNumModels, rModelList.pModels, rModelList.bCreateCollision, rModelList.bCreateCollision, rModelList.iNumCollisionModelNodes );
		g_oLoadScreen.Update();
	}

	new AAniEffectManager();

	if ( TStringManager::String8Compare( s_strCurrentStateName.GetString8().GetString(), "ABarnExploreMinigameState", -1 ) == 0 )
		s_bBarnExploreMiniGame = TTRUE;

	AAssetLoader::CreateAssetsFromLibrary( s_strCurrentStateName.GetString8().GetString() );

	// Load music wavebank samples
	ASoundManager::ms_bShouldUpdateLoadingScreen = TTRUE;
	pSoundManager->LoadWaveBankSamples( TPS8( music ), AWaveBank::LOADFLAGS_NONE, -1 );
	ASoundManager::ms_bShouldUpdateLoadingScreen = TFALSE;

	pSoundManager->LoadSoundBankSamples( TPS8( music ) );
	g_oLoadScreen.Update();

	// NOTE: the original calls DOES_NOTHING (005acd80) on AShadowManager here

	if ( s_strCurrentStateName == TPS8( ABarnExploreMinigameState ) )
	{
		ARegrowthManager::GetSingleton()->Activate( TTRUE );
		ARegrowthManager::GetSingleton()->LoadModels();
		g_oLoadScreen.Update();
	}

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( ACamera::sm_fCurrentFOV );
	AGameSystemManager::GetSingleton()->SetUnk104( 1 );

	// Create and push the state
	const TClass* pClass = TClass::Find( s_strCurrentStateName.GetString8().GetString(), &TGetClass( AGameState ) );
	AGameState*   pState = TSTATICCAST( AGameState, pClass->CreateObject() );

	AHackedQuickSave::GetSingleton()->LoadTime();
	g_oLoadScreen.Update();

	AGameStateController::GetSingleton()->PushState( pState );
	g_oLoadScreen.Update();

	AHackedQuickSave::GetSingleton()->LoadProfile();
	g_oLoadScreen.Update();

	if ( pState->ShouldPlacePlayerAtStartGoal() == TTRUE && s_bPlacePlayerAtStartGoal )
	{
		// Place the player at the start goal
		s_bPlacePlayerAtStartGoal = TFALSE;

		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetPlayerUnit( 0 );
		TSTATICCAST( APlayerSteer, pPlayer->GetSteer() )->PlaceAtNamedGoal( s_strPlayerStartGoal, TTRUE, TTRUE );
	}

	pState->OnLoaded();
	g_oLoadScreen.Update();

	s_bGameStateLibraryLoaded = TTRUE;
	s_bLoadingSavedGame       = TFALSE;
	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
	AWeatherManager::GetSingleton()->LockToCurrentWeatherType( TFALSE );

	AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
	AMusicManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	g_oLoadScreen.ResetIndicators( TTRUE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00427e70
// NOTE: the original ends with RET 4, so it probably takes an argument it never uses
TBOOL AGameLoader::UpdateTransitionTask()
{
	if ( s_bChangingTerrain )
	{
		s_bLastTransitionUpdateResult = LoadMainGameImpl();
		if ( !s_bLastTransitionUpdateResult )
			return TFALSE;

		s_bChangingTerrain = TFALSE;
		g_oLoadScreen.Reset();
		g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );
	}
	else if ( s_bLoadingMiniGame )
	{
		s_bLastTransitionUpdateResult = LoadMiniGameImpl();

		if ( !s_bLastTransitionUpdateResult )
			return TFALSE;

		s_bLoadingMiniGame = TFALSE;
	}
	else if ( s_bLoadingGameState )
	{
		s_bLastTransitionUpdateResult = LoadGameStateImpl();
		if ( !s_bLastTransitionUpdateResult )
			return TFALSE;

		s_bLoadingGameState = TFALSE;
		g_oLoadScreen.Reset();
		g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );
	}
	else if ( s_bReturningToFrontEnd || s_iScriptedSequence != -1 )
	{
		if ( s_bReturningToFrontEnd )
		{
			s_bLastTransitionUpdateResult = ReturnToFrontEndImpl();
		}
		else
		{
			s_bLastTransitionUpdateResult = LoadScriptedSequenceImpl();
		}

		if ( !s_bLastTransitionUpdateResult )
			return TFALSE;

		s_bReturningToFrontEnd = TFALSE;
		g_oLoadScreen.Reset();
		g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );
	}
	else if ( !s_bLastTransitionUpdateResult )
	{
		return TFALSE;
	}

	if ( ATerrainInterface::IsSingletonCreated() )
	{
		ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
		pTerrain->SetCollisionModelLoadCallback( OnCollisionModelLoaded );
		pTerrain->SetOnVISGroupChangedCallback( OnVISGroupChanged );

		Debug_VerifyModelsInAssetPack( AAssetLoader::GetAssetTRB( AAssetType_AssetPack ) );
	}

	return s_bLastTransitionUpdateResult;
}

// $Barnyard: FUNCTION 00423f90
void AGameLoader::SetTransitionToMiniGame( TPString8 a_strName, TBOOL a_bStartLoading )
{
	TINT iMiniGame = AMiniGameManager::GetSingleton()->FindMiniGame( a_strName );

	if ( iMiniGame != -1 )
		SetTransitionToMiniGame( iMiniGame, a_bStartLoading );
}

// $Barnyard: FUNCTION 004239d0
void AGameLoader::SetTransitionToMiniGame( TINT a_iMiniGame, TBOOL a_bStartLoading )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( a_bStartLoading )
	{
		g_oLoadScreen.StartLoading( 100, TTRUE );

		s_pPrevGameStateClass = pController->GetCurrentState()->GetClass();
		ACameraManager::GetSingleton()->DetachCameraHelpers();
		ACameraManager::GetSingleton()->ResetHelperTargets();

		ARootTask::GetSingleton()->SetTransitioning( TTRUE );

		s_bLoadingMiniGame = TTRUE;
		s_iCurrMiniGame    = a_iMiniGame;

		AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
		APlayerManager::GetSingleton()->ResetPlayerIterator();
	}
	else
	{
		pController->SetIgnoreInput( TTRUE );
		pController->SetTransitionToMiniGame( TTRUE );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );

		s_iNextMiniGame = a_iMiniGame;
	}
}

// $Barnyard: FUNCTION 00423930
void AGameLoader::SetTransitionToMainGame( TBOOL a_bLoadedGame )
{
	g_oLoadScreen.StartLoading( 100, TTRUE );

	ACameraManager::GetSingleton()->DetachCameraHelpers();
	ACameraManager::GetSingleton()->ResetHelperTargets();

	ARootTask::GetSingleton()->SetTransitioning( TTRUE );
	s_bChangingTerrain = TTRUE;

	AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );
	AWeatherManager::GetSingleton()->LockToCurrentWeatherType( TFALSE );

	if ( a_bLoadedGame && !s_bLoadingSavedGame )
		ASimPersistData::GetSingleton()->ClearFlags( ASimPersistData::FLAGS_MAINGAME_LOADED | ASimPersistData::FLAGS_BARN_DAY_VISITED | ASimPersistData::FLAGS_BARN_NIGHT_VISITED );

	AGameStateController::GetSingleton()->ResetStack();

	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
}

// $Barnyard: FUNCTION 00424000
// $BarnyardWiiJP: FUNCTION 80128590
void AGameLoader::RestartMiniGame( TBOOL a_bImmediately )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( !a_bImmediately )
	{
		// Restart once the screen has faded out (see AGameStateController::OnUpdate)
		pController->AddFlags( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_RESTART_MINIGAME );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );
		return;
	}

	g_oLoadScreen.StartLoading( 100, TTRUE );

	AMiniGameManager* pMiniGameManager = AMiniGameManager::GetSingleton();

	// Loading screen picture
	TPString8 strStateClassName;
	TBOOL     bIsShowdown = pMiniGameManager->GetStateClassName( strStateClassName, s_iCurrMiniGame ) == TPS8( ACowTippingMiniGameState ) && pMiniGameManager->GetMiniGameVariant() == 2;

#ifdef BARNYARD_LATEST
	// Wii: the XUI load screen of the minigame (its resource is still loaded)
	TPString8 strXUILoadScreen = bIsShowdown ? TPString8( "CoyoteShowdown_LoadScreen" ) : pMiniGameManager->GetXUILoadScreenName( s_iCurrMiniGame );

	if ( strXUILoadScreen.IsEmpty() || !ShowXUILoadScreen( strXUILoadScreen, TFALSE ) )
#endif
	{
		TPString8 strPicture;
		if ( bIsShowdown )
			AGUISlideshow::LocaliseBackgroundFileName( strPicture, TPS8( bkg_Showdown ) );
		else
			pMiniGameManager->GetLoadingScreenName( strPicture, s_iCurrMiniGame );

		g_oLoadScreen.SetPicture( strPicture, TTRUE );
	}

	g_oLoadScreen.Update();

	// Recreate the minigame state
	TPString8 strClassName( pMiniGameManager->GetStateClass( s_iCurrMiniGame )->GetName() );
	pController->RecreateMiniGameState( strClassName );

	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
	g_oLoadScreen.ResetIndicators( TFALSE );

	TPString8 strLoadingScreen;
	if ( !pMiniGameManager->GetLoadingScreenName( strLoadingScreen, s_iCurrMiniGame ).IsEmpty() )
	{
		AGameState* pMiniGameState = pController->FindGameStateOnStack( strClassName );

		AMiniGameInstructionState* pInstructionState = new AMiniGameInstructionState();
		pInstructionState->SetMiniGame( pMiniGameState );
		pController->PushState( pInstructionState );
	}

	ASoundManager::GetSingleton()->Reset();
	AMusicManager::GetSingleton()->Reset();
}

// $Barnyard: FUNCTION 004247f0
void AGameLoader::SetTransitionToGameState( const TPString8& a_rcName, TBOOL a_bStartLoading )
{
	s_strCurrentStateName = a_rcName;

	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( a_bStartLoading )
	{
		g_oLoadScreen.StartLoading( 100, TTRUE );
		s_pPrevGameStateClass = pController->GetCurrentState()->GetClass();
		ARootTask::GetSingleton()->SetTransitioning( TTRUE );
		s_bLoadingGameState = TTRUE;
		pController->ResetStack();
		AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
	}
	else
	{
		// Load once the screen has faded out
		pController->AddFlags( AGameStateController::FLAGS_IGNORE_INPUT );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );
	}
}

// $Barnyard: FUNCTION 004257f0
void AGameLoader::LeaveMiniGame( TBOOL a_bImmediately )
{
	if ( !a_bImmediately )
	{
		// Leave once the screen has faded out (see AGameStateController::UpdateTransition)
		AGameStateController* pController = AGameStateController::GetSingleton();
		pController->AddFlags( AGameStateController::FLAGS_IGNORE_INPUT );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );
		return;
	}

	g_oLoadScreen.StartLoading( 100, TTRUE );
	APlayerManager::GetSingleton()->SetCurrentPlayerSet( APlayerManager::PLAYERSET_SIM );

	if ( s_bStoryMode )
	{
		// Back to the story: to the state the minigame was started from, or to the sim world
		if ( s_pPrevGameStateClass &&
		     TStringManager::String8Compare( s_pPrevGameStateClass->GetName(), "ASimState", -1 ) != 0 &&
		     TStringManager::String8Compare( s_pPrevGameStateClass->GetName(), "AReplayVariantSelectState", -1 ) != 0 &&
		     TStringManager::String8Compare( s_pPrevGameStateClass->GetName(), "AAnimalConversationState", -1 ) != 0 )
		{
			TPString8 strState( s_pPrevGameStateClass->GetName() );
			s_pPrevGameStateClass = TNULL;
			SetTransitionToGameState( strState, TTRUE );
		}
		else
		{
			s_pPrevGameStateClass = TNULL;
			SetTransitionToMainGame( TFALSE );
		}
	}
	else
	{
		// Back to the frontend
		ACameraManager::GetSingleton()->DetachCameraHelpers();
		ACameraManager::GetSingleton()->ResetHelperTargets();
		ReturnToFrontEnd( TNULL, TTRUE, TTRUE );
	}
}

// $Barnyard: FUNCTION 004237b0
void AGameLoader::SetTransitionToScriptedSequence( TINT a_iSequence )
{
	ACameraManager::GetSingleton()->DetachCameraHelpers();
	ACameraManager::GetSingleton()->ResetHelperTargets();
	g_oLoadScreen.StartLoading( 100, TTRUE );

	// Remember the state to go back to after the sequence
	const TClass* pCurrentStateClass = AGameStateController::GetSingleton()->GetCurrentState()->GetClass();

	if ( TStringManager::String8Compare( pCurrentStateClass->GetName(), "AFrontEndDebugSSeqState", -1 ) != 0 )
		s_pPrevGameStateClass = pCurrentStateClass;
	else
		s_pPrevGameStateClass = TNULL;

	// NOTE: the stack itself is reset later by DestroyAll
	AGameStateController::GetSingleton()->ResetOverlays();
	ARootTask::GetSingleton()->SetTransitioning( TTRUE );

	s_iScriptedSequence     = a_iSequence;
	s_iLastScriptedSequence = a_iSequence;

	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
}

// $Barnyard: FUNCTION 00423910
void AGameLoader::StartNewGame()
{
	SetTransitionToScriptedSequence( 0 );

	// The main game is loaded once SS01 is over (LeaveScriptedSequence)
	s_pPrevGameStateClass = &TGetClass( ASimState );
}

// $Barnyard: FUNCTION 00425410
void AGameLoader::LeaveScriptedSequence( TBOOL a_bImmediately )
{
	if ( !a_bImmediately )
	{
		// Leave once the screen has faded out (see AGameStateController::UpdateTransition)
		AGameStateController* pController = AGameStateController::GetSingleton();
		pController->AddFlags( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_LEAVE_SCRIPTED_SEQUENCE );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );
		return;
	}

	g_oLoadScreen.StartLoading( 100, TTRUE );

	// Put the gates back as they were before the sequence
	if ( AGateManager::IsSingletonCreated() && s_iSSeqGateType != -1 )
	{
		if ( s_bSSeqGateOpen )
			AGateManager::GetSingleton()->OpenGatesOfType( AGateManager::GATETYPE( s_iSSeqGateType ), TFALSE );
		else
			AGateManager::GetSingleton()->CloseGatesOfType( AGateManager::GATETYPE( s_iSSeqGateType ) );
	}

	AGameStateController::GetSingleton()->ResetStack();

	if ( !s_pPrevGameStateClass )
	{
		// Back to the frontend
		ReturnToFrontEnd( TNULL, TTRUE, TTRUE );
		return;
	}

	const TCHAR* szPrevState = s_pPrevGameStateClass->GetName();

	if ( TStringManager::String8Compare( szPrevState, "ACarChaseMiniGameState", -1 ) == 0 )
	{
		// The car chase is started from its intro sequence
		AMiniGameManager::GetSingleton()->SetMiniGameVariant( 5 );
		SetTransitionToMiniGame( AMiniGameManager::GetSingleton()->GetNextMiniGameIndex(), TTRUE );
		s_pPrevGameStateClass = &TGetClass( ASimState );
	}
	else if ( TStringManager::String8Compare( szPrevState, "ASimState", -1 ) != 0 && TStringManager::String8Compare( szPrevState, "AReplayVariantSelectState", -1 ) != 0 && TStringManager::String8Compare( szPrevState, "AAnimalConversationState", -1 ) != 0 )
	{
		// Back to the state the sequence was started from
		TPString8 strState( szPrevState );
		s_pPrevGameStateClass = TNULL;

		SetTransitionToGameState( strState, TTRUE );
	}
	else
	{
		// Back to the sim
		s_pPrevGameStateClass = TNULL;
		SetTransitionToMainGame( TFALSE );
	}
}

// $Barnyard: FUNCTION 00425980
void AGameLoader::UnloadForExit()
{
	if ( ASimPersistData::IsSingletonCreated() )
		ASimPersistData::GetSingleton()->Reset();

	if ( AGameStateController::IsSingletonCreated() )
		AGameStateController::GetSingleton()->ResetStack();

	if ( AHudManager::IsSingletonCreated() )
		AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	if ( AMusicManager::IsSingletonCreated() )
	{
		AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
		AMusicManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	}

	if ( ASoundManager::IsSingletonCreated() )
		ASoundManager::GetSingleton()->Reset();

	g_oLoadScreen.StartLoading( 100, TTRUE );

	if ( ACameraManager::IsSingletonCreated() && ACameraManager::GetSingleton()->GetCurrentCameraHelper() )
	{
		ACameraManager* pCameraManager = ACameraManager::GetSingleton();

		if ( pCameraManager->GetCurrentCameraHelper()->IsA( &TGetClass( ABoneAttachCameraHelper ) ) )
		{
			ABoneAttachCameraHelper* pBoneAttachHelper = TSTATICCAST( ABoneAttachCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_BONEATTACH ) );
			pBoneAttachHelper->Reset();
			pBoneAttachHelper->OnDetach();
		}
	}

	DestroyLevel();

	if ( AInstanceManager::IsSingletonCreated() )
		AInstanceManager::GetSingleton()->Reset();

	if ( ARegrowthManager::IsSingletonCreated() )
		ARegrowthManager::GetSingleton()->Deactivate();

	if ( ABikeManager::IsSingletonCreated() )
	{
		ABikeManager::GetSingleton()->DestroyBikeAssets();
		ABikeManager::GetSingleton()->Disable();
	}

	if ( AGameSystemManager::IsSingletonCreated() )
		AGameSystemManager::GetSingleton()->Reset();

	// NOTE: the original calls DOES_NOTHING (005acd80) on AGUISystem here

	if ( TRenderInterface::GetSingleton() )
		TRenderInterface::GetSingleton()->FlushDyingResources();

	g_oLoadScreen.Update( 1.0f, TFALSE );

	// Unload the assets of the current state
	if ( s_bInMiniGame == TTRUE )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( AMiniGameManager::GetSingleton()->GetLibraryName( s_iCurrMiniGame ) );
		s_bInMiniGame = TFALSE;
	}
	else if ( s_bGameStateLibraryLoaded == TTRUE )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( s_strCurrentStateName.GetString8().GetString() );
		s_bGameStateLibraryLoaded = TFALSE;
		s_strCurrentStateName     = TPString8();
	}

	if ( s_bMainGameLoaded == TTRUE )
	{
		ATreasureManager::GetSingleton()->Disable();
		AAssetLoader::DestroyAssetPackOfLibrary( "lib_maingame" );
	}
	else
	{
		AAssetLoader::DestroyAssetPackOfLibrary( "lib_frontend" );
	}

	if ( ASoundManager::IsSingletonCreated() )
	{
		ASoundManager::GetSingleton()->UnloadSoundBank( TPS8( music ), TFALSE );
		ASoundManager::GetSingleton()->UnloadSoundBank( TPS8( ui ), TFALSE );
	}
}

// $Barnyard: FUNCTION 0042ab30
void AGameLoader::ResetSimLoadingPictures()
{
	s_uiSimLoadingPictures = 1;
}

// Loads the scripted sequence s_iScriptedSequence: its library and terrain, then pushes ASSeqState
// $Barnyard: FUNCTION 00427690
// $BarnyardWiiJP: FUNCTION 8012c76c
static TBOOL LoadScriptedSequenceImpl()
{
	AGameStateController::GetSingleton()->ResetStack();
	AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	g_oLoadScreen.Update();

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMusicManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->Reset();
	pSoundManager->UnloadWaveBankSamples( TPS8( music ) );
	g_oLoadScreen.Update();

	if ( s_bStoryMode == TTRUE && s_bInMiniGame == TFALSE )
	{
		// Leaving the story, save the world state first
		if ( s_bGameStateLibraryLoaded == TTRUE )
		{
			AHackedQuickSave::GetSingleton()->SaveTime();
			AHackedQuickSave::GetSingleton()->SaveProfile();
		}
		else
		{
			AHackedQuickSave::GetSingleton()->SaveGame();
		}

		AMiniGameManager::GetSingleton()->ClearNextMiniGame();
		g_oLoadScreen.Update();
	}

	// Remove the old level
	AGameLoader::DestroyLevel();
	g_oLoadScreen.Update();

	AInstanceManager::GetSingleton()->Reset();
	g_oLoadScreen.Update();

	TRenderInterface::GetSingleton()->FlushDyingResources();

	if ( AAniEffectManager::IsSingletonCreated() )
	{
		delete AAniEffectManager::GetSingleton();
		g_oLoadScreen.Update();
	}

	ARegrowthManager::GetSingleton()->Deactivate();
	g_oLoadScreen.Update();

	AGameSystemManager::GetSingleton()->Reset();
	g_oLoadScreen.Update();

	// NOTE: the original calls DOES_NOTHING (005acd80) on AGUISystem here

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	ACameraHelper*  pCameraHelper  = pCameraManager->GetCurrentCameraHelper();

	if ( pCameraHelper && pCameraHelper->IsA( &TGetClass( ABoneAttachCameraHelper ) ) )
		TSTATICCAST( ABoneAttachCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_BONEATTACH ) )->Reset();

	g_oLoadScreen.Update();

	// Unload the assets of the previous state
	if ( s_bInMiniGame == TTRUE )
	{
		AAssetLoader::DestroyAssetPackOfLibrary( AMiniGameManager::GetSingleton()->GetLibraryName( s_iCurrMiniGame ) );
		s_bInMiniGame = TFALSE;
	}
	else if ( s_bStoryMode == TTRUE )
	{
		if ( s_bGameStateLibraryLoaded == TFALSE )
		{
			ATreasureManager::GetSingleton()->Disable();
			AAssetLoader::DestroyAssetPackOfLibrary( "lib_maingame" );
			AUnitManager::GetSingleton()->UnloadLibrary( "lib_maingame" );
		}
		else if ( s_bGameStateLibraryLoaded == TTRUE )
		{
			AAssetLoader::DestroyAssetPackOfLibrary( s_strCurrentStateName.GetString8().GetString() );
			s_bGameStateLibraryLoaded = TFALSE;
			s_strCurrentStateName     = TPString8();
		}
	}
	else if ( s_bStoryMode == TFALSE && s_bGameStateLibraryLoaded == TFALSE )
	{
		// Coming from the frontend
		AAssetLoader::DestroyAssetPackOfLibrary( "lib_frontend" );
		TRenderInterface::GetSingleton()->FlushDyingResources();
		TRenderInterface::GetSingleton()->FlushDyingResources();
	}

	// Load the sequence
	const AScriptedSequenceActivity::SequenceInfo& rSequence = s_aScriptedSequences[ s_iScriptedSequence ];

	TString8 strLibrary( "ScriptedSequence/" );
	strLibrary += rSequence.szName;

	AAssetLoader::LoadAssetPackOfLibrary( strLibrary.GetString(), TTRUE );
	g_oLoadScreen.Update();

	const TINT iModelList = rSequence.iInstanceModels;
	ACollisionManager::GetSingleton()->CreateObjectHashMain( ACollisionManager::HashType( iModelList ) );
	g_oLoadScreen.Update();

	const InstanceModelList& rModelList = s_aInstanceModelLists[ iModelList ];
	AInstanceManager::GetSingleton()->LoadModels( iModelList, rModelList.iNumModels, rModelList.pModels, rModelList.bCreateCollision, rModelList.bCreateCollision, rModelList.iNumCollisionModelNodes );
	g_oLoadScreen.Update();

	AGameLoader::Terrain_Set( rSequence.eTerrain, TTRUE, TTRUE, 0, 0, 0, -1 );

	// Stream the terrain from its "Dummy" section
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	TINT               iSection = pTerrain->GetSectionIndex( "Dummy" );

	if ( iSection == -1 )
	{
		pTerrain->ForceSection( 0, TFALSE );
		s_iSSeqVISGroup = 0;
	}
	else
	{
		pTerrain->ForceSection( iSection, TFALSE );
		s_iSSeqVISGroup = iSection;
	}

	TBOOL bResetVISGroup = TFALSE;

	if ( s_iScriptedSequence == 5 )
	{
		pTerrain->SetGetCurrentVISGroupCallback( GetSSeqVISGroup );
		bResetVISGroup = TTRUE;
	}
#ifdef BARNYARD_LATEST
	else if ( s_iScriptedSequence == AScriptedSequenceActivity::SEQUENCE_SSUL_MAIN )
	{
		// The tomato launcher sequence (from the debug menu) streams the main terrain from the hilltop
		iSection = pTerrain->GetSectionIndex( "Hilltop" );
		pTerrain->ForceSection( iSection, TTRUE );
		s_iSSeqVISGroup = iSection;

		pTerrain->SetGetCurrentVISGroupCallback( GetSSeqVISGroup );
		bResetVISGroup = TTRUE;
	}
#endif // BARNYARD_LATEST

	AGameLoader::Terrain_Load();

	if ( bResetVISGroup )
		s_iSSeqVISGroup = -1;

	AGameTimeManager::GetSingleton()->PushUpdateState();
	g_oLoadScreen.Update();

	AAssetLoader::CreateAssetsFromLibrary( strLibrary.GetString() );
	g_oLoadScreen.Update();

	// Game time FX of the sequence
	TBOOL bForcedFX = TFALSE;

	if ( AGameTimeFXManager::IsSingletonCreated() && rSequence.iTimeFX != -1 )
	{
		AGameTimeFXManager* pFXManager = AGameTimeFXManager::GetSingleton();
		pFXManager->SetForcedFX( pFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE( rSequence.iTimeFXGroup ), AGameTimeManager::DAYPHASE( rSequence.iTimeFX ) ) );
		bForcedFX = TTRUE;
	}

	s_iSSeqGateType = -1;

	if ( AGateManager::IsSingletonCreated() && rSequence.iGate != -1 )
	{
		AGateManager::Gate* pGate = AGateManager::GetSingleton()->FindGate( AGateManager::GATETYPE( rSequence.iGate ), 0 );

		if ( rSequence.bRestoreGate )
		{
			s_iSSeqGateType = rSequence.iGate;
			s_bSSeqGateOpen = pGate->bOpen;
		}

		if ( rSequence.bGateState )
			AGateManager::GetSingleton()->OpenGatesOfType( AGateManager::GATETYPE( rSequence.iGate ), TFALSE );
		else
			AGateManager::GetSingleton()->CloseGatesOfType( AGateManager::GATETYPE( rSequence.iGate ) );
	}

	ASSeqState* pSSeqState = new ASSeqState( rSequence.szName, rSequence.szScript, rSequence.szCamera, TVector4( 0.0f, 0.0f, 0.0f, 1.0f ), rSequence.iSyncType, TFALSE, TTRUE, TFALSE, TFALSE, bForcedFX, rSequence.bShowChapterTitle, TTRUE, TFALSE );
	pSSeqState->SetCanSkip( rSequence.bCanSkip );
	AGameStateController::GetSingleton()->PushState( pSSeqState );

	AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY( 2 ) );
	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( ACamera::sm_fCurrentFOV );
	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );

	g_oLoadScreen.ResetIndicators( TTRUE );
	g_oLoadScreen.Reset();
	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );
	ARenderer::GetSingleton()->ForceUpdate30FPS();

	return TTRUE;
}

// $Barnyard: FUNCTION 004236e0
TBOOL AGameLoader::SaveProgressInfo( TINT* a_pOutInfo )
{
	a_pOutInfo[ 0 ] = APlayerProfileManager::GetSingleton()->GetMainGender();
	a_pOutInfo[ 1 ] = APlayerProfileManager::GetSingleton()->GetMainCostume();
	a_pOutInfo[ 2 ] = ASimPersistData::GetSingleton()->GetAct();
	a_pOutInfo[ 3 ] = s_uiSimLoadingPictures;

	return TTRUE;
}

// $Barnyard: FUNCTION 00423c70
void AGameLoader::LoadSavedGame( TBOOL a_bNow )
{
	if ( !a_bNow )
	{
		// Load once the screen has faded out (see AGameStateController::UpdateTransition)
		AGameStateController* pController = AGameStateController::GetSingleton();
		pController->AddFlags( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_LOAD_SAVED_GAME | AGameStateController::FLAGS_FADE_SOUND );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );
		return;
	}

	s_bLoadingSavedGame = TTRUE;

	// NOTE: the original doesn't check for a missing slot here
	APlayerProgress::SaveSlot* pSlot = APlayerProgress::GetSingleton()->GetCurrentSlot();

	AHackedQuickSave::GetSingleton()->LoadFromMemory( pSlot->aQuickSave, sizeof( pSlot->aQuickSave ) );
	AHintManager::LoadState( pSlot->aHints, TREINTERPRETCAST( void*, sizeof( pSlot->aHints ) ) );

	AHighScoreManager::Load( pSlot->aHighScores, sizeof( pSlot->aHighScores ) );

	APlayerProfileManager::GetSingleton()->SetMainGender( pSlot->iGender );
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetCostume( pSlot->iCostume );
	s_uiSimLoadingPictures = pSlot->uiSimLoadingPictures;
	ASimPersistData::GetSingleton()->SetTerrainVIS( -1 );
	ASimPersistData::GetSingleton()->SetAct( pSlot->iAct );

	SetTransitionToMainGame( TTRUE );
}

// $Barnyard: FUNCTION 00423de0
void AGameLoader::ReturnToFrontEnd( const TCHAR* a_szStateName, TBOOL a_bResetPlayerSet, TBOOL a_bImmediately )
{
	if ( !a_bImmediately )
	{
		// Return once the screen has faded out (see AGameStateController::UpdateTransition)
		AGameStateController* pController = AGameStateController::GetSingleton();
		pController->AddFlags( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_RETURN_TO_FRONTEND );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );
		return;
	}

	APlayerProfileManager::GetSingleton()->GetMainProfile()->Reset();

	AReplayManager::GetSingleton()->Reset();

	ARootTask::GetSingleton()->SetTransitioning( TTRUE );
	s_bReturningToFrontEnd = TTRUE;

	AGameStateController::GetSingleton()->ResetStack();
	g_oLoadScreen.StartLoading( 100, TTRUE );
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );

	if ( a_bResetPlayerSet == TTRUE )
		APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_MINIGAME )->Reset();

	AReplayManager::GetSingleton()->ResetAllUnlocked();
	ASimPersistData::GetSingleton()->Reset();
	AAnimalPopulationManager::GetSingleton()->LoadSpawnData();

	AInteractionManager::GetSingleton()->ResetForcedRequirements();
	AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );
	AWeatherManager::GetSingleton()->LockToCurrentWeatherType( TTRUE );

	AGameTimeManager::GetSingleton()->ChangePhaseOfDay( AGameTimeManager::DAYPHASE( 0 ), TTRUE, TTRUE );

	s_bPlacePlayerAtStartGoal = TFALSE;
	s_bSimBikeStored          = TFALSE;
	s_bStoryMode              = TFALSE;
	s_bWasBarnExploreMiniGame = TFALSE;

	if ( a_szStateName )
		s_strCurrentStateName = TPString8( a_szStateName );
}

TBOOL AGameLoader::IsInMiniGame()
{
	return s_bInMiniGame;
}

TINT AGameLoader::GetLastScriptedSequence()
{
	return s_iLastScriptedSequence;
}

void AGameLoader::ResetLastScriptedSequence()
{
	s_iLastScriptedSequence = -1;
}

TBOOL AGameLoader::WasBarnExploreMiniGame()
{
	return s_bWasBarnExploreMiniGame;
}

void AGameLoader::ResetWasBarnExploreMiniGame()
{
	s_bWasBarnExploreMiniGame = TFALSE;
}

TBOOL AGameLoader::IsLoadingSavedGame()
{
	return s_bLoadingSavedGame;
}

TINT AGameLoader::GetCurrMiniGame()
{
	return s_iCurrMiniGame;
}

TINT AGameLoader::GetNextMiniGame()
{
	return s_iNextMiniGame;
}

TBOOL AGameLoader::IsChangingTerrain()
{
	return s_bChangingTerrain;
}

TBOOL AGameLoader::IsBarnExploreMiniGame()
{
	return s_bBarnExploreMiniGame;
}

void AGameLoader::SetBarnExploreMiniGame( TBOOL a_bBarnExploreMiniGame )
{
	s_bBarnExploreMiniGame = a_bBarnExploreMiniGame;
}

TBOOL AGameLoader::ShouldPlacePlayerAtStartGoal()
{
	return s_bPlacePlayerAtStartGoal;
}

#ifdef BARNYARD_LATEST
TBOOL AGameLoader::IsPigApplePieReturn()
{
	return s_bPigApplePieReturn;
}

void AGameLoader::SetPigApplePieReturn( TBOOL a_bReturn )
{
	s_bPigApplePieReturn = a_bReturn;
}
#endif

// $Barnyard: FUNCTION 004257b0
void AGameLoader::SetPlayerStartGoal( const TPString8& a_rcGoal )
{
	s_strPlayerStartGoal      = a_rcGoal;
	s_bPlacePlayerAtStartGoal = TTRUE;
}

void AGameLoader::SetPrevGameStateClass( const TClass* a_pClass )
{
	s_pPrevGameStateClass = a_pClass;
}

TBOOL AGameLoader::IsStoryMode()
{
	return s_bStoryMode;
}

void AGameLoader::SetStoryMode( TBOOL a_bStoryMode )
{
	s_bStoryMode = a_bStoryMode;
}

TBOOL AGameLoader::IsMainGameLoaded()
{
	return s_bMainGameLoaded;
}

TBOOL AGameLoader::IsGameStateLibraryLoaded()
{
	return s_bGameStateLibraryLoaded;
}

TBOOL AGameLoader::IsReturningToFrontEnd()
{
	return s_bReturningToFrontEnd;
}

TINT AGameLoader::GetScriptedSequence()
{
	return s_iScriptedSequence;
}

// $Barnyard: FUNCTION 004232d0
TINT AGameLoader::GetTerrainVIS()
{
	return s_iTerrainVISIndex;
}

AGameLoader::Terrain AGameLoader::GetCurrentTerrain()
{
	return s_eCurrentLevel;
}

// $Barnyard: FUNCTION 006c1e80
void AGameLoader::Debug_VerifyModelsInAssetPack( Toshi::TTRB* a_pTRB )
{
	// Empty in retail, probably only did something in debug builds
	if ( !a_pTRB ) return;

	TCHAR      szBuffer[ 256 ];
	const TINT iSkeletonStrLength = TStringManager::String8Length( "_Skeleton" );

	for ( TINT i = 0; i < a_pTRB->GetNumSymbols(); i++ )
	{
		TTRB::TTRBSymbol* pSymbol           = a_pTRB->GetSymbol( i );
		const TCHAR*      pchSymbolName     = a_pTRB->GetSymbolName( pSymbol );
		const TINT        iSymbolNameLength = TStringManager::String8Length( a_pTRB->GetSymbolName( pSymbol ) );

		// Check string postfix is _Skeleton
		if ( iSymbolNameLength > iSkeletonStrLength )
		{
			const TINT iPostFixPosition = ( iSymbolNameLength - iSkeletonStrLength );

			if ( 0 == Toshi::TStringManager::String8Compare( pchSymbolName + iPostFixPosition, "_Skeleton" ) )
			{
				const TINT iModelNameSize = iPostFixPosition + 1;
				TStringManager::String8Copy( szBuffer, pchSymbolName, iModelNameSize );
				szBuffer[ iModelNameSize ] = '\0';

				T2_FOREACH( TModelManager::ms_oUsedList, it )
				{
					TModel* pModel = it->GetModel();

					if ( pModel->m_bIsAssetFile &&
					     pModel->m_szSymbolPrefixLength == iModelNameSize &&
					     TStringManager::String8Compare( pModel->m_szSymbolPrefix, szBuffer, iModelNameSize ) == 0 )
					{
						break;
					}
				}

				// AND YES IT DOES NOTHING AFTER ALL
			}
		}
	}

	// This whole method is genius omg
	T2_FOREACH( TModelManager::ms_oUsedList, it )
	{
#ifdef TOSHI_DEBUG
		// something goes here...?
#endif
	}
}

// $Barnyard: FUNCTION 00423a80
void AGameLoader::Terrain_Set( TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup )
{
	if ( a_eTerrain == s_eCurrentLevel ) return;

	// Destroy the golf flags of the main game
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	if ( AAnimatableObjectType* pGolfFlagType = pAnimObjManager->FindType( TPS8( sim_golf_flag ) ) )
	{
		pAnimObjManager->DestroyAnimatableObjectsOfType( pGolfFlagType );
		pAnimObjManager->DestroyType( TPS8( sim_golf_flag ) );
		AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPS8( sim_golf_flag ) );
		AMaterialLibraryManager::GetSingleton()->UnrefLibrary( TPS8( sim_golf_flag ), TTRUE );
	}

	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );

	if ( s_pCurrentTerrain )
		delete s_pCurrentTerrain;

	s_pCurrentTerrain = TNULL;
	TRenderInterface::GetSingleton()->FlushDyingResources();

	if ( a_eTerrain == Terrain_FrontEnd )
	{
		a_iStartVISGroup = 0;
	}

	s_pCurrentTerrain = new ATerrainInterface(
	    a_iUnused1,
	    a_iUnused2,
	    a_iPreloadTerrainBlockSize < 1 ? 0x2200000 : a_iPreloadTerrainBlockSize,
	    a_iStartVISGroup
	);

	T2FormatString128 terrainFile;
	terrainFile.Format( "Data\\Terrain\\%s\\%s.trb", GetTerrainName( a_eTerrain ), GetTerrainName( a_eTerrain ) );

	s_pCurrentTerrain->LoadFromFile( terrainFile.Get(), TTRUE, a_bPersistantCollision );
	s_eCurrentLevel = a_eTerrain;
	AGameTimeFXManager::GetSingleton()->Reset();

	if ( !a_bLoadLater ) Terrain_Load();

	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
}

// $Barnyard: FUNCTION 00423850
void AGameLoader::Terrain_Load()
{
	TPROFILER_SCOPE();

	auto pTerrain         = ATerrainInterface::GetSingleton();
	pTerrain->m_bIsLoaded = TFALSE;

	if ( AGameLoader::IsChangingTerrain() )
		pTerrain->m_fnGetCurrentVISGroup = AGameLoader::GetTerrainVIS;

	pTerrain->Update();
	while ( !pTerrain->IsLoaded() )
	{
		TRenderInterface::GetSingleton()->Update( 0.01f );
		AAssetStreaming::GetSingleton()->Update();

#ifndef BARNYARD_COMMUNITY_PATCH
		ThreadSleep( 20 );
#endif // BARNYARD_COMMUNITY_PATCH

		pTerrain->Update();
		g_oLoadScreen.Update();
	}

	pTerrain->m_bIsLoaded = TTRUE;

	if ( AGameLoader::IsChangingTerrain() )
		pTerrain->m_fnGetCurrentVISGroup = ATerrainInterface::GetSectionPlayerIsIn;
}


// $Barnyard: FUNCTION 00427c40
// $BarnyardWiiJP: FUNCTION 80127e30
void AGameLoader::LoadFrontEnd()
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	// Load ui & music soundbanks
	pSoundManager->LoadSoundBank( TPS8( ui ), TFALSE, TTRUE );
	pSoundManager->LoadSoundBank( TPS8( music ), TFALSE, TFALSE );

#ifdef BARNYARD_LATEST
	// Wii: the backdrop of the XUI load screens (Data/Matlibs/bkg_Background.ttl), it stays loaded
#  ifdef BARNYARD_COMMUNITY_PATCH
	// The Wii data is optional on PC
	if ( TFile* pFile = TFile::Create( "Data/Matlibs/bkg_Background.ttl", TFILEMODE_READ ) )
	{
		pFile->Destroy();
		AGUISystem::GetSingleton()->SetBackgroundPicture( TPString8( "bkg_Background" ) );
	}
#  else
	AGUISystem::GetSingleton()->SetBackgroundPicture( TPString8( "bkg_Background" ) );
#  endif
#endif

	AGameLoader::Debug_VerifyModelsInAssetPack( AAssetLoader::GetAssetTRB( AAssetType_Startup ) );

	AAssetLoader::LoadAssetPackOfLibrary( "lib_frontend", TTRUE );
	ACollisionManager::GetSingleton()->CreateObjectHashMain( ACollisionManager::HashType_UNK2 );
	AInstanceManager::GetSingleton()->LoadModels( 1, s_iNumFrontEndInstanceModels, s_aFrontEndInstanceModels, TFALSE, TFALSE, s_iNumFrontEndCollModelNodes );

	AGameLoader::Terrain_Set( AGameLoader::Terrain_FrontEnd, TTRUE, TTRUE, 0, 0, 0, 0 );
	AGameLoader::Terrain_Load();
	AAssetLoader::CreateAssetsFromLibrary( "lib_frontend" );

	// Load music wavebank samples
	ASoundManager::ms_bShouldUpdateLoadingScreen = TTRUE;
	pSoundManager->LoadWaveBankSamples( TPS8( music ), AWaveBank::LOADFLAGS_NONE, -1 );
	ASoundManager::ms_bShouldUpdateLoadingScreen = TFALSE;

	// Load music samples
	pSoundManager->LoadSoundBankSamples( TPS8( music ) );

	AGameTimeManager::GetSingleton()->PopUpdateState();
	AGameTimeFXManager::GetSingleton()->SetDayPhase( AGameTimeManager::DAYPHASE_DAWN, TFALSE, 0.0f );
	AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );
	AWeatherManager::GetSingleton()->LockToCurrentWeatherType( TTRUE );

	// Create front end state
	AFrontEndMainMenuState2* pFrontEndState = new AFrontEndMainMenuState2();
	AGameLoader::Terrain_Set( AGameLoader::Terrain_FrontEnd, TFALSE, TTRUE, 0, 0, 0, 0 );
	pFrontEndState->SetupCamera();

	AGameLoader::Terrain_Load();

	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devattract" starts at the title screen that retail never creates
	if ( TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devattract" ) )
	{
		delete pFrontEndState;
		pFrontEndState = TNULL;
		AGameStateController::GetSingleton()->PushState( new AFrontEndAttractState() );
	}
	else
#endif
		// Push the state
		AGameStateController::GetSingleton()->PushState( pFrontEndState );
	AGameLoader::Debug_VerifyModelsInAssetPack( AAssetLoader::GetAssetTRB( AAssetType_AssetPack ) );

	// The barn doors of the frontend are open
	if ( !AGateManager::IsSingletonCreated() )
		new AGateManager();

	AGateManager::GetSingleton()->OpenGatesOfType( AGateManager::GATETYPE_BARN, TFALSE );
	AGateManager::GetSingleton()->OpenGatesOfType( AGateManager::GATETYPE_NIGHTBARN, TFALSE );

	// Hide loading screen
	g_oLoadScreen.Reset();
	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );

	// Fade screen from black
	AFadeManager::GetSingleton()->StartFade(
	    AFade::Color( 255, 0, 0, 0 ),
	    AFade::Color( 0, 0, 0, 0 ),
	    1.0f
	);
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Returns the value of "-minigame <name>" from the command line (empty if it's not there)
static const TCHAR* Dev_GetMiniGameArg( TCHAR* a_szOutName, TINT a_iSize )
{
	a_szOutName[ 0 ] = '\0';

	const TCHAR* szArg = TStringManager::String8FindString( GetCommandLineA(), "-minigame " );
	if ( !szArg )
		return a_szOutName;

	szArg += TStringManager::String8Length( "-minigame " );

	TINT iLength = 0;
	while ( szArg[ iLength ] != '\0' && szArg[ iLength ] != ' ' && iLength < a_iSize - 1 )
	{
		a_szOutName[ iLength ] = szArg[ iLength ];
		iLength++;
	}

	a_szOutName[ iLength ] = '\0';
	return a_szOutName;
}

// The micro games of the sim aren't minigames of AMiniGameManager: the sim starts them (a lever, a kick, an item...).
// "-minigame <micro game>" plays them in a new game of the sim, through the dev switches that start each of them
struct DevMicroGame
{
	const TCHAR* szName;
	const TCHAR* szSwitches; // TNULL: added by name to the sim 8 seconds after it has started (CreateAndAddChildState)
};

static const DevMicroGame s_aDevMicroGames[] = {
	{ "ACoinPickupMicroGame", "-devgoldrush" },
	{ "AShakeTreeMicroGame", "-devshake tree" },
	{ "AMilkCanMicroGame", "-devnightbarn -devshake milkcan" },
	{ "ATreasueHuntMicroGame", "-devtreasure" },
	{ "ABarnExploreMinigameState", "-devnightbarn -devdayphase 4" },
	{ "ACookingMiniGame", "-devnightbarn -devdayphase 4 -devcocktails 4" },
	{ "ASuperSquirtMicroGame", TNULL },
	{ "AFarmerTrainMicroGame", TNULL },
};

// The micro game of "-minigame", TNULL if there's none
static const DevMicroGame* Dev_GetMicroGame()
{
	TCHAR szName[ 64 ];
	Dev_GetMiniGameArg( szName, sizeof( szName ) );

	for ( TINT i = 0; i < TARRAYSIZE( s_aDevMicroGames ); i++ )
	{
		if ( TStringManager::String8CompareNoCase( s_aDevMicroGames[ i ].szName, szName ) == 0 )
			return &s_aDevMicroGames[ i ];
	}

	return TNULL;
}

const TCHAR* AGameLoader::Dev_GetCommandLine()
{
	static TString8 s_strCommandLine;
	static TBOOL    s_bCreated = TFALSE;

	if ( !s_bCreated )
	{
		s_bCreated       = TTRUE;
		s_strCommandLine = GetCommandLineA();

		if ( const DevMicroGame* pMicroGame = Dev_GetMicroGame() )
		{
			s_strCommandLine += " -simstate";

			if ( pMicroGame->szSwitches )
			{
				s_strCommandLine += " ";
				s_strCommandLine += pMicroGame->szSwitches;
			}
		}
	}

	return s_strCommandLine;
}

// Returns TTRUE if a_szSwitch is on the command line
static TBOOL Dev_HasSwitch( const TCHAR* a_szSwitch, const TCHAR** a_ppOutArgs = TNULL )
{
	// The whole switch ("-devsave" isn't "-devsavefile"), a_ppOutArgs receives what follows it
	const TINT   iLength = TStringManager::String8Length( a_szSwitch );
	const TCHAR* szFound = AGameLoader::Dev_GetCommandLine();

	while ( ( szFound = TStringManager::String8FindString( szFound, a_szSwitch ) ) != TNULL )
	{
		if ( szFound[ iLength ] == '\0' || szFound[ iLength ] == ' ' || szFound[ iLength ] == '"' )
		{
			if ( a_ppOutArgs )
				*a_ppOutArgs = szFound + iLength;

			return TTRUE;
		}

		szFound += iLength;
	}

	return TFALSE;
}

//-----------------------------------------------------------------------------
// Development: "-devtier <act> <tier>" starts a new game (like "-simstate") at that act and tier, in the state
// the story leaves it when the tier is reached by playing: the quests of the earlier tiers are completed and
// their lasting rewards given. "-devadvancetier <n>" is the same, at the tier n tiers after the first one.
//-----------------------------------------------------------------------------

// What the quests of the story give for good (items, unlocks), applied for the quests of the tiers before the one
// "-devtier" starts at. From the activities of Data/Interactions. Only the gifts that stay with the player are
// listed, not the items a quest takes back.
struct DevTierQuestReward
{
	const TCHAR* szQuest;
	const TCHAR* szItem;        // gift (genitem), TNULL for none
	TBOOL        bPlayerSquirt; // amiscfunctions::enableplayersquirt( true )
	const TCHAR* szLatch;       // ainteractionmanager::setpersistentlatch( name, 1 )
	const TCHAR* szGate;        // agatemanager::setgate( name, true )
	TBOOL        bSuperSquirt;  // amiscfunctions::enablesupersquirtparticles( true )
};

static const DevTierQuestReward s_aDevTierQuestRewards[] = {
	{ "Pig Apple Pie", "cookbook", TFALSE, TNULL, TNULL, TFALSE },
	{ "Pig Apple Pie", "cocktails", TFALSE, TNULL, TNULL, TFALSE },
	{ "Pip Barnyard Secrets", "sunglasses", TTRUE, TNULL, TNULL, TFALSE },
	{ "Otis Barnyard Champion", "scorebook", TFALSE, TNULL, TNULL, TFALSE },
	{ "Otis Barnyard Champion", "bikekey", TFALSE, TNULL, TNULL, TFALSE },
	{ "Root (Night) Barn Detailing", "goldrush_token", TFALSE, "gotcash", TNULL, TFALSE },
	{ "Root Barn Rewards", "milkweed_seed", TFALSE, TNULL, TNULL, TFALSE },
	{ "HANNA Honey Collecting", "honey_gate_key", TFALSE, TNULL, TNULL, TFALSE },
	{ "Ben Coyote Warning", "woods_gate_key", TFALSE, TNULL, TNULL, TFALSE },
	{ "OTIS Flowers For Daisy", "metaldetector", TFALSE, TNULL, TNULL, TFALSE },
	{ "Otis Super Squirt", "golf_gate_key", TFALSE, TNULL, TNULL, TTRUE },
	{ "Ben Tomato Launcher", TNULL, TFALSE, TNULL, "tomato", TFALSE },
	{ "Miles Junkyard Scores", "goldrush_token", TFALSE, TNULL, TNULL, TFALSE },
	{ "Miles Raccoon Sabotage", "goldrush_token", TFALSE, TNULL, TNULL, TFALSE },
	{ "Miles Raccoon Sabotage 2", "goldrush_token", TFALSE, TNULL, TNULL, TFALSE },
	{ "Barnyard Hero", "junkyard_gate_key", TFALSE, TNULL, TNULL, TFALSE },
};

// The act and tier of "-devtier" / "-devadvancetier", TFALSE if there's neither
static TBOOL Dev_GetDevTier( TINT& a_rAct, TINT& a_rTier )
{
	const TCHAR* szArgs;

	if ( Dev_HasSwitch( "-devtier", &szArgs ) )
	{
		a_rAct  = 1;
		a_rTier = 1;
		sscanf( szArgs, " %d %d", &a_rAct, &a_rTier );
	}
	else if ( Dev_HasSwitch( "-devadvancetier", &szArgs ) )
	{
		// The tier n tiers after the first one (the numbers of tiers of the acts are those of tier_data)
		TINT iNumTiers = 0;
		sscanf( szArgs, " %d", &iNumTiers );

		static const TINT s_aiNumTiers[] = { 16, 20, 18 };
		TINT              iAbsTier       = TMath::Max( iNumTiers, 0 ) + 1;

		a_rAct = 1;

		while ( a_rAct < 3 && iAbsTier > s_aiNumTiers[ a_rAct - 1 ] )
			iAbsTier -= s_aiNumTiers[ a_rAct++ - 1 ];

		a_rTier = iAbsTier;
	}
	else
	{
		return TFALSE;
	}

	a_rAct  = TMath::Min( TMath::Max( a_rAct, 1 ), 3 );
	a_rTier = TMath::Max( a_rTier, 1 );
	return TTRUE;
}

// Called before a new main game is loaded: the act decides which assets the main game loads
static void Dev_StartDevTierGame()
{
	TINT iAct, iTier;

	if ( !Dev_GetDevTier( iAct, iTier ) )
		return;

	// The assets of the main game depend on the act (AAssetLoader::GetLibraryFileName), the same way loading a save
	// sets the act of its slot first (AGameLoader::LoadSavedGame)
	ASimPersistData::GetSingleton()->SetAct( iAct );
	TINFO( "[devtier] new game at act %d tier %d\n", iAct, iTier );
}

// Completes the quests of the tier a_iAbsTier the way the story leaves them once they're done: the completed
// quests of the tier and the completed quest events (AQuestManager::RemoveQuest). Returns the number of quests
static TINT Dev_CompleteQuestTier( TINT a_iAbsTier )
{
	AQuestManager*            pQuestManager = AQuestManager::GetSingleton();
	AQuestManager::QuestTier* pTier         = pQuestManager->GetQuestTier( a_iAbsTier );
	TINT                      iNumEvents    = 0;

	for ( TINT k = 0; k < pTier->uiNumQuests; k++ )
	{
		// Already done
		if ( pTier->uiCompletedMask & ( 1 << k ) )
			continue;

		pTier->uiCompletedMask |= 1 << k;

		AQuest* pQuest = pQuestManager->GetQuest( pTier->auiQuests[ k ] );

		AInteractionQuestEvent* pEvent = TSTATICCAST( AInteractionQuestEvent, AInteractionEventManager::GetSingleton()->NewActiveEvent( &TGetClass( AInteractionQuestEvent ) ) );

		if ( !pEvent )
		{
			TWARN( "[devtier] no free quest event for '%s'\n", pQuest->GetName().GetString() );
			continue;
		}

		pEvent->SetQuest( pQuest );
		pEvent->SetCompleted( TTRUE );

		// The giver may not be spawned, its type is enough (AInteractionQuestEvent::SetUnitRef)
		AUnitRef oGiver;
		oGiver.pType = AUnitManager::GetSingleton()->FindUnit( pQuest->GetQuestGiver().GetString() );
		pEvent->SetUnitRef( &oGiver );

		iNumEvents++;
	}

	return iNumEvents;
}

// Called instead of AQuestManager::SetQuestTier( 1, 1 ) of a new game. Returns TFALSE if there's no "-devtier"
static TBOOL Dev_ApplyDevTier()
{
	TINT iAct, iTier;

	if ( !Dev_GetDevTier( iAct, iTier ) )
		return TFALSE;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	iTier                        = TMath::Min( iTier, pQuestManager->GetNumTiersInAct( iAct - 1 ) );

	const TINT iAbsTier = pQuestManager->GetAbsQuestTier( iAct, iTier );

	// The quests of the earlier tiers are complete. It's what a save keeps of them: the completed quests of every tier
	// (AQuestManager::SaveCompletedQuests) and the quest events AQuestManager::RemoveQuest throws for a completed quest,
	// which are persistent (saved by AInteractionEventManager). The "X Incomplete" requirements check that there's no
	// such event: without them the chained interactions of old quests (their minigames...) would still run
	TINT iNumEvents = 0;

	for ( TINT i = 1; i < iAbsTier; i++ )
		iNumEvents += Dev_CompleteQuestTier( i );

	TINFO( "[devtier] %d quests of the earlier tiers completed\n", iNumEvents );

	// Same as the start of a new game, at the given tier
	pQuestManager->SetQuestTier( iAct, iTier, TFALSE );

	// Same as AQuestManager::Load: the "Act N Tier M - Complete" requirements follow the tier
	ACheatActivator::GetSingleton()->UpdateQuestTier();
	ACheatActivator::GetSingleton()->UpdateTierRequirements();

	// What the completed quests gave for good
	AUnit*                        pPlayer    = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	AInventoryComponentInterface* pInventory = pPlayer ? AInventoryComponentInterface::GetInventory( pPlayer ) : TNULL;

	for ( TINT i = 0; i < TARRAYSIZE( s_aDevTierQuestRewards ); i++ )
	{
		const DevTierQuestReward& rcReward = s_aDevTierQuestRewards[ i ];

		if ( !pQuestManager->IsQuestComplete( TPString8( rcReward.szQuest ), 0 ) )
			continue;

		if ( rcReward.szItem && pInventory && !pInventory->FindItemByType( TPString8( rcReward.szItem ) ) )
			pInventory->AddItems( TPString8( rcReward.szItem ), 1, AInventoryComponentInterface::ms_strDefaultInventory );

		if ( rcReward.bPlayerSquirt && pPlayer )
			AMiscFunctions::GetSingleton()->EnablePlayerSquirt( TTRUE );

		if ( rcReward.szLatch )
			AInteractionManager::GetSingleton()->SetPersistentLatch( TPString8( rcReward.szLatch ), 1 );

		if ( rcReward.szGate )
			AGateManager::GetSingleton()->SetGate( TPString8( rcReward.szGate ), TTRUE );

		if ( rcReward.bSuperSquirt )
			AMiscFunctions::GetSingleton()->EnableSuperSquirtParticles( TTRUE );

		TINFO( "[devtier] reward of '%s': %s\n", rcReward.szQuest, rcReward.szItem ? rcReward.szItem : "(unlock)" );
	}

	// The quests of the tier
	AQuestManager::QuestTier* pTier = pQuestManager->GetCurrentQuestTier();

	for ( TINT k = 0; k < pTier->uiNumQuests; k++ )
	{
		AQuest* pQuest = pQuestManager->GetQuest( pTier->auiQuests[ k ] );
		TINFO( "[devtier] quest of the tier: '%s' (giver %s)\n", pQuest->GetName().GetString(), pQuest->GetQuestGiver().GetString() );
	}

	TINFO( "[devtier] act %d tier %d (abs %d of %d), persist act %d, %d quests assigned\n", pQuestManager->GetAct(), pQuestManager->GetTier(), pQuestManager->GetCurAbsQuestTier(), pQuestManager->GetNumTiers(), ASimPersistData::GetSingleton()->GetAct(), pQuestManager->GetNumAssignedQuests() );
	fflush( stdout );
	return TTRUE;
}

// Called after the animals of a "-devtier" game are spawned. AQuestManager::SetQuestTier (Dev_ApplyDevTier) ran before
// AAnimalPopulationManager::Populate, when the population manager ignores it (no spawn flags yet), so the story changes
// of the population were missing until the next change of the phase of the day: Ben stayed alive after his death (act
// 2 tier 19), Igg, Bud and Eddie weren't spawned, etc. Destroying Ben then, in the frame the night came, left a dying
// unit that AFarmerTruck::SetSequenceMode found by name (crash in AAnimatedUnit::IsVisible, no animatable object).
// The natural order of a new game is Populate, then SetQuestTier, like here
static void Dev_PopulateDevTier()
{
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	AAnimalPopulationManager::GetSingleton()->SetQuestTier( pQuestManager->GetCurAbsQuestTier(), TFALSE, 0 );

	// The units the story removed (Ben) are gone before the first frame, as in a loaded save
	AUnitManager::GetSingleton()->DeleteDyingUnits();
}

TBOOL AGameLoader::Dev_IsMiniGameRequested()
{
	TCHAR szName[ 64 ];
	return Dev_GetMiniGameArg( szName, sizeof( szName ) )[ 0 ] != '\0';
}

void AGameLoader::Dev_UpdateMiniGameRequest()
{
	static TBOOL s_bStarted           = TFALSE;
	static DWORD s_uiInstructionStart = 0;

	// The micro games are started by the sim (Dev_UpdateSimStateRequest)
	if ( !Dev_IsMiniGameRequested() || Dev_GetMicroGame() )
		return;

	if ( s_bStarted )
	{
		// Unattended runs: confirm the instruction screen after ~2 seconds, like pressing OK
		AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

		if ( pCurrentState && pCurrentState->IsA( &TGetClass( AMiniGameInstructionState ) ) )
		{
			if ( s_uiInstructionStart == 0 )
				s_uiInstructionStart = GetTickCount();

			if ( s_uiInstructionStart != DWORD( -1 ) && GetTickCount() - s_uiInstructionStart >= 2000 )
			{
				TINFO( "-minigame: dismissing the instruction screen\n" );
				s_uiInstructionStart = DWORD( -1 );
				TSTATICCAST( AMiniGameInstructionState, pCurrentState )->Dismiss();
			}
		}
		else
		{
			s_uiInstructionStart = 0;
		}

		return;
	}

	// Wait until the frontend's main menu is up
	if ( !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AFrontEndMainMenuState2 ) ) )
		return;

	s_bStarted = TTRUE;

	TCHAR szName[ 64 ];
	Dev_GetMiniGameArg( szName, sizeof( szName ) );

	AMiniGameManager* pMiniGameManager = AMiniGameManager::GetSingleton();
	TINT              iMiniGame        = -1;

	for ( TINT i = 0; i < pMiniGameManager->GetTotalNumMiniGames(); i++ )
	{
		if ( TStringManager::String8CompareNoCase( pMiniGameManager->GetMiniGame( i )->GetName().GetString(), szName ) == 0 )
		{
			iMiniGame = i;
			break;
		}
	}

	if ( iMiniGame == -1 || !pMiniGameManager->GetMiniGame( iMiniGame )->GetGameStateClass() )
	{
		TERROR( "-minigame: '%s' isn't a minigame or its game state isn't implemented\n", szName );
		return;
	}

	TINFO( "-minigame: starting '%s'\n", szName );

	// Same as picking the minigame in AFrontEndMiniGameState2::OnMenuClose
	if ( APlayerProfileManager::GetSingleton()->GetMainGender() == APlayerProfile::GENDER_NONE )
		APlayerProfileManager::GetSingleton()->SetMainGender( APlayerProfile::GENDER_MALE );

	APlayerSet* pPlayerSet = APlayerManager::GetSingleton()->GetPlayerSet( 1 );

	// "-devplayers <n>": n human players sharing the main controller (multiplayer turn tests)
	if ( const TCHAR* szPlayers = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devplayers " ) )
	{
		TINT iNumPlayers = TMath::Min( atoi( szPlayers + TStringManager::String8Length( "-devplayers " ) ), pMiniGameManager->GetMiniGame( iMiniGame )->GetMaxNumPlayers() );
		TINFO( "-devplayers: %d human players\n", iNumPlayers );

		for ( TINT i = iNumPlayers - 1; i >= 0; i-- )
		{
			TInputDeviceController* pController = ( i > 0 ) ? AInputHandler::GetSingleton()->GetController( i - 1 ) : TNULL;
			pPlayerSet->AddHumanPlayer( pController, -1 );
		}
	}
	else
	{
		pPlayerSet->AddHumanPlayer( AInputHandler::GetSingleton()->GetMainController(), -1 );
	}

	if ( pMiniGameManager->GetMiniGame( iMiniGame )->GetName() == TPS8D( "AGateCrashMiniGameState" ) )
	{
		for ( TINT i = pMiniGameManager->GetMiniGame( iMiniGame )->GetMaxNumPlayers() - pPlayerSet->GetHumanPlayerCount(); i > 0; i-- )
			pPlayerSet->AddAIPlayer();
	}

	pPlayerSet->MakeTeamsFair();
	pMiniGameManager->MakeValidMiniGamePlayerSet( iMiniGame );
	pPlayerSet->FinishUpAddingAIs();

	pMiniGameManager->SetMiniGameVariant( pMiniGameManager->GetMiniGame( iMiniGame )->GetDefaultVariant() );

	// The hidden minigames have no default variant (-1), the quests starting them set one
	if ( pMiniGameManager->GetMiniGameVariant() < 0 )
		pMiniGameManager->SetMiniGameVariant( 0 );

	// "-devvariant <n>": play variant n instead of the default one (as AFrontEndVariantSelectState does)
	if ( const TCHAR* szVariant = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devvariant " ) )
	{
		TINT iVariant = atoi( szVariant + TStringManager::String8Length( "-devvariant " ) );
		TINFO( "-devvariant: variant %d\n", iVariant );
		pMiniGameManager->SetMiniGameVariant( iVariant );
	}

	SetTransitionToMiniGame( iMiniGame, TFALSE );
}

// Development: gives the player the ingredients of the first recipe (and a few more) and starts ACookingMiniGame
// the way the kitchen (or, in the barn, the bar: the cocktails) does
static void Dev_StartCooking()
{
	AUnit*                    pPlayer       = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
	AInventoryGroupComponent* pInventory    = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	const TCHAR**             ppIngredients = TSTATICCAST( const TCHAR*, AAssetLoader::GetSymbolAddress( "cooking", "AllIngredients", AAssetType_AssetPack ) );
	RecipeEntryData*          pRecipes      = TSTATICCAST( RecipeEntryData, AAssetLoader::GetSymbolAddress( "cooking", "AllRecipes", AAssetType_AssetPack ) );

	if ( pRecipes && ppIngredients )
	{
		for ( TINT i = 0; i < ACookingRecipe::MAX_INGREDIENTS && pRecipes[ 0 ].aszIngredients[ i ]; i++ )
			pInventory->AddItems( TPString8( pRecipes[ 0 ].aszIngredients[ i ] ), 3, AInventoryComponentInterface::ms_strDefaultInventory );

		// cooking.AllIngredients has 7 words per ingredient, the first one is the unit type
		for ( TINT i = 0; i < 4; i++ )
			pInventory->AddItems( TPString8( ppIngredients[ i * 7 ] ), 2, AInventoryComponentInterface::ms_strDefaultInventory );
	}
	else
	{
		TWARN( "[devcooking] the cooking asset pack isn't loaded, no ingredients given\n" );
	}

	AGameStateController::GetSingleton()->StartGameState( TPS8( ACookingMiniGame ) );
}

// Development: "-devinventory" opens the inventory screen of the sim 6 seconds after the sim has
// started (like the inventory button does) and scrolls its items (right twice, left twice...) every 3 seconds. With
// "-devinventoryuse" the selected item is used 10 seconds later (which opens the screen of the item)
// Development, save system test (unattended runs of "-simstate"):
//  "-devsave":  15 seconds after the sim has started, changes the time, the pick ups and the player's
//               position and saves the game into slot 0 of the dev save file.
//  "-devload":  instead of starting a new game, loads the dev save file and starts the main game from slot 0.
//  "-devsavefile <path>": the dev save file, %TEMP%\OpenBarnyard_devsave.by by default (not in Final builds:
//               always the default there). The retail save (Save\savedata.by next to the executable) is never used by these.
static void Dev_SetupSavePath()
{
	static TBOOL s_bDone = TFALSE;

	if ( s_bDone )
		return;

	s_bDone = TTRUE;

	TCHAR szPath[ 260 ];

#  ifndef TOSHI_FINAL
	const TCHAR* szArg = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devsavefile " );

	if ( szArg )
	{
		szArg += TStringManager::String8Length( "-devsavefile " );
		TINT iLength = 0;

		while ( szArg[ iLength ] != '\0' && szArg[ iLength ] != ' ' && iLength < TARRAYSIZE( szPath ) - 1 )
		{
			szPath[ iLength ] = szArg[ iLength ];
			iLength++;
		}

		szPath[ iLength ] = '\0';
	}
	else
#  endif
	{
		GetTempPathA( sizeof( szPath ), szPath );
		strcat( szPath, "OpenBarnyard_devsave.by" );
	}

	SaveLoadSKU::Dev_SetSavePath( szPath );
	TINFO( "[devsave] save file: %s\n", szPath );
}

TBOOL AGameLoader::Dev_SetupSaveFileAtBoot()
{
#  ifndef TOSHI_FINAL
	if ( !TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devsavefile " ) )
		return TFALSE;

	Dev_SetupSavePath();
	return TTRUE;
#  else
	// No "-devsavefile" in Final builds
	return TFALSE;
#  endif
}

static void Dev_LogGameState( const TCHAR* a_szTag )
{
	AGameTimeManager*  pTime    = AGameTimeManager::GetSingleton();
	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
	AQuestManager*     pQuests  = AQuestManager::GetSingleton();
	AUnit*             pPlayer  = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	TVector4           vecPos   = pPlayer ? *pPlayer->GetSteer()->GetPosition() : TVector4::VEC_ZERO;

	TCHAR szName[ 64 ];
	TStringManager::StringUnicodeToChar( szName, pProfile->GetName().GetString( 0 ), -1 );

	TINFO( "[%s] time %02d:%02d phase %d date %d/%d/%d total %.1f\n", a_szTag, pTime->GetCurrentHour(), pTime->GetCurrentMinute(), pTime->GetCurrentDayPhase(), pTime->GetDay(), pTime->GetMonth(), pTime->GetYear(), pTime->GetTotalTime() );
	TINFO( "[%s] profile '%s' gender %d costume %d pickups %d stamina %.2f/%.2f\n", a_szTag, szName, pProfile->GetGender(), pProfile->GetCostume(), pProfile->GetDisplayedPickups(), pProfile->GetStamina(), pProfile->GetMaxStamina() );
	TINFO( "[%s] quests act %d tier %d assigned %d, persist act %d flags 0x%x\n", a_szTag, pQuests->GetAct(), pQuests->GetTier(), pQuests->GetNumAssignedQuests(), ASimPersistData::GetSingleton()->GetAct(), ASimPersistData::GetSingleton()->GetFlags() );
	TINFO( "[%s] player %s at (%.2f %.2f %.2f)\n", a_szTag, pPlayer ? pPlayer->GetType()->GetName().GetString() : "none", vecPos.x, vecPos.y, vecPos.z );

	fflush( stdout );
}

// Starts a new main game without the frontend's New Game flow ("-simstate", or "-devload" with an empty slot 0).
// The SS01 intro sequence that's skipped picks the gender (ASSeqUI::SetGender, same as
// AFrontEndNewGameSelectState::OnMenuClose) and enters the name (ASSeqUI::OnLetterActivated). Without a gender
// AAssetLoader doesn't load the player's model, so the "player" unit gets no animatable object and its
// facial component crashes in AUnitFacialComponent::GetHeadPosition (as it would in retail)
static void Dev_StartNewMainGame()
{
	if ( APlayerProfileManager::GetSingleton()->GetMainGender() == APlayerProfile::GENDER_NONE )
		APlayerProfileManager::GetSingleton()->SetMainGender( APlayerProfile::GENDER_MALE );

	if ( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().Length() == 0 )
		APlayerProfileManager::GetSingleton()->GetMainProfile()->SetName( L"Archie" );

	// "-devtier": the act decides the assets of the main game
	Dev_StartDevTierGame();

	AGameLoader::SetTransitionToMainGame( TFALSE );
}

static void Dev_UpdateSaveTest()
{
	// NOTE: separate phases, so that "-devsave -devload" (a new game when the dev save is empty, then a save)
	// doesn't let the load phase skip the save
	static TFLOAT s_fTime      = 0.0f;
	static TINT   s_iSavePhase = 0;
	static TINT   s_iLoadPhase = 0;

	s_fTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	if ( Dev_HasSwitch( "-devsave" ) )
	{
		if ( s_iSavePhase == 0 && s_fTime >= 15.0f && APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) &&
		     AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( ASimState ) ) )
		{
			s_iSavePhase = 1;
			Dev_SetupSavePath();

			// Make the state easy to recognise
			AGameTimeManager::GetSingleton()->SetTime( 15, 30, 0, TFALSE, TTRUE );
			APlayerProfileManager::GetSingleton()->GetMainProfile()->SetPickups( 4321, TFALSE, TTRUE );

			AUnit*   pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
			TVector4 vecPos  = *pPlayer->GetSteer()->GetPosition();
			vecPos.x += 3.0f;
			pPlayer->GetSteer()->SetPosition( vecPos );

			Dev_LogGameState( "devsave before" );

			APlayerProgress::GetSingleton()->SetCurrentSlotIndex( 0 );

			// Same as AGameStateController::SaveCurrentGameState (the save prompt)
			SaveLoadSKU* pSaveLoad = new SaveLoadSKU( APlayerProgress::GetSingleton(), TFALSE, TNULL );
			pSaveLoad->StartSave( TFALSE, TFALSE, TTRUE );
			AGameStateController::GetSingleton()->PushState( pSaveLoad );
		}
		else if ( s_iSavePhase == 1 && !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( SaveLoadSKU ) ) )
		{
			s_iSavePhase = 2;

			APlayerProgress::SaveSlot* pSlot = APlayerProgress::GetSingleton()->GetSlot( 0 );
			TINFO( "[devsave] saved: slot occupied %d, date 0x%08x time 0x%08x, gender %d costume %d act %d, checksum 0x%08x\n", pSlot->bOccupied, pSlot->uiDate, pSlot->uiTime, pSlot->iGender, pSlot->iCostume, pSlot->iAct, APlayerProgress::GetSingleton()->GetSaveData()->oHeader.uiChecksum );
			Dev_LogGameState( "devsave after" );
		}
	}

	if ( Dev_HasSwitch( "-devload" ) )
	{
		if ( s_iLoadPhase == 0 && s_fTime >= 8.0f )
		{
			s_iLoadPhase = 1;
			Dev_LogGameState( "devload loaded" );
		}
	}
}

static void Dev_UpdateInventoryTest()
{
	static TFLOAT s_fTime       = 0.0f;
	static TFLOAT s_fNextScroll = 9.0f;
	static TINT   s_iNumScrolls = 0;
	static TBOOL  s_bOpened     = TFALSE;
	static TBOOL  s_bUsed       = TFALSE;

	s_fTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pPlayer || !AGameStateController::GetSingleton()->GetCurrentState()->IsA( &TGetClass( ASimState ) ) )
		return;

	if ( !s_bOpened && s_fTime >= 6.0f )
	{
		s_bOpened = TTRUE;

		// Press the inventory button
		APlayerControlComponent*    pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );
		TInputInterface::InputEvent oEvent( AInputHandler::GetSingleton()->GetKeyboardDevice(), 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
		pControlComponent->ProcessCommand( AInputCommand_Inventory, &oEvent );

		TINFO( "[devinventory] opened the inventory\n" );
		fflush( stdout );
	}

	AInventoryState2* pInventoryState = TSTATICCAST( AInventoryState2, AGameStateController::GetSingleton()->GetCurrentState()->FindChildState( &TGetClass( AInventoryState2 ) ) );

	if ( !pInventoryState || !pInventoryState->IsOpen() )
	{
		static TBOOL s_bReportedClosed = TFALSE;

		if ( s_bOpened && !s_bReportedClosed )
		{
			s_bReportedClosed = TTRUE;
			TINFO( "[devinventory] the inventory is closed (state %s)\n", pInventoryState ? "exists" : "removed" );
			fflush( stdout );
		}

		return;
	}

	TInputInterface::InputEvent oEvent( AInputHandler::GetSingleton()->GetKeyboardDevice(), 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
	TBOOL                       bStopEvents = TFALSE;

	if ( Dev_HasSwitch( "-devinventoryuse" ) && !s_bUsed && s_iNumScrolls >= 3 )
	{
		s_bUsed = TTRUE;
		TINFO( "[devinventory] using the selected item\n" );
		pInventoryState->ProcessCommand( AInputCommand_Interact, &oEvent, bStopEvents );
		fflush( stdout );
		return;
	}

	if ( s_fTime >= s_fNextScroll )
	{
		s_fNextScroll = s_fTime + 3.0f;
		s_iNumScrolls++;

		TINFO( "[devinventory] scrolling (%d)\n", s_iNumScrolls );
		pInventoryState->ProcessCommand( ( s_iNumScrolls & 2 ) ? AInputCommand_Left : AInputCommand_Right, &oEvent, bStopEvents );
		fflush( stdout );
	}
}

// Development: "-devkeys W:2-6,Up:8-10,I:12" presses keyboard keys (names of the input map
// manager's keyboard doodads, or the mouse buttons Button1-Button8) at the given seconds after the sim has started, holding them until the
// end time or for a short tap. The key changes are injected into the keyboard device itself, so the
// whole input path (input map, commands, game states and control components) gets exercised
// Development: "-devsunglasses" gives the sim player the sunglasses and puts them on 5 seconds after
// the sim has started. The player can only use the milk squirt while wearing them
static void Dev_UpdateSunglasses()
{
	static TFLOAT s_fTime = 0.0f;
	static TBOOL  s_bDone = TFALSE;

	if ( s_bDone )
		return;

	s_fTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	if ( s_fTime < 5.0f )
		return;

	s_bDone = TTRUE;

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pPlayer )
		return;

	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, AInventoryComponentInterface::GetInventory( pPlayer ) );

	if ( !pInventory )
		return;

	if ( !pInventory->FindItemByType( TPS8( sunglasses ) ) )
		pInventory->AddItems( TPS8( sunglasses ), 1, AInventoryComponentInterface::ms_strDefaultInventory );

#  ifdef BARNYARD_LATEST
	// "-devwiicamoff" turns the Wii Style camera option off (the PC retail aim camera)
	if ( Dev_HasSwitch( "-devwiicamoff" ) )
		AOptions::GetSingleton()->SetWiiStyleCamera( TFALSE );
#  endif

	AInventoryItem* pItem   = pInventory->FindItemByType( TPS8( sunglasses ) );
	TBOOL           bResult = AInventoryGroupComponent::TakeOutItem( pItem, AGameStateController::GetSingleton()->GetCurrentState() );

	TINFO( "[devsunglasses] item %p, put on: %d\n", pItem, bResult );
	fflush( stdout );
}

// Development: "-devshake tree|crate|milkcan" puts the player 6 seconds after the sim has started in front of the
// closest tree with fruits, crate or milk can (of the loaded terrain), and 3 seconds later kicks it the way the kick
// of the player does (AShakeTreeMicroGame::PushState( -1 )) or adds AMilkCanMicroGame by name. With "-devnightbarn"
// it's done 6 seconds after entering the barn
static void Dev_UpdateShake()
{
	static TFLOAT s_fTime  = 0.0f;
	static TINT   s_iPhase = 0;

	s_fTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	// With "-devnightbarn" it's done in the barn (the milk cans are there)
	if ( !pPlayer || ( Dev_HasSwitch( "-devnightbarn" ) && !AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "ABarnExploreMinigameState" ) ) ) )
	{
		s_fTime = 0.0f;
		return;
	}

	const TCHAR* szArg    = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devshake " ) + TStringManager::String8Length( "-devshake " );
	TBOOL        bTree    = TStringManager::String8Compare( szArg, "tree", 4 ) == 0;
	TBOOL        bMilkCan = TStringManager::String8Compare( szArg, "milkcan", 7 ) == 0;

	if ( s_iPhase == 0 && s_fTime >= 6.0f )
	{
		s_iPhase = 1;

		ARegrowthManager* pRegrowth = ARegrowthManager::GetSingleton();
		TVector4          vecPlayer = *pPlayer->GetSteer()->GetPosition();
		TVector4          vecTarget;
		TFLOAT            fBestDistance = TMath::MAXFLOAT;

		for ( TINT i = 0; i < 32; i++ )
		{
			TFLOAT   fAngle = TFLOAT( i ) * ( TMath::TWO_PI / 32.0f );
			TVector2 vecDirection( TMath::Cos( fAngle ), TMath::Sin( fAngle ) );
			TVector4 vecPosition;
			TBOOL    bFound = TFALSE;

			if ( bTree )
			{
				ATreeManager::TreeInstance* pTree = ATreeManager::GetSingleton()->FindTreeInFront( vecPlayer, vecDirection, 300.0f );

				if ( pTree && pTree->uiRegrowthGroup != 0xff )
				{
					TINT iType = pRegrowth->FindTypeOfGroup( pTree->uiRegrowthGroup );

					if ( ARegrowthManager::HasTypeFlag( iType, ARegrowthManager::TYPEFLAG_FRUIT ) && pRegrowth->FindRandomObjectAtStage( pTree->uiRegrowthGroup, 10000, iType ) )
					{
						vecPosition = pTree->GetPosition();
						bFound      = TTRUE;
					}
				}
			}
			else if ( i == 0 )
			{
				// The grown objects of the types
				ARegrowthManager::TypeMask oMask = bMilkCan ? ARegrowthManager::TypeMask::FromName( TPS8D( "milkcan" ) ) : ARegrowthManager::TypeMask::Crates();
				TFLOAT                     fTime = AGameTimeManager::GetSingleton()->GetTotalTime();

				for ( TINT iType = ARegrowthManager::GetTypes()->GetNumEnums() - 1; iType >= 0; iType-- )
				{
					if ( !ARegrowthManager::TypeMatches( iType, oMask ) )
						continue;

					const ARegrowthManager::GrowthMapping& rcMapping = pRegrowth->GetGrowthMapping( iType );

					T2_FOREACH( pRegrowth->GetTypeLists( iType ).llGrowing, it )
					{
						if ( rcMapping.uiFirstModel == 0xff || TINT( TINT64( ( fTime - it->fGrowTime ) * rcMapping.fStagesPerSecond ) ) < rcMapping.iNumStages - 1 )
							continue;

						TFLOAT fDistance = TVector4::DistanceXZ( vecPlayer, it->vecPosition );

						if ( fDistance < fBestDistance )
						{
							fBestDistance = fDistance;
							vecTarget     = it->vecPosition;
						}
					}
				}
			}

			if ( bFound )
			{
				TFLOAT fDistance = TVector4::DistanceXZ( vecPlayer, vecPosition );

				if ( fDistance < fBestDistance )
				{
					fBestDistance = fDistance;
					vecTarget     = vecPosition;
				}
			}
		}

		if ( fBestDistance < TMath::MAXFLOAT )
		{
			TVector4 vecFacing( vecTarget.x - vecPlayer.x, 0.0f, vecTarget.z - vecPlayer.z, 1.0f );
			vecFacing.Normalise();

			TFLOAT   fStandDistance = bTree ? 1.8f : 1.0f;
			TVector4 vecStand( vecTarget.x - vecFacing.x * fStandDistance, vecTarget.y, vecTarget.z - vecFacing.z * fStandDistance, 1.0f );

			pPlayer->SetSteerPosition( vecStand );
			pPlayer->SetSteerDirection( vecFacing );

			TINFO( "[devshake] the player is in front of the %s at %.2f %.2f %.2f (%.1f away)\n", bTree ? "tree" : ( bMilkCan ? "milk can" : "crate" ), vecTarget.x, vecTarget.y, vecTarget.z, fBestDistance );
		}
		else
		{
			TWARN( "[devshake] nothing found\n" );
		}

		fflush( stdout );
	}
	else if ( s_iPhase == 1 && s_fTime >= 9.0f )
	{
		s_iPhase = 2;

		if ( bMilkCan )
		{
			AGameStateController::GetSingleton()->AddChildStateByName( TPS8D( "AMilkCanMicroGame" ) );
			TINFO( "[devshake] added AMilkCanMicroGame\n" );
		}
		else
		{
			AShakeTreeMicroGame::PushState( -1 );
			TINFO( "[devshake] kicking\n" );
		}

		fflush( stdout );
	}
}

// Development: "-devmilkweed" plants a milkweed (ALadderBeansGame) 4 seconds after the sim has started: it gives
// the player a milkweed_seed, takes them to the closest empty patch (in 15 unit hops, the terrain streams in around
// the camera) and presses the action button (the area
// interaction "milkweed"). Then it puts the player 4 units in front of the milkweed, facing it, so that
// "-devsunglasses -devkeys Button2:14-60,Button1:16-16.2,..." squirts it. Every change of the ladders is logged
static void Dev_UpdateMilkweed()
{
	static TFLOAT   s_fTime       = 0.0f;
	static TFLOAT   s_fNextAction = 4.0f;
	static TINT     s_iPhase      = 0;
	static TINT     s_iLadder     = -1;
	static TINT     s_iNumTries   = 0;
	static TINT8    s_aiStages[ ASimPersistData::NUM_LADDER_BEANS ];
	static TBOOL    s_bPopupLogged = TFALSE;
	static TVector4 s_vecHold;
	static TVector4 s_vecTarget;
	static TVector4 s_vecFace;
	static TBOOL    s_bHoldYSet = TFALSE;

	s_fTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "ASimState" ) ) );
	AUnit*     pPlayer   = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pSimState || !pSimState->GetLadderBeansGame() || !pPlayer )
		return;

	ALadderBeansGame*                pGame    = pSimState->GetLadderBeansGame();
	ASimPersistData::LadderBeansSet* pLadders = ASimPersistData::GetSingleton()->GetLadderBeans();
	AInteriorNetwork*                pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();

	if ( s_iPhase == 0 && s_fTime >= s_fNextAction )
	{
		// The closest empty patch
		const TVector4 vecPlayerPos = *pPlayer->GetSteer()->GetPosition();
		TFLOAT         fBestDistSq  = TMath::MAXFLOAT;

		for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
		{
			s_aiStages[ i ] = pLadders->aLadders[ i ].iStage;

			if ( pGame->IsPlanted( i ) )
				continue;

			TCHAR szArea[ 16 ];
			T2String8::Format( szArea, "mweed%02d", ALadderBeansGame::GetLadderId( i ) );

			TVector4 vecArea;
			pNetwork->GetArea( pNetwork->FindAreaIndex( TPString8( szArea ) ) )->GetPosition( vecArea );

			const TFLOAT fDistSq = TVector4::DistanceSq( vecArea, vecPlayerPos );

			if ( fDistSq < fBestDistSq )
			{
				fBestDistSq = fDistSq;
				s_iLadder   = i;
			}
		}

		s_iPhase = 3;

		if ( s_iLadder < 0 )
		{
			TWARN( "[devmilkweed] every patch is planted already\n" );
			fflush( stdout );
			return;
		}

		AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, AInventoryComponentInterface::GetInventory( pPlayer ) );

		if ( pInventory && !pInventory->FindItemByType( TPS8D( "milkweed_seed" ) ) )
			pInventory->AddItems( TPS8D( "milkweed_seed" ), 1, AInventoryComponentInterface::ms_strDefaultInventory );

		TCHAR szArea[ 16 ];
		T2String8::Format( szArea, "mweed%02d", ALadderBeansGame::GetLadderId( s_iLadder ) );
		AInteriorArea* pArea = pNetwork->GetArea( pNetwork->FindAreaIndex( TPString8( szArea ) ) );

		pArea->GetPosition( s_vecTarget );
		s_vecTarget.y = pArea->GetSpecialHeight() - 0.5f; // -Y is up

		TVector4 vecMilkweed;
		ALadderBeansGame::GetMilkweedPosition( pArea, vecMilkweed );
		s_vecFace = TVector4( vecMilkweed.x - s_vecTarget.x, 0.0f, vecMilkweed.z - s_vecTarget.z, 0.0f );
		s_vecFace.Normalise();
		s_fNextAction = s_fTime;

		TINFO( "[devmilkweed] patch %s (ladder %d, stage %d) at (%.2f %.2f %.2f), seed given\n", szArea, s_iLadder, pLadders->aLadders[ s_iLadder ].iStage, s_vecTarget.x, s_vecTarget.y, s_vecTarget.z );
		fflush( stdout );
	}
	else if ( s_iPhase == 3 && s_fTime >= s_fNextAction )
	{
		// Hop towards the patch, the terrain only streams in around the camera
		const TVector4 vecPlayerPos = *pPlayer->GetSteer()->GetPosition();
		TVector4       vecToTarget( s_vecTarget.x - vecPlayerPos.x, 0.0f, s_vecTarget.z - vecPlayerPos.z, 0.0f );
		const TFLOAT   fDistance = vecToTarget.Magnitude();

		if ( fDistance < 15.0f )
		{
			pPlayer->GetSteer()->SetPosition( s_vecTarget );
			pPlayer->GetSteer()->SetDirection( s_vecFace );
			s_vecHold     = s_vecTarget;
			s_bHoldYSet   = TFALSE;
			s_iPhase      = 1;
			s_fNextAction = s_fTime + 1.0f;

			TINFO( "[devmilkweed] the player is in the patch\n" );
		}
		else
		{
			vecToTarget.Divide( fDistance );

			TVector4 vecPos( vecPlayerPos.x + vecToTarget.x * 15.0f, 0.0f, vecPlayerPos.z + vecToTarget.z * 15.0f, 1.0f );
			vecPos.y = ATerrainInterface::GetSingleton()->GetGroundHeightTop( vecPos.x, vecPos.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );

			// Keep the height where the ground isn't loaded yet
			vecPos.y = ( vecPos.y != 0.0f ) ? vecPos.y - 0.3f : vecPlayerPos.y;

			{
				pPlayer->GetSteer()->SetPosition( vecPos );
				pPlayer->GetSteer()->SetDirection( vecToTarget );
				s_vecHold   = vecPos;
				s_bHoldYSet = TTRUE;
				TINFO( "[devmilkweed] hop to (%.2f %.2f %.2f)\n", vecPos.x, vecPos.y, vecPos.z );
			}

			s_fNextAction = s_fTime + 0.75f;
		}

		fflush( stdout );
	}
	else if ( s_iPhase == 1 && s_iLadder >= 0 && s_fTime >= s_fNextAction )
	{
		if ( !pGame->IsPlanted( s_iLadder ) )
		{
			if ( s_iNumTries++ < 5 )
			{
				TINFO( "[devmilkweed] pressing the action button (try %d), player at (%.2f %.2f %.2f)\n", s_iNumTries, pPlayer->GetSteer()->GetPosition()->x, pPlayer->GetSteer()->GetPosition()->y, pPlayer->GetSteer()->GetPosition()->z );
				APlayerControlComponent::DevInteract();
				s_fNextAction = s_fTime + 2.5f;
			}
		}
		else if ( !( pLadders->aLadders[ s_iLadder ].uiFlags & ASimPersistData::LadderBeans::LADDERFLAGS_GROWING ) && !pPlayer->HasInteraction() )
		{
			// Stand in front of the planted milkweed, facing it
			TVector4        vecMilkweed = *pGame->GetMilkweed( s_iLadder )->GetSteer()->GetPosition();
			const TVector4* pPlayerPos  = pPlayer->GetSteer()->GetPosition();
			TVector4        vecFace( vecMilkweed.x - pPlayerPos->x, 0.0f, vecMilkweed.z - pPlayerPos->z, 0.0f );
			vecFace.Normalise();

			TVector4 vecPos( vecMilkweed.x - vecFace.x * 4.0f, s_vecHold.y, vecMilkweed.z - vecFace.z * 4.0f, 1.0f );

			pPlayer->GetSteer()->SetPosition( vecPos );
			s_vecHold = vecPos;
			pPlayer->GetSteer()->SetDirection( vecFace );
			s_iPhase = 2;

			TINFO( "[devmilkweed] planted, the player is in front of the milkweed at (%.2f %.2f %.2f)\n", vecPos.x, vecPos.y, vecPos.z );
		}

		fflush( stdout );
	}

	if ( s_iPhase == 2 && s_fTime >= s_fNextAction )
	{
		// Keep the milk full
		s_fNextAction = s_fTime + 2.0f;

		for ( TINT i = 0; i < AUnitPlayer::NUM_LAUNCHERS; i++ )
		{
			AProjectileLauncher* pLauncher = TSTATICCAST( AUnitPlayer, pPlayer )->GetLauncher( i );

			if ( pLauncher && pLauncher->GetAmmo() )
				pLauncher->GetAmmo()->SetAmmo( pLauncher->GetAmmo()->GetMaxAmmo() );
		}
	}

	// The terrain collision may not be there: put the player back when they fall through
	if ( s_iPhase > 0 && s_iLadder >= 0 )
	{
		const TVector4* pPlayerPos = pPlayer->GetSteer()->GetPosition();

		if ( !s_bHoldYSet )
		{
			s_vecHold.y = pPlayerPos->y;
			s_bHoldYSet = TTRUE;
		}
		else if ( pPlayerPos->y > s_vecHold.y + 1.0f )
		{
			pPlayer->GetSteer()->SetPosition( s_vecHold );
		}
	}

	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		const ASimPersistData::LadderBeans& rcLadder = pLadders->aLadders[ i ];

		if ( s_iPhase > 0 && rcLadder.iStage != s_aiStages[ i ] )
		{
			s_aiStages[ i ] = rcLadder.iStage;
			TINFO( "[devmilkweed] ladder %d: stage %d, height %.3f -> %.3f (max %.3f)\n", i, rcLadder.iStage, rcLadder.fHeight, rcLadder.fTargetHeight, rcLadder.fMaxHeight );
			fflush( stdout );
		}
	}

	if ( !s_bPopupLogged && AGameStateController::GetSingleton()->GetCurrentState()->IsA( &TGetClass( AMessagePopupState ) ) )
	{
		s_bPopupLogged = TTRUE;
		TINFO( "[devmilkweed] reward popup shown, unlocked flags 0x%x\n", APlayerProgress::GetSingleton()->GetUnlockedFlags() );
		fflush( stdout );
	}
}

#  ifndef TOSHI_FINAL

// Development (not in Final builds): "-devkeys" and "-devmouse"
static void Dev_PressKey( TInputDevice::Doodad a_iDoodad, TBOOL a_bDown )
{
	if ( a_iDoodad < TInputDeviceMouse::BUTTON_1 )
	{
		TInputDeviceKeyboard::Dev_QueueKey( a_iDoodad, a_bDown );
		return;
	}

	// Mouse buttons (Button1-Button8) are thrown straight to the input interface's emitter (and count as down for the repeats)
	TInputDeviceMouse::Dev_SetButtonDown( a_iDoodad, a_bDown );
	TInputDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>();

	if ( pMouse )
	{
		TInputInterface::GetSingleton()->GetInputEventEmitter().Throw(
		    TInputInterface::InputEvent( pMouse, a_iDoodad, a_bDown ? TInputInterface::EVENT_TYPE_GONE_DOWN : TInputInterface::EVENT_TYPE_GONE_UP )
		);
	}
}

static void Dev_UpdateKeys()
{
	struct DevKey
	{
		TInputDevice::Doodad iDoodad;
		TFLOAT               fStart;
		TFLOAT               fEnd;
		TINT                 iState; // 0: waiting, 1: down, 2: done
	};

	static DevKey s_aKeys[ 64 ];
	static TINT   s_iNumKeys = -1;
	static TFLOAT s_fTime    = 0.0f;

	if ( s_iNumKeys < 0 )
	{
		s_iNumKeys = 0;

		const TCHAR* szArg = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devkeys " ) + TStringManager::String8Length( "-devkeys " );

		while ( *szArg == '"' ) szArg++;

		while ( *szArg != '\0' && *szArg != ' ' && *szArg != '"' && s_iNumKeys < TARRAYSIZE( s_aKeys ) )
		{
			TCHAR szName[ 32 ];
			TINT  iLength = 0;

			while ( *szArg != '\0' && *szArg != ':' && *szArg != ' ' && iLength < TARRAYSIZE( szName ) - 1 )
				szName[ iLength++ ] = *szArg++;

			szName[ iLength ] = '\0';

			TFLOAT fStart = 0.0f;
			TFLOAT fEnd   = -1.0f;

			if ( *szArg == ':' )
			{
				TCHAR* szEnd;
				fStart = TFLOAT( strtod( szArg + 1, &szEnd ) );
				szArg  = szEnd;

				if ( *szArg == '-' )
				{
					fEnd  = TFLOAT( strtod( szArg + 1, &szEnd ) );
					szArg = szEnd;
				}
			}

			if ( fEnd < fStart ) fEnd = fStart + 0.2f;

			TInputDevice::Doodad iDoodad = AInputMapManager::GetSingleton()->GetDoodadFromKey( TPString8( szName ) );

			if ( ( iDoodad >= TInputDeviceKeyboard::DOODAD_START && iDoodad < TInputDeviceKeyboard::DOODAD_END ) ||
			     ( iDoodad >= TInputDeviceMouse::BUTTON_1 && iDoodad <= TInputDeviceMouse::BUTTON_1 + 7 ) )
			{
				s_aKeys[ s_iNumKeys++ ] = { iDoodad, fStart, fEnd, 0 };
				TINFO( "[devkeys] %s (0x%x) from %.1f to %.1f\n", szName, iDoodad, fStart, fEnd );
			}
			else
			{
				TWARN( "[devkeys] no keyboard key called %s\n", szName );
			}

			while ( *szArg != '\0' && *szArg != ',' && *szArg != ' ' && *szArg != '"' ) szArg++;
			if ( *szArg == ',' ) szArg++;
		}

		fflush( stdout );
	}

	s_fTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	AUnit*   pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	TVector4 vPos    = ( pPlayer && pPlayer->GetSteer() ) ? *pPlayer->GetSteer()->GetPosition() : TVector4::VEC_ZERO;

	for ( TINT i = 0; i < s_iNumKeys; i++ )
	{
		DevKey& rKey = s_aKeys[ i ];

		if ( rKey.iState == 0 && s_fTime >= rKey.fStart )
		{
			rKey.iState = 1;
			Dev_PressKey( rKey.iDoodad, TTRUE );
			TINFO( "[devkeys] 0x%x down at %.1f, player at (%.2f, %.2f, %.2f)\n", rKey.iDoodad, s_fTime, vPos.x, vPos.y, vPos.z );
			fflush( stdout );
		}
		else if ( rKey.iState == 1 && s_fTime >= rKey.fEnd )
		{
			rKey.iState = 2;
			Dev_PressKey( rKey.iDoodad, TFALSE );
			TINFO( "[devkeys] 0x%x up at %.1f, player at (%.2f, %.2f, %.2f)\n", rKey.iDoodad, s_fTime, vPos.x, vPos.y, vPos.z );
			fflush( stdout );
		}
	}

	// "-devmouse <dx>,<dy>:<start>-<end>" moves the mouse by dx, dy pixels every frame in that time (sent through the
	// input event emitter like the mouse device does), e.g. to turn a camera. The switch can be given several times
	for ( const TCHAR* szMouse = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devmouse " ); szMouse; szMouse = TStringManager::String8FindString( szMouse + 1, "-devmouse " ) )
	{
		TINT   iDX = 0, iDY = 0;
		TFLOAT fStart = 0.0f, fEnd = 0.0f;

		if ( sscanf( szMouse + TStringManager::String8Length( "-devmouse " ), "%d,%d:%f-%f", &iDX, &iDY, &fStart, &fEnd ) == 4 && s_fTime >= fStart && s_fTime < fEnd )
		{
			TInputDeviceMouse* pMouse = AInputHandler::GetSingleton()->GetMouseDevice();
			TInputInterface::GetSingleton()->GetInputEventEmitter().Throw( TInputInterface::InputEvent( pMouse, TInputDeviceMouse::AXIS_CURSOR, TInputInterface::EVENT_TYPE_MOVED, iDX, iDY ) );
		}
	}
}

#  endif // !TOSHI_FINAL

TBOOL AGameLoader::Dev_IsSimStateRequested()
{
	// "-devtier" and "-devadvancetier" start the sim the same way
	return Dev_HasSwitch( "-simstate" ) || Dev_HasSwitch( "-devtier" ) || Dev_HasSwitch( "-devadvancetier" );
}

// Development: plays the scripted sequence of the "-devtier" tier once the sim is up (see LoadMainGameImpl)
void AGameLoader::Dev_UpdateDevTierSequence()
{
	if ( s_iDevTierSequence == -1 || !s_bMainGameLoaded || ARootTask::GetSingleton()->IsTransitioning() )
		return;

	AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( !pCurrentState || pCurrentState->GetClass() != &TGetClass( ASimState ) )
		return;

	TINFO( "[devtier] playing the scripted sequence %d of the tier\n", s_iDevTierSequence );

	SetTransitionToScriptedSequence( s_iDevTierSequence );
	SetPrevGameStateClass( s_pDevTierSequencePrevState );

	if ( !s_strDevTierSequenceGoal.IsEmpty() && s_strDevTierSequenceGoal != TPS8( park2_goals_playerstart ) )
		SetPlayerStartGoal( s_strDevTierSequenceGoal );

	s_iDevTierSequence          = -1;
	s_pDevTierSequencePrevState = TNULL;
	s_strDevTierSequenceGoal    = TPString8();
}

// Development: "-simstate" starts a new game of the sim like the New Game menu does, but without
// the save slot selection and the SS01 intro sequence
// One-shot dev switches done (see Dev_IsRequestPending)
static TBOOL s_bDevShopOpened   = TFALSE;
static TBOOL s_bDevCoinsSpilled = TFALSE;

TBOOL AGameLoader::Dev_IsRequestPending()
{
	if ( ACheatActivator::Dev_IsPending() )
		return TTRUE;

	if ( Dev_HasSwitch( "-devgophershop" ) && !s_bDevShopOpened )
		return TTRUE;

	if ( Dev_HasSwitch( "-devcoins" ) && !s_bDevCoinsSpilled )
		return TTRUE;

	return TFALSE;
}

void AGameLoader::Dev_UpdateSimStateRequest()
{
	static TBOOL s_bStarted = TFALSE;

#  ifndef TOSHI_FINAL
	if ( !Dev_IsSimStateRequested() && Dev_HasSwitch( "-devkeys" ) )
	{
		// Development: "-devkeys" without "-simstate" presses the keys from the moment the frontend's main menu is
		// first up (New Game / Load Game go through the real menus). The save file is the dev one ("-devsavefile").
		static TBOOL s_bFrontEndUp = TFALSE;

		if ( !s_bFrontEndUp && AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AFrontEndMainMenuState2 ) ) )
		{
			s_bFrontEndUp = TTRUE;
			Dev_SetupSavePath();
			TINFO( "[devkeys] the main menu is up\n" );
		}

		if ( s_bFrontEndUp )
			Dev_UpdateKeys();

		// The other dev switches of the sim work once the main game is loaded this way
		if ( !s_bStarted && s_bMainGameLoaded )
			s_bStarted = TTRUE;
	}
#  endif

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devwalk" ) )
	{
		// Development: "-devwalk" makes the sim player walk forward while slowly changing direction
		static TFLOAT s_fWalkTime = 0.0f;

		s_fWalkTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		APlayerControlComponent::SetDevStick( TMath::Sin( s_fWalkTime * 0.5f ) * 0.6f, -1.0f );
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devtalk" ) )
	{
		// Development: "-devtalk" puts the sim player in front of the closest unit that can talk and
		// presses the action button every few seconds to start a conversation
		static TFLOAT s_fTalkTime   = 0.0f;
		static TFLOAT s_fNextAction = 4.0f;
		static TBOOL  s_bPlaced     = TFALSE;

		s_fTalkTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		if ( pPlayer && !s_bPlaced && s_fTalkTime >= 3.0f )
		{
			s_bPlaced = TTRUE;

			TINT                    iNumUnits;
			const TVector4          vecPlayerPos = *pPlayer->GetSteer()->GetPosition();
			AUnitHash::QueryResult* pResults     = AUnitManager::GetSingleton()->FindUnits( vecPlayerPos, 60.0f, &iNumUnits, TFALSE, 420, TFALSE, TNULL );
			AUnit*                  pBest        = TNULL;
			TFLOAT                  fBestDistSq  = TMath::MAXFLOAT;

			for ( TINT i = 0; i < iNumUnits; i++ )
			{
				AUnit* pUnit = pResults[ i ].pUnit;

				if ( pUnit == pPlayer || !pUnit->IsA( &TGetClass( ACharacterUnit ) ) || !AAnimalConversationState::CanConverse( pUnit ) )
					continue;

				TFLOAT fDistSq = TVector4::DistanceSq( vecPlayerPos, *pUnit->GetSteer()->GetPosition() );

				if ( fDistSq < fBestDistSq )
				{
					fBestDistSq = fDistSq;
					pBest       = pUnit;
				}
			}

			if ( pBest )
			{
				// Stand 1.5 in front of the unit, facing it
				TVector4 vecDir = *pBest->GetSteer()->GetDirection();
				TVector4 vecPos = *pBest->GetSteer()->GetPosition();
				vecPos.x += vecDir.x * 1.5f;
				vecPos.z += vecDir.z * 1.5f;

				TVector4 vecFace( -vecDir.x, 0.0f, -vecDir.z, 0.0f );
				pPlayer->GetSteer()->SetPosition( vecPos );
				pPlayer->GetSteer()->SetDirection( vecFace );

				TINFO( "[devtalk] placed the player in front of %s (id %d) at (%.2f %.2f %.2f)\n", pBest->GetType()->GetName().GetString(), pBest->GetId(), vecPos.x, vecPos.y, vecPos.z );
			}
			else
			{
				TWARN( "[devtalk] no unit to talk to\n" );
			}
		}

		if ( s_fTalkTime >= s_fNextAction )
		{
			s_fNextAction = s_fTalkTime + 12.0f;

			AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

			if ( pCurrentState->IsA( &TGetClass( AAnimalConversationState ) ) )
			{
				// Press OK to go to the next line of the conversation
				TINFO( "[devtalk] next line\n" );

				TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
				TBOOL                       bStopEvents = TFALSE;
				pCurrentState->ProcessCommand( AInputCommand_OK, &oEvent, bStopEvents );
			}
			else
			{
				APlayerControlComponent::DevInteract();
			}
		}

		// The log may be read after the process is killed
		fflush( stdout );
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devrock" ) )
	{
		// Development: "-devrock" puts the sim player next to the closest recipe rock (ARegrowthManager type "rock"),
		// facing it, 3 seconds after the sim has started. Use with "-devkeys" (Jump = Space, Interact = F)
		static TFLOAT s_fRockTime = 0.0f;
		static TBOOL  s_bPlaced   = TFALSE;

		s_fRockTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		if ( pPlayer && !s_bPlaced && s_fRockTime >= 3.0f )
		{
			s_bPlaced = TTRUE;

			const TVector4                  vecPlayerPos = *pPlayer->GetSteer()->GetPosition();
			ARegrowthManager::GrowthObject* pRock        = ARegrowthManager::GetSingleton()->FindClosestVisible( vecPlayerPos, ARegrowthManager::TypeMask::FromName( TPS8D( "rock" ) ), 100000.0f );

			if ( pRock )
			{
				// Stand 0.6 away from the rock on the side of the player, facing it
				TVector4 vecDir( pRock->vecPosition.x - vecPlayerPos.x, 0.0f, pRock->vecPosition.z - vecPlayerPos.z, 0.0f );
				vecDir.Normalise();

				TVector4 vecPos = pRock->vecPosition;
				vecPos.x -= vecDir.x * 0.6f;
				vecPos.z -= vecDir.z * 0.6f;

				pPlayer->GetSteer()->SetPosition( vecPos );
				pPlayer->GetSteer()->SetDirection( vecDir );

				TINFO( "[devrock] placed the player at (%.2f %.2f %.2f), rock at (%.2f %.2f %.2f), recipes %d\n", vecPos.x, vecPos.y, vecPos.z, pRock->vecPosition.x, pRock->vecPosition.y, pRock->vecPosition.z, AHintManager::GetNumRecipes() );
			}
			else
			{
				TWARN( "[devrock] no rock\n" );
			}

			fflush( stdout );
		}
	}

	const TCHAR* szDevPosArgs;
	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devpos", &szDevPosArgs ) )
	{
		// Development: "-devpos <x> <y> <z> [<dirx> <dirz> [<section>]]" puts the sim player at that position (facing that direction,
		// loading that terrain section, e.g. 32 for the Junkyard of EnvMain) 3 seconds after the sim has started and logs the position every second
		static TFLOAT s_fPosTime   = 0.0f;
		static TFLOAT s_fPosLogged = 0.0f;
		static TBOOL  s_bPosPlaced = TFALSE;

		s_fPosTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		// Held there for 7 seconds, so the terrain section there gets streamed in
		if ( pPlayer && s_fPosTime >= 3.0f && s_fPosTime < 10.0f )
		{
			TVector4 vecPos( 0.0f, 0.0f, 0.0f, 1.0f );
			TVector4 vecDir( 0.0f, 0.0f, 1.0f, 0.0f );
			sscanf( szDevPosArgs, " %f %f %f %f %f", &vecPos.x, &vecPos.y, &vecPos.z, &vecDir.x, &vecDir.z );
			vecDir.Normalise();

			pPlayer->GetSteer()->SetPosition( vecPos );
			pPlayer->GetSteer()->SetDirection( vecDir );

			if ( !s_bPosPlaced )
			{
				TINFO( "[devpos] placed the player at (%.2f %.2f %.2f)\n", vecPos.x, vecPos.y, vecPos.z );

				// The terrain section there is loaded only when the camera is above a loaded neighbour, so it's forced
				TINT iSection = -1;
				sscanf( szDevPosArgs, " %*f %*f %*f %*f %*f %d", &iSection );
				if ( iSection >= 0 ) ATerrainInterface::GetSingleton()->ForceSection( iSection, TFALSE );
			}

			s_bPosPlaced = TTRUE;
		}

		if ( pPlayer && s_bPosPlaced && s_fPosTime - s_fPosLogged >= 1.0f )
		{
			s_fPosLogged          = s_fPosTime;
			const TVector4 vecPos = *pPlayer->GetSteer()->GetPosition();
			TINFO( "[devpos] t=%.0f player at (%.2f %.2f %.2f)\n", s_fPosTime, vecPos.x, vecPos.y, vecPos.z );
			fflush( stdout );
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devwindmill" ) )
	{
		// Development: "-devwindmill" holds the sim player in the air 25 units from the windmill wmill_01 (AWindmillHelper),
		// facing it (the ground collision there isn't loaded, so the position is set again every frame)
		static TFLOAT   s_fWindmillTime   = 0.0f;
		static TBOOL    s_bWindmillPlaced = TFALSE;
		static TVector4 s_vecWindmillPos;
		static TVector4 s_vecWindmillDir;

		s_fWindmillTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		if ( pPlayer && !s_bWindmillPlaced && s_fWindmillTime >= 15.0f && AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( ASimState ) ) )
		{
			s_bWindmillPlaced = TTRUE;

			TMatrix44 matLocator;
			ATerrainInterface::GetSingleton()->GetLocatorTransform( "wmill_01", matLocator );

			TVector4 vecTarget = matLocator.GetTranslation();
			TVector4 vecPos    = *pPlayer->GetSteer()->GetPosition();
			s_vecWindmillDir   = TVector4( vecTarget.x - vecPos.x, 0.0f, vecTarget.z - vecPos.z, 0.0f );
			s_vecWindmillDir.Normalise();

			s_vecWindmillPos   = vecTarget;
			s_vecWindmillPos.x = vecTarget.x - s_vecWindmillDir.x * 25.0f;
			s_vecWindmillPos.z = vecTarget.z - s_vecWindmillDir.z * 25.0f;
			s_vecWindmillPos.y = vecTarget.y - 3.0f; // -Y is up

			TINFO( "[devwindmill] holding the player at (%.2f %.2f %.2f) facing the windmill at (%.2f %.2f %.2f)\n", s_vecWindmillPos.x, s_vecWindmillPos.y, s_vecWindmillPos.z, vecTarget.x, vecTarget.y, vecTarget.z );
			fflush( stdout );
		}

		if ( pPlayer && s_bWindmillPlaced )
		{
			pPlayer->GetSteer()->SetPosition( s_vecWindmillPos );
			pPlayer->GetSteer()->SetDirection( s_vecWindmillDir );
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devbike" ) )
	{
		// Development: "-devbike" puts the sim player next to the closest bike, gives the bike key, gets on it with the action button (the normal interaction
		// that runs the FSM native StartupPlayerBike) and rides it in circles in the open ground in front of the bike
		static TFLOAT s_fBikeTime = 0.0f;
		static TBOOL  s_bPlaced   = TFALSE;
		static TBOOL  s_bStarted  = TFALSE;

		s_fBikeTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		AUnit*        pPlayer      = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
		ABikeManager* pBikeManager = ABikeManager::GetSingleton();

		if ( pPlayer && pBikeManager->GetNumBikes() > 0 && !s_bPlaced && s_fBikeTime >= 8.0f )
		{
			s_bPlaced = TTRUE;

			AUnit* pBike     = TNULL;
			TINT   iBikeType = 0;
			pBikeManager->FindClosestBike( *pPlayer->GetSteer()->GetPosition(), pBike, iBikeType );

			if ( pBike )
			{
				// Stand 1.5 to the side of the bike
				TVector4 vecDir = *pBike->GetSteer()->GetDirection();
				TVector4 vecPos = *pBike->GetSteer()->GetPosition();
				vecPos.x += vecDir.z * 1.5f;
				vecPos.z -= vecDir.x * 1.5f;

				pPlayer->GetSteer()->SetPosition( vecPos );
				pPlayer->GetSteer()->SetDirection( vecDir );

				TINFO( "[devbike] placed the player next to the bike at (%.2f %.2f %.2f)\n", vecPos.x, vecPos.y, vecPos.z );

				// The bike's interaction needs the bike key
				AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, AInventoryComponentInterface::GetInventory( pPlayer ) );

				if ( pInventory && !pInventory->FindItemByType( TPS8( bikekey ) ) )
					pInventory->AddItems( TPS8( bikekey ), 1, AInventoryComponentInterface::ms_strDefaultInventory );
			}
			else
			{
				TWARN( "[devbike] no bike\n" );
			}
		}

		if ( s_bPlaced && !s_bStarted && s_fBikeTime >= 9.0f )
		{
			s_bStarted = TTRUE;
			TINFO( "[devbike] interact with the bike\n" );
			APlayerControlComponent::DevInteract();
		}

		static TBOOL    s_bHomeSet = TFALSE;
		static TVector4 s_vecHome;

		if ( s_bStarted )
		{
			// Keep the ride in the open ground: ride straight away from the barn wall, then circle there
			// and turn back if the bike drifts too far away
			TFLOAT fSteer = 0.0f;
			AUnit* pBike  = pBikeManager->IsPlayerOnBike() ? pBikeManager->GetPlayerBike() : TNULL;

			if ( pBike )
			{
				const TVector4& vecPos = *pBike->GetSteer()->GetPosition();
				const TVector4& vecDir = *pBike->GetSteer()->GetDirection();

				static TBOOL    s_bStartSet = TFALSE;
				static TVector4 s_vecStart;

				if ( !s_bStartSet )
				{
					s_bStartSet = TTRUE;
					s_vecStart  = vecPos;
				}

				// The bike is parked next to the barn wall, facing away from it: the centre of the ride
				// is the point 10 units into the open ground the bike first drives through
				if ( !s_bHomeSet && TVector4::DistanceXZ( s_vecStart, vecPos ) >= 10.0f )
				{
					s_bHomeSet = TTRUE;
					s_vecHome  = vecPos;
					TINFO( "[devbike] centre of the ride (%.2f %.2f %.2f)\n", vecPos.x, vecPos.y, vecPos.z );
				}

				TFLOAT fToX  = s_vecHome.x - vecPos.x;
				TFLOAT fToZ  = s_vecHome.z - vecPos.z;
				TFLOAT fDist = s_bHomeSet ? TMath::Sqrt( fToX * fToX + fToZ * fToZ ) : 0.0f;

				if ( s_bHomeSet && fDist > 14.0f )
				{
					// Too far away: head back (positive cross = home is to the right of the facing)
					TFLOAT fCross = ( vecDir.z * fToX - vecDir.x * fToZ ) / fDist;
					TFLOAT fDot   = ( vecDir.x * fToX + vecDir.z * fToZ ) / fDist;
					fSteer        = ( fDot < 0.0f ) ? ( fCross < 0.0f ? -1.0f : 1.0f ) : TMath::Max( -1.0f, TMath::Min( fCross * 4.0f, 1.0f ) );
				}
				else if ( s_bHomeSet )
				{
					// Circle at full lock to the right, away from the sheds left of the starting line
					fSteer = 1.0f;
				}
			}

			ABikeControlComponent::SetDevInput( fSteer, s_bHomeSet ? 0.3f : 0.6f );
		}

		static TINT s_iLastState = -1;

		if ( pBikeManager->GetState() != s_iLastState )
		{
			s_iLastState = pBikeManager->GetState();
			TINFO( "[devbike] state %d, on bike %d\n", s_iLastState, pBikeManager->IsPlayerOnBike() );
		}

		fflush( stdout );
	}

	if ( s_bStarted && Dev_HasSwitch( "-devnightbarn" ) )
	{
		// Development: "-devnightbarn" goes into the barn (ABarnExploreMinigameState) 8 seconds after the sim has started,
		// the way the barn door does: the spawn activity sets the start goal, then AGameStateController::StartGameState.
		// With "-devdayphase 4" it's the night barn (the party). "-devbarnleave <N>" leaves the barn N seconds later the
		// way its exit does (FinishGameState, back to the sim). "-devbarnloft <N>" puts the player in the loft N seconds
		// after entering the barn.
		static TFLOAT s_fBarnTime = 0.0f;
		static TINT   s_iStage    = 0; // 0: in the sim, 1: entering, 2: in the barn, 3: leaving, 4: back in the sim

		AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();
		TBOOL       bInBarn       = pCurrentState && pCurrentState->GetClass() == &TGetClass( ABarnExploreMinigameState );

		// The 8 seconds only count while the sim is the current state: "-devtier" plays the scripted sequence of the tier
		// first, and "-devdayphase" is only applied by the sim's update
		if ( s_iStage != 0 || ( pCurrentState && pCurrentState->GetClass() == &TGetClass( ASimState ) ) )
			s_fBarnTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		if ( s_iStage == 0 && s_bMainGameLoaded && s_fBarnTime >= 8.0f && APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
		{
			s_iStage = 1;
			TINFO( "[devnightbarn] entering the barn (night %d)\n", AGameTimeManager::GetSingleton()->IsItNight() );
			AGameLoader::SetPlayerStartGoal( TPS8D( "barn_entrance" ) );
			AGameStateController::GetSingleton()->StartGameState( TPS8D( "ABarnExploreMinigameState" ) );
		}
		else if ( s_iStage == 1 && bInBarn )
		{
			s_iStage    = 2;
			s_fBarnTime = 0.0f;
			TINFO( "[devnightbarn] in the barn, floor %d\n", ABarnExploreMinigameState::ms_iFloor );
		}
		else if ( s_iStage == 2 && bInBarn )
		{
			static TBOOL s_bLoft = TFALSE;

			if ( const TCHAR* szArg = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devbarnloft " ) )
			{
				if ( !s_bLoft && s_fBarnTime >= TFLOAT( atof( szArg + TStringManager::String8Length( "-devbarnloft " ) ) ) )
				{
					s_bLoft = TTRUE;

					AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
					TSTATICCAST( APlayerSteer, pPlayer->GetSteer() )->PlaceAtNamedGoal( TPS8D( "root_nightbarn_pos" ), TTRUE, TTRUE );
					TINFO( "[devnightbarn] the player is in the loft (y %.2f)\n", pPlayer->GetSteer()->GetPosition()->y );
				}
			}

			// "-devcocktails <N>": N seconds after entering the barn, the cocktails (ACookingMiniGame from the barn)
			static TBOOL s_bCocktails = TFALSE;

			if ( const TCHAR* szArg = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devcocktails " ) )
			{
				if ( !s_bCocktails && s_fBarnTime >= TFLOAT( atof( szArg + TStringManager::String8Length( "-devcocktails " ) ) ) )
				{
					s_bCocktails = TTRUE;
					TINFO( "[devnightbarn] starting the cocktails\n" );
					Dev_StartCooking();
				}
			}

			if ( const TCHAR* szArg = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devbarnleave " ) )
			{
				if ( s_fBarnTime >= TFLOAT( atof( szArg + TStringManager::String8Length( "-devbarnleave " ) ) ) )
				{
					s_iStage = 3;
					TINFO( "[devnightbarn] leaving the barn (floor %d, music stream state %d, radio %d, wurlitzer %d)\n", ABarnExploreMinigameState::ms_iFloor, AMultiplexStream::GetSingleton()->GetState(), ABarnExploreMinigameState::ms_bRadioOn, ABarnExploreMinigameState::ms_bWurlitzerOn );
					AGameLoader::SetPlayerStartGoal( TPS8D( "park2_goals_barnspawn" ) );
					AGameStateController::GetSingleton()->FinishGameState();
				}
			}
		}
		else if ( s_iStage == 3 && s_bMainGameLoaded && pCurrentState && pCurrentState->GetClass() == &TGetClass( ASimState ) )
		{
			s_iStage = 4;
			TINFO( "[devnightbarn] back in the sim\n" );
		}

		fflush( stdout );
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devhighscore" ) )
	{
		// Development: "-devhighscore" gives the player the best score of the first charts (shown by the scorebook)
		// 6 seconds after the sim has started
		static TFLOAT s_fHighScoreTime = 0.0f;
		static TBOOL  s_bAdded         = TFALSE;

		s_fHighScoreTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		if ( !s_bAdded && s_fHighScoreTime >= 6.0f && APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
		{
			s_bAdded = TTRUE;

			for ( TINT i = 0, iNumAdded = 0; i < AHighScoreManager::RECORD_STARS && iNumAdded < 8; i++ )
			{
				AHighScoreManager::Record* pRecord = AHighScoreManager::GetRecord( i );

				if ( pRecord->iHidden != 0 )
					continue;

				iNumAdded++;
				TFLOAT fScore = pRecord->aScores[ 0 ];

				if ( fScore != 0.0f )
					fScore += ( pRecord->eType == AHighScoreRecord::TYPE_INTEGER_LOW || pRecord->eType == AHighScoreRecord::TYPE_DECIMAL_LOW || pRecord->eType == AHighScoreRecord::TYPE_TIME_LOW ) ? -1.0f : 1.0f;

				AHighScoreManager::AddScore( i, fScore, TTRUE, AHighScoreManager::PLAYER_1, TTRUE );
			}

			TINFO( "[devhighscore] added the scores of the player\n" );
			fflush( stdout );
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devreplay " ) )
	{
		// Development: "-devreplay <replay>" unlocks the first two variants of the replay and replays it 6 seconds after
		// the sim has started (like its star does), then picks the second variant in AReplayVariantSelectState
		static TFLOAT s_fReplayTime = 0.0f;
		static TINT   s_iStage      = 0;

		s_fReplayTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		if ( s_iStage == 0 && s_fReplayTime >= 6.0f && APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
		{
			s_iStage = 1;

			TCHAR        szName[ 64 ];
			const TCHAR* szArg   = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devreplay " ) + TStringManager::String8Length( "-devreplay " );
			TINT         iLength = 0;

			while ( szArg[ iLength ] != '\0' && szArg[ iLength ] != ' ' && iLength < TARRAYSIZE( szName ) - 1 )
			{
				szName[ iLength ] = szArg[ iLength ];
				iLength++;
			}

			szName[ iLength ] = '\0';

			TPString8        strName( szName );
			AReplayableGame* pReplay = AReplayManager::GetSingleton()->FindReplay( strName );

			if ( pReplay )
			{
				pReplay->m_abVariants[ 0 ] = TTRUE;
				pReplay->m_abVariants[ 1 ] = TTRUE;

				TINFO( "[devreplay] replaying %s (%d variants)\n", szName, pReplay->GetNumVariants() );
				AReplayManager::GetSingleton()->ReplayMiniGame( strName );
			}
			else
			{
				TWARN( "[devreplay] no replay called %s\n", szName );
			}

			fflush( stdout );
		}
		else if ( s_iStage == 1 && s_fReplayTime >= 9.0f )
		{
			AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

			if ( pCurrentState && pCurrentState->IsA( &TGetClass( AReplayVariantSelectState ) ) )
			{
				// Down, then OK
				TInputInterface::InputEvent oEvent( AInputHandler::GetSingleton()->GetKeyboardDevice(), 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
				TBOOL                       bStopEvents = TFALSE;

				TINFO( "[devreplay] choosing the second variant\n" );
				pCurrentState->ProcessCommand( AInputCommand_Down, &oEvent, bStopEvents );
				s_iStage = 2;
			}
			else
			{
				TWARN( "[devreplay] the variant select state isn't shown\n" );
				s_iStage = 3;
			}

			fflush( stdout );
		}
		else if ( s_iStage == 2 && s_fReplayTime >= 10.0f )
		{
			AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

			if ( pCurrentState && pCurrentState->IsA( &TGetClass( AReplayVariantSelectState ) ) )
			{
				TInputInterface::InputEvent oEvent( AInputHandler::GetSingleton()->GetKeyboardDevice(), 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
				TBOOL                       bStopEvents = TFALSE;
				pCurrentState->ProcessCommand( AInputCommand_OK, &oEvent, bStopEvents );
			}

			s_iStage = 3;
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_GetMicroGame() && !Dev_GetMicroGame()->szSwitches )
	{
		// "-minigame <micro game>" of a micro game the sim adds by name (the "highsquirt" spots...): it's added
		// 8 seconds after the sim has started, the way the script function CreateAndAddChildState does
		static TFLOAT s_fMicroGameTime = 0.0f;
		static TBOOL  s_bAdded         = TFALSE;

		s_fMicroGameTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		if ( !s_bAdded && s_fMicroGameTime >= 8.0f && APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
		{
			s_bAdded = TTRUE;
			TINFO( "-minigame: adding the micro game '%s' to the sim\n", Dev_GetMicroGame()->szName );
			AGameStateController::GetSingleton()->AddChildStateByName( TPString8( Dev_GetMicroGame()->szName ) );
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devtreasure" ) )
	{
		// Development: "-devtreasure" gives the player a metal detector and starts the treasure hunt 6 seconds after
		// the sim has started (like the detector does), then digs every 4 seconds and ends it with the inventory button 30 seconds later
		static TFLOAT s_fTreasureTime = 0.0f;
		static TFLOAT s_fNextDig      = 0.0f;
		static TBOOL  s_bPushed       = TFALSE;

		s_fTreasureTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		if ( !s_bPushed && s_fTreasureTime >= 6.0f && pPlayer )
		{
			s_bPushed  = TTRUE;
			s_fNextDig = s_fTreasureTime + 4.0f;

			AUnitTransform oTransform( *pPlayer->GetSteer()->GetPosition(), TVector4::VEC_POSZ, 0, -1, -1, TTRUE );
			AUnit*         pDetector = AUnitManager::GetSingleton()->SpawnUnit( TPS8D( "metaldetector" ), &oTransform );

			if ( pDetector )
			{
				TSTATICCAST( AAnimatedUnit, pPlayer )->AttachUnit( pDetector, TPS8D( "cb_rlocator" ) );
				TINFO( "[devtreasure] starting the treasure hunt\n" );
				ATreasueHuntMicroGame::PushState( TFALSE );
			}
			else
			{
				TWARN( "[devtreasure] couldn't spawn the metal detector\n" );
			}

			fflush( stdout );
		}
		else if ( s_bPushed && s_fTreasureTime >= s_fNextDig )
		{
			s_fNextDig = s_fTreasureTime + 4.0f;

			AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

			if ( pCurrentState && pCurrentState->IsA( &TGetClass( ATreasueHuntMicroGame ) ) )
			{
				TInputInterface::InputEvent oEvent( AInputHandler::GetSingleton()->GetKeyboardDevice(), 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
				TBOOL                       bStopEvents = TFALSE;

				TBOOL bEnd = s_fTreasureTime >= 36.0f;
				TINFO( bEnd ? "[devtreasure] ending the treasure hunt\n" : "[devtreasure] digging\n" );
				pCurrentState->ProcessCommand( bEnd ? AInputCommand_Inventory : AInputCommand_Use, &oEvent, bStopEvents );
				fflush( stdout );
			}
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devgoldrush" ) )
	{
		// Development: "-devgoldrush [N]" plays the gold rush at the coin spawner N (1 by default) 6 seconds after
		// the sim has started: gives the player a token, puts them at the spawner and pushes the micro game the
		// way the interaction of its lever does. With "-devautoplay" the player then runs along the coins.
		static TFLOAT s_fCoinsTime    = 0.0f;
		static TBOOL  s_bPushed       = TFALSE;
		static TFLOAT s_fNextCoinTime = 0.0f;
		static TINT   s_iLastState    = -1;

		s_fCoinsTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		if ( !s_bPushed && s_fCoinsTime >= 6.0f && pPlayer )
		{
			s_bPushed = TTRUE;

			const TCHAR* szArg    = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devgoldrush" ) + TStringManager::String8Length( "-devgoldrush" );
			TINT         iSpawner = ( szArg[ 0 ] == ' ' && szArg[ 1 ] >= '1' && szArg[ 1 ] <= '8' ) ? szArg[ 1 ] - '1' : 0;
			AUnit*       pStump   = ACoinPickupMicroGame::GetStump( iSpawner );

			if ( pStump )
			{
				TVector4 vecPosition = *pStump->GetSteer()->GetPosition();
				vecPosition.x += 2.0f;
				pPlayer->SetSteerPosition( vecPosition );

				// "-devnotoken": the player has to be told that a token is needed
				if ( !Dev_HasSwitch( "-devnotoken" ) )
					AInventoryComponentInterface::GetInventory( pPlayer )->AddItems( TPS8D( "goldrush_token" ), 1, AInventoryComponentInterface::ms_strDefaultInventory );

				ACoinPickupMicroGame::SetSplineID( iSpawner );
				ACoinPickupMicroGame::SetNumberOfCoins( 40 );
				ACoinPickupMicroGame::SetTimeLimit( 45.0f );
				AGameStateController::GetSingleton()->PushState( new ACoinPickupMicroGame() );

				TINFO( "[devgoldrush] pushed ACoinPickupMicroGame at the spawner %d\n", iSpawner + 1 );
			}
			else
			{
				TWARN( "[devgoldrush] no coin spawner %d\n", iSpawner + 1 );
			}

			fflush( stdout );
		}

		ACoinPickupMicroGame* pGame = TSTATICCAST( ACoinPickupMicroGame, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "ACoinPickupMicroGame" ) ) );

		if ( pGame )
		{
			if ( pGame->GetState() != s_iLastState )
			{
				s_iLastState = pGame->GetState();
				TINFO( "[devgoldrush] state %d, %d coins, %d left, score %d\n", s_iLastState, pGame->GetNumCoins(), pGame->GetNumCoinsLeft(), pGame->GetScore() );
				fflush( stdout );

				if ( s_iLastState == ACoinPickupMicroGame::STATE_COLLECTING )
					s_fNextCoinTime = s_fCoinsTime + 8.0f;
			}

			// Run to the next coin every 0.3 seconds, starting 8 seconds after the coins came out
			if ( Dev_HasSwitch( "-devautoplay" ) && pGame->GetState() == ACoinPickupMicroGame::STATE_COLLECTING && s_fCoinsTime >= s_fNextCoinTime )
			{
				s_fNextCoinTime = s_fCoinsTime + 0.3f;

				for ( TINT i = 0; i < pGame->GetNumCoins(); i++ )
				{
					ACoinPickupCoinItem* pCoin = TSTATICCAST( ACoinPickupCoinItem, pGame->GetCoin( i ) );

					if ( !pCoin->IsCollected() )
					{
						pPlayer->SetSteerPosition( *pCoin->GetSteer()->GetPosition() );
						break;
					}
				}
			}
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devgophershop" ) )
	{
		// Development: "-devgophershop" opens the gopher shop 26 seconds (after "-devcheat") after the sim has started, the way
		// the activity of the gopher does (agamestatecontroller::startgamestate), see AGopherShopGame for the rest
		static TFLOAT s_fShopTime = 0.0f;

		s_fShopTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		if ( !s_bDevShopOpened && s_fShopTime >= 26.0f && APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
		{
			s_bDevShopOpened = TTRUE;
			TINFO( "[devgophershop] opening the shop\n" );
			// Close the screens on top of the sim (the message of -devcheat), the shop returns to the state it was started from
			AGameStateController::GetSingleton()->ReturnToNamedState( TPS8D( "ASimState" ) );
			AGameStateController::GetSingleton()->StartGameState( TPS8( AGopherShopGame ) );
			fflush( stdout );
		}

		static TBOOL s_bLogged = TFALSE;

		if ( s_bDevShopOpened && !s_bLogged && AGopherShopGame::ms_iDevBoughtId != -1 && APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) &&
		     !AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( AGopherShopGame ) ) )
		{
			s_bLogged = TTRUE;
			TINFO( "[devgophershop] back in the sim: GetNumBought( %d ) = %d, GetPercentBought = %d, money %d\n", AGopherShopGame::ms_iDevBoughtId, AGopherShopInterface::GetNumBought( AGopherShopGame::ms_iDevBoughtId ), AGopherShopInterface::GetPercentBought(), APlayerProfileManager::GetSingleton()->GetMainProfile()->GetDisplayedPickups() );
			fflush( stdout );
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devinventory" ) )
		Dev_UpdateInventoryTest();

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devsunglasses" ) )
		Dev_UpdateSunglasses();

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devmilkweed" ) )
		Dev_UpdateMilkweed();

	if ( s_bStarted && TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devshake " ) )
		Dev_UpdateShake();

		// (also in minigames started with -minigame, the times are then counted from the minigame start, and in the barn of "-devnightbarn")
#  ifndef TOSHI_FINAL
	if ( ( ( s_bStarted && ( s_bMainGameLoaded || Dev_HasSwitch( "-devnightbarn" ) ) && Dev_IsSimStateRequested() ) || s_bInMiniGame ) && TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devkeys " ) )
		Dev_UpdateKeys();
#  endif

	if ( s_bStarted && s_bMainGameLoaded && Dev_HasSwitch( "-devcoins" ) )
	{
		// Development: "-devcoins" spills 10 coins in front of the player 8 seconds after the sim has started
		static TFLOAT s_fCoinTime = 0.0f;

		s_fCoinTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		if ( !s_bDevCoinsSpilled && s_fCoinTime >= 8.0f && APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
		{
			s_bDevCoinsSpilled = TTRUE;

			ASteer*  pSteer      = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer();
			TVector4 vecPosition = *pSteer->GetPosition();
			vecPosition.x += pSteer->GetDirection()->x * 1.5f;
			vecPosition.z += pSteer->GetDirection()->z * 1.5f;
			vecPosition.y -= 1.0f;

			ACoinSpawner::SpawnMoney( 10, vecPosition );
			TINFO( "[devcoins] spilled 10 coins\n" );
		}
	}

	if ( s_bStarted && s_bMainGameLoaded && !s_bChangingTerrain && ( Dev_HasSwitch( "-devsave" ) || Dev_HasSwitch( "-devload" ) ) )
		Dev_UpdateSaveTest();

	if ( !s_bStarted && Dev_IsSimStateRequested() && Dev_HasSwitch( "-devload" ) )
	{
		// Load the dev save from the frontend's main menu, then start the main game from it
		static TINT s_iLoadPhase = 0;

		if ( s_iLoadPhase == 0 )
		{
			if ( !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AFrontEndMainMenuState2 ) ) )
				return;

			s_iLoadPhase = 1;
			Dev_SetupSavePath();
			TINFO( "[devload] loading the save file\n" );

			SaveLoadSKU* pSaveLoad = new SaveLoadSKU( APlayerProgress::GetSingleton(), TFALSE, TNULL );
			pSaveLoad->StartLoad();
			AGameStateController::GetSingleton()->PushState( pSaveLoad );
			return;
		}

		if ( AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( SaveLoadSKU ) ) )
			return;

		s_bStarted = TTRUE;

		APlayerProgress::SaveSlot* pSlot = APlayerProgress::GetSingleton()->GetSlot( 0 );
		TINFO( "[devload] loaded: slot occupied %d, date 0x%08x time 0x%08x, gender %d costume %d act %d\n", pSlot->bOccupied, pSlot->uiDate, pSlot->uiTime, pSlot->iGender, pSlot->iCostume, pSlot->iAct );

		if ( !pSlot->bOccupied )
		{
			TWARN( "[devload] slot 0 is empty, starting a new game\n" );
			Dev_StartNewMainGame();
			return;
		}

		APlayerProgress::GetSingleton()->SetCurrentSlotIndex( 0 );
		LoadSavedGame( TTRUE );
		return;
	}

	if ( s_bStarted || !Dev_IsSimStateRequested() )
		return;

	// Wait until the frontend's main menu is up
	if ( !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AFrontEndMainMenuState2 ) ) )
		return;

	s_bStarted = TTRUE;
	TINFO( "-simstate: loading the main game\n" );
	Dev_StartNewMainGame();
}

#endif // BARNYARD_COMMUNITY_PATCH
