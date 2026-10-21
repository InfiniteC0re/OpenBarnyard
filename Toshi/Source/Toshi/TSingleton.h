#pragma once
#include <type_traits>

TOSHI_NAMESPACE_START

#define TSINGLETON_DECLARE_ALIAS( CLASS, NAME )               \
	inline static struct SingletonHelper_##NAME               \
	{                                                         \
		CLASS* operator->() { return CLASS::GetSingleton(); } \
		operator CLASS*() { return CLASS::GetSingleton(); }   \
	} g_p##NAME

#define TSINGLETON_DECLARE_INHERITED_ALIAS( PARENT, CLASS, NAME )                    \
	inline static struct SingletonHelper_##NAME                                      \
	{                                                                                \
		CLASS* operator->() { return TSTATICCAST( CLASS, PARENT::GetSingleton() ); } \
		operator CLASS*() { return TSTATICCAST( CLASS, PARENT::GetSingleton() ); }   \
	} g_p##NAME

#ifdef MODAPI_MOD
#  define TSINGLETON_INLINE TNOINLINE
#else
#  define TSINGLETON_INLINE TFORCEINLINE
#endif

template <typename T>
class TSingleton
{
public:
	TSingleton( const TSingleton& )             = delete;
	TSingleton( const TSingleton&& )            = delete;
	TSingleton& operator=( const TSingleton& )  = delete;
	TSingleton& operator=( const TSingleton&& ) = delete;

	// Creates Singleton instance and returns it
	template <typename... Args>
	TSINGLETON_INLINE static T* CreateSingleton( Args&&... args )
	{
		return new T( std::forward<Args>( args )... );
	}

	// Creates Singleton instance for a derived class and returns it
	template <typename D, typename... Args>
	TSINGLETON_INLINE static D* CreateSingleton( Args&&... args )
	{
		TSTATICASSERT( std::is_base_of_v<T, D> == TTRUE );
		return new D( std::forward<Args>( args )... );
	}

	TSINGLETON_INLINE static void DestroySingleton()
	{
		delete GetSingletonSafe();
		ms_pSingleton = TNULL;
	}

	// Returns pointer and asserts if it's not allocated
	TSINGLETON_INLINE static T* GetSingletonSafe()
	{
		TASSERT( ms_pSingleton != TNULL, "Singleton instance is not created" );
		return ms_pSingleton;
	}

	TSINGLETON_INLINE static TBOOL IsSingletonCreated()
	{
		return ms_pSingleton != TNULL;
	}

	// Returns pointer even if it's not allocated
	TSINGLETON_INLINE static T* GetSingleton()
	{
		return ms_pSingleton;
	}

#if defined( TOSHI_MODLOADER ) || defined( TOSHI_MODLOADER_CLIENT )
	TSINGLETON_INLINE static T* SetSingletonExplicit( T* a_pPtr )
	{
		ms_pSingleton = a_pPtr;
		return ms_pSingleton;
	}
#endif

protected:
	TSINGLETON_INLINE TSingleton()
	{
		TASSERT( ms_pSingleton == TNULL, "Singleton instance is already created" );
		ms_pSingleton = TSTATICCAST( T, this );
	}

