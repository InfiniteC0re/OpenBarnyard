#pragma once
#include <BYardSDK/AMaterialLibraryManager.h>
#include <BYardSDK/AGUISlideshow.h>
#include <BYardSDK/ATerrainInterface.h>

#include <Toshi/T2Vector.h>

enum Hook
{
	Hook_AGUI2_MainPostRenderCallback,
	Hook_AGUISlideshow_ProcessInput,
	Hook_ATerrain_Render,
	Hook_AModelLoader_LoadTRBCallback,
	Hook_NewGameStarted,
	Hook_MaterialLibrary_LoadTTLData,
	Hook_TRenderInterface_SetLightColourMatrix,
	Hook_TOrderTable_Flush,
	Hook_NUMOF,
};

enum HookType
{
	HookType_Before,
	HookType_After,
	HookType_NUMOF
};

class MODCORE_API AHooks
{
public:
	constexpr static TUINT MAX_NUM_CALLBACKS = 16;

	struct GUI2
	{
		using t_MainPostRenderCallback = void ( * )();
		inline static Toshi::T2Vector<t_MainPostRenderCallback, MAX_NUM_CALLBACKS> MainPostRenderCallback[ HookType_NUMOF ];
	};

	struct GUISlideshow
	{
		using t_ProcessInput = TBOOL ( * )( AGUISlideshow* a_pSlideshow, Toshi::TInputInterface::InputEvent* a_pEvent );
		inline static Toshi::T2Vector<t_ProcessInput, MAX_NUM_CALLBACKS> ProcessInput[ HookType_NUMOF ];
	};

	struct Terrain
	{
		using t_Render = void ( * )( ATerrainInterface* a_pTerrain );
		inline static Toshi::T2Vector<t_Render, MAX_NUM_CALLBACKS> Render[ HookType_NUMOF ];
	};

	struct ModelLoader
	{
		using t_LoadTRBCallback = TBOOL ( * )( Toshi::TModel* a_pModel );
		inline static Toshi::T2Vector<t_LoadTRBCallback, MAX_NUM_CALLBACKS> LoadTRBCallback[ HookType_NUMOF ];
	};

	struct MaterialLibrary
	{
		using t_LoadTTLData = TBOOL ( * )( AMaterialLibrary* a_pMatLib, AMaterialLibrary::TTL* a_pTTL );
		inline static Toshi::T2Vector<t_LoadTTLData, MAX_NUM_CALLBACKS> LoadTTLData[ HookType_NUMOF ];
	};

	struct RenderInterface
	{
		using t_SetLightColourMatrix = void                                      ( * )( Toshi::TRenderInterface* a_pRenderInterface, Toshi::TMatrix44* a_pLightColour );
		inline static Toshi::T2Vector<t_SetLightColourMatrix, MAX_NUM_CALLBACKS> SetLightColourMatrix[ HookType_NUMOF ];
	};

	struct OrderTable
	{
		using t_Flush = void                                      ( * )( Toshi::TOrderTable* a_pOrderTable );
		inline static Toshi::T2Vector<t_Flush, MAX_NUM_CALLBACKS> Flush[ HookType_NUMOF ];
	};

	struct Uncategorized
	{
		using t_NewGameStarted = void ( * )();
		inline static Toshi::T2Vector<t_NewGameStarted, MAX_NUM_CALLBACKS> NewGameStarted[ HookType_NUMOF ];
	};

public:
	static void Initialise();
	TBOOL static AddHook( Hook a_eHook, HookType a_eHookType, void* a_pCallback );
};