	// NOTE: retail keeps out-of-line instances of this destructor (mov ms_pSingleton, 0; ret), called from the derived
	// destructors and constructor unwind code. Instances in 0x560000-0x6dd6ff, in marker order:
	// ASteerPool, AKinematicsObstructionManager, AKinematicsPool, AAnimatableObjectManager, AControlComponentManager,
	// AInputHandler, AInputMapManager, AGUISystem, AInteractionComponentManager, AInteractionManager, AInteractionEventManager,
	// AQuestManager, AUnitFlagSetsManager, APlayerKnowledgeManager, AMultiplexStream, AMusicManager, ASoundManager, AGateManager,
	// AInstanceManager, ARegrowthManager, ATerrainInterface, ATreeManager, AFSMManager, AMoviePlayer, AAssetStreaming,
	// AKeyFrameLibraryManager, AInstanceLightingHelper, AGlowViewport, ARenderer, AShadowManager, ASkinLightingManager,
	// AModelRepos, AMaterialLibraryManager, APlayerManager, APlayerProfileManager, ASSeqController, AHackedQuickSave,
	// AAniEffectManager, ANPCBehaviourManager, AGUI2, AGUI2ModelRendererScreen, AParticleEffectLocatorManager,
	// AParticleEffectManager, APlayerProgress, AVehicleSkidManager, AOptions, ASimpleProjectileManager, ASubtitleManager
	// $Barnyard: FUNCTION 00572110
	// $Barnyard: FUNCTION 00573860
	// $Barnyard: FUNCTION 00574020
	// $Barnyard: FUNCTION 0057d380
	// $Barnyard: FUNCTION 0058c5c0
	// $Barnyard: FUNCTION 00595a40
	// $Barnyard: FUNCTION 00596df0
	// $Barnyard: FUNCTION 0059dea0
	// $Barnyard: FUNCTION 005a04f0
	// $Barnyard: FUNCTION 005a2150
	// $Barnyard: FUNCTION 005c5be0
	// $Barnyard: FUNCTION 005cc040
	// $Barnyard: FUNCTION 005d20b0
	// $Barnyard: FUNCTION 005d2730
	// $Barnyard: FUNCTION 005d3cc0
	// $Barnyard: FUNCTION 005d45f0
	// $Barnyard: FUNCTION 005d6330
	// $Barnyard: FUNCTION 005dd990
	// $Barnyard: FUNCTION 005dfcf0
	// $Barnyard: FUNCTION 005e26f0
	// $Barnyard: FUNCTION 005e8370
	// $Barnyard: FUNCTION 005edf30
	// $Barnyard: FUNCTION 005ffa90
	// $Barnyard: FUNCTION 006056e0
	// $Barnyard: FUNCTION 006066d0
	// $Barnyard: FUNCTION 00606be0
	// $Barnyard: FUNCTION 00607580
	// $Barnyard: FUNCTION 006085c0
	// $Barnyard: FUNCTION 0060afe0
	// $Barnyard: FUNCTION 0060cb70
	// $Barnyard: FUNCTION 0060dd60
	// $Barnyard: FUNCTION 006122f0
	// $Barnyard: FUNCTION 006158f0
	// $Barnyard: FUNCTION 0061fb30
	// $Barnyard: FUNCTION 00620110
	// $Barnyard: FUNCTION 00629cf0
	// $Barnyard: FUNCTION 0062e460
	// $Barnyard: FUNCTION 0062f9d0
	// $Barnyard: FUNCTION 00634840
	// $Barnyard: FUNCTION 00635400
	// $Barnyard: FUNCTION 006457b0
	// $Barnyard: FUNCTION 006505b0
	// $Barnyard: FUNCTION 00650c20
	// $Barnyard: FUNCTION 00651c20
	// $Barnyard: FUNCTION 00662660
	// $Barnyard: FUNCTION 00662e60
	// $Barnyard: FUNCTION 0066d160
	// $Barnyard: FUNCTION 0066e350
	// Out-of-line instantiations of the destructor (mov ms_pSingleton, 0; ret) in 0x401000-0x55ffff:
	// ALoadTimeProfiler 004024b0, ARandom 004029c0, ARootTask 00402bc0, AUnitManager 0040f600, AUnitNameManager 00411d90,
	// AHighScoreManager 0041c3b0, AHintManager 0041d280, ACheatActivator 00421c30, AGameStateController 00428b90,
	// ACameraManager 0045b950, AFadeManager 0045c580, AMiniGameManager 00469140, ACowTippingMiniGameState 004a7c20,
	// ATapperMiniGameState 004df320, ATreasureManager 00530dd0, AAnimalPopulationManager 0053a070, ABikeManager 005401c0,
	// AConversationManager 00543ed0, AGameTimeFXManager 00544650, AGameTimeManager 005462a0, AReplayManager 00547080,
	// ASimPersistData 0054b4d0, ASpeechDialogManager 0054ba20, AWeatherManager 0054c5b0, AInteriorNetworkManager 00555220
	// $Barnyard: FUNCTION 004024b0
	// $Barnyard: FUNCTION 004029c0
	// $Barnyard: FUNCTION 00402bc0
	// $Barnyard: FUNCTION 0040f600
	// $Barnyard: FUNCTION 00411d90
	// $Barnyard: FUNCTION 0041c3b0
	// $Barnyard: FUNCTION 0041d280
	// $Barnyard: FUNCTION 00421c30
	// $Barnyard: FUNCTION 00428b90
	// $Barnyard: FUNCTION 0045b950
	// $Barnyard: FUNCTION 0045c580
	// $Barnyard: FUNCTION 00469140
	// $Barnyard: FUNCTION 004a7c20
	// $Barnyard: FUNCTION 004df320
	// $Barnyard: FUNCTION 00530dd0
	// $Barnyard: FUNCTION 0053a070
	// $Barnyard: FUNCTION 005401c0
	// $Barnyard: FUNCTION 00543ed0
	// $Barnyard: FUNCTION 00544650
	// $Barnyard: FUNCTION 005462a0
	// $Barnyard: FUNCTION 00547080
	// $Barnyard: FUNCTION 0054b4d0
	// $Barnyard: FUNCTION 0054ba20
	// $Barnyard: FUNCTION 0054c5b0
	// $Barnyard: FUNCTION 00555220
	TSINGLETON_INLINE ~TSingleton() { ms_pSingleton = TNULL; }

	static T* ms_pSingleton;
};

template <typename T>
T* TSingleton<T>::ms_pSingleton = TNULL;

#undef TSINGLETON_INLINE

TOSHI_NAMESPACE_END
