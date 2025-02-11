#include "pch.h"
#include "AHooks.h"
#include "AImGUI.h"
#include "HookHelpers.h"
#include "AModLoaderTask.h"
#include "ACoreSettings.h"

#include <BYardSDK/AGameStateController.h>
#include <BYardSDK/THookedRenderD3DInterface.h>
#include <BYardSDK/AAssetLoader.h>
#include <BYardSDK/AGUI2.h>
#include <BYardSDK/ARenderer.h>
#include <BYardSDK/ACamera.h>
#include <BYardSDK/THookedSingleton.h>
#include <BYardSDK/THookedMemory.h>

#include <Input/TInputDeviceKeyboard.h>
#include <Render/TCameraObject.h>
#include <Render/TModel.h>
#include <Render/TShader.h>
#include <Platform/Windows/TMSWindow.h>
#include <Platform/DX8/TRenderInterface_DX8.h>

TOSHI_NAMESPACE_USING

TUINT  g_uiWindowWidth  = 0;
TUINT  g_uiWindowHeight = 0;
TBOOL  g_bBikeFOVPatch  = TFALSE;
TFLOAT g_fOriginalFOV   = 0.0f;

// Replacing this method since it's all useless now but can crash the game
HOOK( 0x006b4cb0, TMemory_UnkMethod, TBOOL, TMemory::MemBlock* a_pMemBlock )
{
#ifdef TMEMORY_USE_DLMALLOC

	return TTRUE;

#else // TMEMORY_USE_DLMALLOC

	return CallOriginal( a_pMemBlock );

#endif
}

HOOK( 0x006b5740, TMemory_Initialise, TBOOL, TUINT a_uiHeapSize, TUINT a_uiReservedSize, TUINT a_uiUnused )
{
#ifdef TMEMORY_USE_DLMALLOC

	// Allocate TMemory
	TMemory* pMemory = (TMemory*)GlobalAlloc( GMEM_ZEROINIT, sizeof( TMemory ) );

	new ( pMemory ) TMemory();
	THookedMemory::SetSingleton( pMemory );

	// Allocate TMemory_dlmalloc
	TMemoryDL* pMemModule = (TMemoryDL*)GlobalAlloc( GMEM_ZEROINIT, sizeof( TMemoryDL ) );

	// Set minimum heap size
	if ( a_uiHeapSize == 0 )
		a_uiHeapSize = 256 * 1024 * 1024;

	new ( pMemModule ) TMemoryDL( TMemoryDL::Flags_Standard, a_uiHeapSize + a_uiReservedSize );
	pMemory->SetMemModule( pMemModule );

	// Initialise the memory block
	TBOOL bInitialised = pMemModule->Init() == TMemoryDL::Error_Ok;

	pMemory->m_pGlobalBlock = pMemModule->GetHeap();

	return bInitialised;

#else // TMEMORY_USE_DLMALLOC

	return CallOriginal( a_uiHeapSize, a_uiReservedSize, a_uiUnused );

#endif
}

MEMBER_HOOK( 0x006b5510, TMemory, TMemory_CreateMemBlock, TMemory::MemBlock*, TUINT a_uiSize, const TCHAR* a_szName, TMemory::MemBlock* a_pOwnerBlock, TINT a_iUnused )
{
#ifdef TMEMORY_USE_DLMALLOC

	TMemoryDL* pMemModule = THookedMemory::GetSingleton()->GetMemModule();

	if ( !a_pOwnerBlock )
		a_pOwnerBlock = pMemModule->GetHeap();

	return pMemModule->dlheapcreatesubheap( a_pOwnerBlock, a_uiSize + 1024 * 4, TMemoryHeapFlags_UseMutex, a_szName );

#else // TMEMORY_USE_DLMALLOC

	return CallOriginal( a_uiSize, a_szName, a_pOwnerBlock, a_iUnused );

#endif // !TMEMORY_USE_DLMALLOC
}

MEMBER_HOOK( 0x006b5090, TMemory, TMemory_DestroyMemBlock, void, TMemory::MemBlock* a_pMemBlock )
{
#ifdef TMEMORY_USE_DLMALLOC

	TMemoryDL* pMemModule = THookedMemory::GetSingleton()->GetMemModule();
	pMemModule->dlheapdestroy( a_pMemBlock );

#else // TMEMORY_USE_DLMALLOC

	CallOriginal( a_pMemBlock );

#endif // !TMEMORY_USE_DLMALLOC
}

HOOK( 0x006b4ba0, TMemory_GetMemInfo, void, TMemory::MemInfo* a_pMemInfo, TMemory::MemBlock* a_pMemBlock )
{
#ifdef TMEMORY_USE_DLMALLOC

	THookedMemory::GetSingleton()->GetMemInfo( *a_pMemInfo, a_pMemBlock );

#else // TMEMORY_USE_DLMALLOC

	CallOriginal( a_pMemInfo, a_pMemBlock );

#endif // !TMEMORY_USE_DLMALLOC
}

MEMBER_HOOK( 0x006b4b80, TMemory, TMemory_SetGlobalBlock, TMemory::MemBlock*, TMemory::MemBlock* a_pMemBlock )
{
#ifdef TMEMORY_USE_DLMALLOC

	TMemoryDL* pMemModule = THookedMemory::GetSingleton()->GetMemModule();
	MemBlock*  pOldHeap   = pMemModule->GetHeap();

	m_pGlobalBlock = a_pMemBlock;
	pMemModule->SetHeap( a_pMemBlock );

	return pOldHeap;

#else // TMEMORY_USE_DLMALLOC

	return CallOriginal( a_pMemBlock );

#endif // !TMEMORY_USE_DLMALLOC
}

MEMBER_HOOK( 0x006b4a20, TMemory, TMemory_Free, TBOOL, void* a_pMem )
{
#ifdef TMEMORY_USE_DLMALLOC

	TMemoryDL* pMemModule = THookedMemory::GetSingleton()->GetMemModule();
	pMemModule->GetContext().Free( a_pMem );

	return TTRUE;

#else // TMEMORY_USE_DLMALLOC

	return CallOriginal( a_pMem );

#endif // !TMEMORY_USE_DLMALLOC
}

MEMBER_HOOK( 0x006b5230, TMemory, TMemory_Alloc, void*, TUINT a_uiSize, TUINT a_uiAlignment, TMemory::MemBlock* a_pMemBlock, const char* a_szUnused1, TINT a_iUnused2 )
{
#ifdef TMEMORY_USE_DLMALLOC

	if ( a_uiSize < 4 )
		a_uiSize = 4;

	if ( a_uiAlignment < 16 )
		a_uiAlignment = 16;

	if ( a_pMemBlock == TNULL )
		a_pMemBlock = THookedMemory::GetSingleton()->GetMemModule()->GetHeap();

	return a_pMemBlock->Memalign( a_uiAlignment, a_uiSize );

#else // TMEMORY_USE_DLMALLOC

	return CallOriginal( a_uiSize, a_uiAlignment, a_pMemBlock, a_szUnused1, a_iUnused2 );

#endif // !TMEMORY_USE_DLMALLOC
}

class ACollisionObjectModel
{
	TCHAR PADDING[ 0xA4 ];
};
class AInstanceManager_CollObjectModel : public ACollisionObjectModel
    , public T2SList<AInstanceManager_CollObjectModel>::Node
{
};

MEMBER_HOOK( 0x005dfac0, AInstanceManager_CollObjectModel, CollObjectModel_DCTOR, void )
{
	TCHAR* pInstanceManager = *(TCHAR**)0x0078deb0;

	auto pList1 = (T2SList<AInstanceManager_CollObjectModel>*)( pInstanceManager + 0xefa0 );
	auto pList2 = (T2SList<AInstanceManager_CollObjectModel>*)( pInstanceManager + 0xefa4 );

	// HACK [7/18/2024 InfiniteC0re]
	// For some reason the original game doesn't clear both the lists before destroying this
	// type of objects and it can cause the game to crash during unloading a level sometimes.
	//
	// To reproduce, start chicken launch or sharp squirter from the antics menu and return
	// back to the menu and the game will crash
	//
	// Chasing Chicks crash is NOT related to this one!

	pList1->Reset();
	pList2->Reset();

	CallOriginal();
}

MEMBER_HOOK( 0x006ce820, Toshi::TModel, TModel_LoadTRB, TBOOL, const TCHAR* a_szFileName, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen )
{
	TBOOL bResult = CallOriginal( a_szFileName, a_pAssetTRB, a_ui8FileNameLen );

	if ( m_bIsAssetFile )
	{
		// Trying to fix careless memory handling that sometimes causes crashes in the original game
		m_szSymbolPrefix = T2String8::CreateCopy( a_szFileName );
	}

	return bResult;
}

MEMBER_HOOK( 0x006ce780, Toshi::TModel, TModel_UnloadTRB, void )
{
	CallOriginal();

	if ( m_bIsAssetFile )
	{
		// Out patch creates a copy of the symbol string so need to deallocate it here
		delete[] m_szSymbolPrefix;
	}
}

class AOptions
{};

MEMBER_HOOK( 0x00662d90, AOptions, AOptions_IsResolutionCompatible, TBOOL, TINT a_iWidth, TINT a_iHeight )
{
	TBOOL* pIsWindowed = (TBOOL*)( ( TUINT( this ) + 0x20 ) );
	TINT*  pWidth      = (TINT*)( ( TUINT( this ) + 0x24 ) );
	TINT*  pHeight     = (TINT*)( ( TUINT( this ) + 0x28 ) );

	if ( *pIsWindowed == TFALSE )
	{
		RECT windowRect;
		GetWindowRect( GetDesktopWindow(), &windowRect );

		if ( windowRect.right > a_iWidth || windowRect.bottom > a_iHeight )
		{
			const int msgBoxResult = MessageBoxW( NULL, L"It seems that you aren't using widescreen patch.\nDo you want the game to launch in your native resolution?", L"Barnyard", MB_YESNO );

			if ( msgBoxResult == IDYES )
			{
				a_iWidth  = windowRect.right;
				a_iHeight = windowRect.bottom;

				HKEY    hkey;
				LSTATUS status = RegCreateKeyExA( HKEY_CURRENT_USER, "Software\\THQ\\Barnyard", NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL );

				if ( status == 0 )
				{
					RegSetValueExA( hkey, "Width", NULL, REG_DWORD_LITTLE_ENDIAN, (BYTE*)&a_iWidth, sizeof( a_iWidth ) );
					RegSetValueExA( hkey, "Height", NULL, REG_DWORD_LITTLE_ENDIAN, (BYTE*)&a_iHeight, sizeof( a_iHeight ) );
					RegCloseKey( hkey );

					TINFO( "Applied widescreen patch.\n" );
				}
				else
				{
					TERROR( "Unable to open Software\\THQ\\Barnyard for writing (Status: %d)\n", status );
				}
			}
		}
	}

	constexpr TFLOAT ASPECT_RATIO_5_4   = 5.0f / 4.0f;
	constexpr TFLOAT ASPECT_RATIO_25_16 = 25.0f / 16.0f;
	constexpr TFLOAT ASPECT_RATIO_16_10 = 16.0f / 10.0f;
	constexpr TFLOAT ASPECT_RATIO_15_9  = 15.0f / 9.0f;
	constexpr TFLOAT ASPECT_RATIO_16_9  = 16.0f / 9.0f;

	TFLOAT  fCurrentAspectRatio = TFLOAT( a_iWidth ) / TFLOAT( a_iHeight );
	TFLOAT* pFOV                = (TFLOAT*)0x007822ac;

	g_fOriginalFOV = *pFOV;

	if ( TMath::Abs( fCurrentAspectRatio - ASPECT_RATIO_5_4 ) <= FLT_EPSILON )
	{
		*pFOV = 0.994199f;
	}
	else if ( TMath::Abs( fCurrentAspectRatio - ASPECT_RATIO_25_16 ) <= FLT_EPSILON )
	{
		*pFOV = 1.18425f;
	}
	else if ( TMath::Abs( fCurrentAspectRatio - ASPECT_RATIO_16_10 ) <= FLT_EPSILON )
	{
		*pFOV           = 1.2244f;
		g_bBikeFOVPatch = TTRUE;
	}
	else if ( TMath::Abs( fCurrentAspectRatio - ASPECT_RATIO_15_9 ) <= FLT_EPSILON )
	{
		*pFOV           = 1.24655f;
		g_bBikeFOVPatch = TTRUE;
	}
	else if ( TMath::Abs( fCurrentAspectRatio - ASPECT_RATIO_16_9 ) <= FLT_EPSILON )
	{
		*pFOV           = 1.313f;
		g_bBikeFOVPatch = TTRUE;
	}

	*pWidth  = a_iWidth;
	*pHeight = a_iHeight;

	g_uiWindowWidth  = a_iWidth;
	g_uiWindowHeight = a_iHeight;

	return TFALSE;
}

class ADisplayModes_Win
{};

MEMBER_HOOK( 0x00614d70, ADisplayModes_Win, ADisplayModes_Win_DoesModeExist, TBOOL, TINT& a_rWidth, TINT& a_rHeight, TUINT& a_rColourDepth )
{
	return TTRUE;
}

MEMBER_HOOK( 0x006c66b0, TRenderD3DInterface, TRenderD3DInterface_UpdateColourSettings, void )
{
	return;
}

MEMBER_HOOK( 0x006bb000, TTRB, TTRB_Load, TINT, const char* a_szFileName, TUINT a_uiUnk )
{
	TString8 filepath = a_szFileName;

	for ( TINT i = 0; i < filepath.Length(); i++ )
		if ( filepath[ i ] == '/' ) filepath[ i ] = '\\';

	filepath.MakeLower();

	auto  pModLoaderTask = AGlobalModLoaderTask::Get();
	auto  pMods          = &pModLoaderTask->GetMods();
	TBOOL bFound         = TFALSE;

	for ( auto it = pMods->Begin(); it != pMods->End(); it++ )
	{
		auto pFileOverrides = it->GetFileOverrides();

		if ( pFileOverrides != TNULL )
		{
			auto pOrigFileName = pFileOverrides->GetProperty( filepath );

			if ( pOrigFileName )
			{
				filepath = GetModsDirectory();
				filepath += pOrigFileName->GetString();
				bFound = TTRUE;
				break;
			}
		}

		if ( bFound ) break;
	}

	return CallOriginal( filepath, a_uiUnk );
}

MEMBER_HOOK( 0x006cd220, TCameraObject, TCameraObject_SetFOV, TFLOAT, TFLOAT a_fFOV )
{
	if ( g_bBikeFOVPatch )
	{
		if ( a_fFOV != g_fOriginalFOV )
		{
			//a_fFOV *= 1.04f;
		}

		//a_fFOV = g_fOriginalFOV;
	}

	return CallOriginal( a_fFOV );
}

MEMBER_HOOK( 0x006c1d40, TModelManager, TModelRegistry_CreateModel, TModelManager::ModelNode*, const char* a_szFileName, TManagedModel& a_rModelRef, TTRB* a_pAssetTRB )
{
	TString8 filepath = a_szFileName;

	for ( TINT i = 0; i < filepath.Length(); i++ )
		if ( filepath[ i ] == '/' ) filepath[ i ] = '\\';

	TString8 inputFileName = filepath.GetString( filepath.FindReverse( '\\', -1 ) + 1 );
	inputFileName.Truncate( inputFileName.FindReverse( '.', -1 ) );

	auto  pModLoaderTask = AGlobalModLoaderTask::Get();
	auto  pMods          = &pModLoaderTask->GetMods();
	TTRB* pFoundAsset    = TNULL;

	for ( auto it = pMods->Begin(); it != pMods->End(); it++ )
	{
		auto pAsset = it->GetAssetFile();

		if ( pAsset != TNULL )
		{
			auto iNumSymbols = pAsset->GetNumSymbols();

			for ( TINT i = 0; i < iNumSymbols; i++ )
			{
				auto szSymbolName = pAsset->GetSymbolName( i );

				if ( TStringManager::String8FindString( szSymbolName, inputFileName ) == szSymbolName )
				{
					pFoundAsset = pAsset;
					break;
				}
			}
		}

		if ( pFoundAsset ) break;
	}

	return CallOriginal( a_szFileName, a_rModelRef, pFoundAsset ? pFoundAsset : a_pAssetTRB );
}

MEMBER_HOOK( 0x006bf6b0, TRenderInterface, TRenderInterface_SetLightColourMatrix, void, TMatrix44* a_pLightColour )
{
	for ( TINT i = 0; i < AHooks::RenderInterface::SetLightColourMatrix[ HookType_Before ].Size(); i++ )
	{
		AHooks::RenderInterface::SetLightColourMatrix[ HookType_Before ][ i ]( this, a_pLightColour );
	}

	CallOriginal( a_pLightColour );

	for ( TINT i = 0; i < AHooks::RenderInterface::SetLightColourMatrix[ HookType_After ].Size(); i++ )
	{
		AHooks::RenderInterface::SetLightColourMatrix[ HookType_After ][ i ]( this, a_pLightColour );
	}
}

class AFrontEndMiniGameState2
{};

MEMBER_HOOK( 0x00409ce0, AFrontEndMiniGameState2, AFrontEndMiniGameState2_CTOR, void, TBOOL a_bHideVariantSelector )
{
	CallOriginal( TFALSE );
}

MEMBER_HOOK( 0x006c6de0, TRenderD3DInterface, TRenderD3DInterface_OnD3DDeviceLost, void )
{
	CallOriginal();
	AImGUI::GetSingleton()->OnD3DDeviceLost();
}

MEMBER_HOOK( 0x006c6e80, TRenderD3DInterface, TRenderD3DInterface_OnD3DDeviceFound, void )
{
	CallOriginal();
	AImGUI::GetSingleton()->OnD3DDeviceFound();
}

MEMBER_HOOK( 0x006154c0, ARenderer, ARenderer_CreateTRender, TBOOL )
{
	TBOOL bResult = CallOriginal();

	TRenderInterface::SetSingletonExplicit(
	    THookedRenderD3DInterface::GetSingleton()
	);

	AGlobalModLoaderTask::Get()->OnRenderInterfaceReady();

	return bResult;
}

MEMBER_HOOK( 0x0060c7c0, ARenderer, ARenderer_OnCreate, TBOOL )
{
	TBOOL bResult = CallOriginal();

	AGlobalModLoaderTask::Get()->OnAppRendererReady();

	return bResult;
}

// NOTE: this is more optimised version of the method which should work a little bit faster to improve loading times
TBOOL MaterialLibrary_LoadTTLData( AMaterialLibrary* a_pMatLib, AMaterialLibrary::TTL* a_pTTLData )
{
	auto pTTL = TSTATICCAST( AMaterialLibrary::TTL, a_pTTLData );

	auto pLibList     = AMaterialLibraryManager::List::GetSingleton();
	TINT iNumTextures = 0;

	if ( AMaterialLibrary::ms_bSkipLoadedTextures )
	{
		for ( TINT i = 0; i < pTTL->m_iNumTextures; i++ )
		{
			if ( !pLibList->FindTexture( pTTL->m_pTextureInfos[ i ].m_szFileName, TNULL, TNULL ) )
			{
				iNumTextures++;
			}
		}
	}
	else
	{
		iNumTextures = pTTL->m_iNumTextures;
	}

	a_pMatLib->m_pTexturesArray = new ATexture[ iNumTextures ];
	a_pMatLib->m_pTextures      = a_pMatLib->m_pTexturesArray;
	a_pMatLib->m_iNumTextures   = iNumTextures;

	// Calculate maximum texture size to preallocate a buffer
	TSIZE uiMaxTextureSize = 0;

	for ( TINT i = 0; i < iNumTextures; i++ )
	{
		auto pTexInfo = &pTTL->m_pTextureInfos[ i ];

		if ( !AMaterialLibrary::ms_bSkipLoadedTextures || !pLibList->FindTexture( pTexInfo->m_szFileName, TNULL, TNULL ) )
		{
			if ( pTexInfo->m_bIsT2Texture == TRUE )
			{
				uiMaxTextureSize = TMath::Max( pTexInfo->m_uiDataSize, uiMaxTextureSize );
			}
		}
	}

	void* pTexData = TMalloc( uiMaxTextureSize );

	for ( TINT i = 0; i < iNumTextures; i++ )
	{
		auto pTexInfo = &pTTL->m_pTextureInfos[ i ];

		if ( !AMaterialLibrary::ms_bSkipLoadedTextures || !pLibList->FindTexture( pTexInfo->m_szFileName, TNULL, TNULL ) )
		{
			TASSERT( pTexInfo->m_bIsT2Texture == TRUE, "No support of other texture types" );
			a_pMatLib->m_pTextures[ i ].Name = pTexInfo->m_szFileName;

			if ( pTexInfo->m_bIsT2Texture == TRUE )
			{
				auto pTexture = new Toshi::T2Texture;

				if ( pTexture )
				{
					Toshi::TUtil::MemCopy( pTexData, pTexInfo->m_pData, pTexInfo->m_uiDataSize );
					pTexture->SetData( pTexData, pTexInfo->m_uiDataSize );
					pTexture->Load();

					// Make sure pointer to the buffer is not stored in the texture anymore
					pTexture->SetData( TNULL, 0 );
				}

				a_pMatLib->m_pTextures[ i ].pTexture = pTexture;
			}
		}
	}

	TFree( pTexData );

	return TTRUE;
}

MEMBER_HOOK( 0x00615d20, AMaterialLibrary, AMaterialLibrary_LoadTTLData, TBOOL, AMaterialLibrary::TTL* a_pTTL )
{
	for ( TINT i = 0; i < AHooks::MaterialLibrary::LoadTTLData[ HookType_Before ].Size(); i++ )
	{
		if ( AHooks::MaterialLibrary::LoadTTLData[ HookType_Before ][ i ]( this, a_pTTL ) )
		{
			return TTRUE;
		}
	}

	if ( !MaterialLibrary_LoadTTLData( this, a_pTTL ) )
	{
		for ( TINT i = 0; i < AHooks::MaterialLibrary::LoadTTLData[ HookType_After ].Size(); i++ )
		{
			if ( AHooks::MaterialLibrary::LoadTTLData[ HookType_After ][ i ]( this, a_pTTL ) )
			{
				return TTRUE;
			}
		}

		return TFALSE;
	}

	return TTRUE;
}

MEMBER_HOOK( 0x006da4d0, TMSWindow, TMSWindow_SetPosition, void, TUINT x, TUINT y, TUINT width, TUINT height )
{
	if ( IsWindowed() )
	{
		// Fix window size when in windowed mode
		RECT rect;
		rect.left   = x;
		rect.top    = y;
		rect.right  = width;
		rect.bottom = height;

		if ( TRUE == AdjustWindowRectEx( &rect, GetWindowLongA( GetHWND(), GWL_STYLE ), FALSE, GetWindowLongA( GetHWND(), GWL_EXSTYLE ) ) )
		{
			x      = rect.left;
			y      = rect.top;
			width  = rect.right;
			height = rect.bottom;
		}
	}

	CallOriginal( x, y, width, height );
}

MEMBER_HOOK( 0x0059dac0, AGUISlideshow, AGUISlideshow_ProcessInput, TBOOL, TInputInterface::InputEvent* a_pEvent )
{
	for ( TINT i = 0; i < AHooks::GUISlideshow::ProcessInput[ HookType_Before ].Size(); i++ )
	{
		if ( AHooks::GUISlideshow::ProcessInput[ HookType_Before ][ i ]( this, a_pEvent ) )
		{
			return TTRUE;
		}
	}

	// Make all slides skippable:
	m_eFlags |= AGUISlideshow::Flags_Skippable;

	if ( !ProcessInput( a_pEvent ) )
	{
		for ( TINT i = 0; i < AHooks::GUISlideshow::ProcessInput[ HookType_After ].Size(); i++ )
		{
			if ( AHooks::GUISlideshow::ProcessInput[ HookType_After ][ i ]( this, a_pEvent ) )
			{
				return TTRUE;
			}
		}

		return TFALSE;
	}

	return TTRUE;
}

HOOK( 0x0042ab30, FUN_0042ab30, void )
{
	for ( TINT i = 0; i < AHooks::Uncategorized::NewGameStarted[ HookType_Before ].Size(); i++ )
	{
		AHooks::Uncategorized::NewGameStarted[ HookType_Before ][ i ]();
	}

	CallOriginal();

	for ( TINT i = 0; i < AHooks::Uncategorized::NewGameStarted[ HookType_After ].Size(); i++ )
	{
		AHooks::Uncategorized::NewGameStarted[ HookType_After ][ i ]();
	}
}

HOOK( 0x00635410, AGUI2_MainPostRenderCallback, void )
{
	for ( TINT i = 0; i < AHooks::GUI2::MainPostRenderCallback[ HookType_Before ].Size(); i++ )
	{
		AHooks::GUI2::MainPostRenderCallback[ HookType_Before ][ i ]();
	}

	CallOriginal();

	for ( TINT i = 0; i < AHooks::GUI2::MainPostRenderCallback[ HookType_After ].Size(); i++ )
	{
		AHooks::GUI2::MainPostRenderCallback[ HookType_After ][ i ]();
	}
}

MEMBER_HOOK( 0x00635440, AGUI2, AGUI2_Constructor, AGUI2* )
{
	CallOriginal();
	return this;
}

MEMBER_HOOK( 0x006355a0, AGUI2, AGUI2_OnCreate, TBOOL )
{
	CallOriginal();

	if ( g_uiWindowWidth >= 1280 && g_uiWindowWidth >= 768 )
	{
		AGUI2::GetContext()->GetRootElement()->SetDimensions( 936, 702 );
	}

	return TTRUE;
}

MEMBER_HOOK( 0x006357d0, AGUI2, AGUI2_OnUpdate, TBOOL, TFLOAT a_fDeltaTime )
{
	if ( m_bShowMemStatsInfo ) m_oMemStats.Show();
	else m_oMemStats.Hide();

	if ( m_bShowFPSInfo ) m_oFPS.Show();
	else m_oFPS.Hide();

	if ( m_bShowPlayerInfo ) m_oPlayerInfo.Show();
	else m_oPlayerInfo.Hide();

	if ( m_bShowTexturesInfo ) m_oTexturesInfo.Show();
	else m_oTexturesInfo.Hide();

	return CallOriginal( a_fDeltaTime );
}

MEMBER_HOOK( 0x004293d0, AGameStateController, AGameStateController_ProcessInput, TBOOL, TInputInterface::InputEvent* a_pInputEvent )
{
	if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN &&
	     a_pInputEvent->GetDoodad() == TInputDeviceKeyboard::KEY_GRAVE )
	{
		Toshi::TInputDeviceKeyboard* pKeyboard = TSTATICCAST( Toshi::TInputDeviceKeyboard, a_pInputEvent->GetSource() );

		if ( pKeyboard->IsAltDown() )
			AImGUI::GetSingleton()->Toggle();
	}
	else if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN && a_pInputEvent->GetDoodad() == TInputDeviceKeyboard::KEY_F5 )
	{
		*(TBOOL*)( ( *(TUINT*)( 0x0079b2ec ) + 0x88 ) ) = !( *(TBOOL*)( ( *(TUINT*)( 0x0079b2ec ) + 0x88 ) ) );
	}

	return CallOriginal( a_pInputEvent );
}

MEMBER_HOOK( 0x005ea8b0, ATerrainInterface, ATerrain_Render, void )
{
	for ( TINT i = 0; i < AHooks::Terrain::Render[ HookType_Before ].Size(); i++ )
	{
		AHooks::Terrain::Render[ HookType_Before ][ i ]( this );
	}

	CallOriginal();

	for ( TINT i = 0; i < AHooks::Terrain::Render[ HookType_After ].Size(); i++ )
	{
		AHooks::Terrain::Render[ HookType_After ][ i ]( this );
	}
}

HOOK( 0x006114d0, AModelLoader_AModelLoaderLoadTRBCallback, TBOOL, TModel* a_pModel )
{
	TBOOL bRes;

	for ( TINT i = 0; i < AHooks::ModelLoader::LoadTRBCallback[ HookType_Before ].Size(); i++ )
	{
		bRes |= AHooks::ModelLoader::LoadTRBCallback[ HookType_Before ][ i ]( a_pModel );
	}

	bRes |= CallOriginal( a_pModel );

	for ( TINT i = 0; i < AHooks::ModelLoader::LoadTRBCallback[ HookType_After ].Size(); i++ )
	{
		bRes |= AHooks::ModelLoader::LoadTRBCallback[ HookType_After ][ i ]( a_pModel );
	}

	return bRes;
}

TBOOL  g_bNoCullingInRadiusOfObject = TTRUE;
TFLOAT g_fNoCullingAdditionalRadius = 50.0f;

HOOK( 0x006cead0, TRenderContext_CullSphereToFrustumSimple, TBOOL, const TSphere& a_rSphere, const TPlane* a_pPlanes, int a_iNumPlane )
{
	if ( g_bNoCullingInRadiusOfObject && ACameraManager::IsSingletonCreated() )
	{
		auto  pCamera            = ACameraManager::GetSingleton()->GetCurrentCamera();
		auto& vCameraTranslation = pCamera->m_Matrix.GetTranslation();

		auto fDistance = TVector3::DistanceSq( a_rSphere.GetOrigin(), vCameraTranslation.AsVector3() );

		if ( fDistance <= a_rSphere.GetRadius() + g_fNoCullingAdditionalRadius )
		{
			return TTRUE;
		}
	}

	for ( TSIZE i = 0; i < 6; i++ )
	{
		TFLOAT fDist = TVector4::DotProduct3( a_rSphere.AsVector4(), a_pPlanes[ i ].AsVector4() );

		if ( a_rSphere.GetRadius() < fDist - a_pPlanes[ i ].GetD() )
			return TFALSE;
	}

	return TTRUE;
}

HOOK( 0x006cea40, TRenderContext_CullSphereToFrustum, TINT, const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask )
{
	if ( g_bNoCullingInRadiusOfObject && ACameraManager::IsSingletonCreated() )
	{
		auto  pCamera            = ACameraManager::GetSingleton()->GetCurrentCamera();
		auto& vCameraTranslation = pCamera->m_Matrix.GetTranslation();

		auto fDistance = TVector3::DistanceSq( a_rSphere.GetOrigin(), vCameraTranslation.AsVector3() );

		if ( fDistance <= a_rSphere.GetRadius() + g_fNoCullingAdditionalRadius )
		{
			return a_iClipFlags;
		}
	}

	TINT iLeftPlanes = a_iClipFlags & a_iClipFlagsMask;
	TINT iPlaneFlag  = 1;

	do {
		if ( iLeftPlanes == 0 )
		{
			return a_iClipFlags;
		}

		if ( iLeftPlanes & iPlaneFlag )
		{
			TFLOAT fDist = TVector4::DotProduct3( a_rSphere.AsVector4(), a_pPlanes->AsVector4() ) - a_pPlanes->GetD();

			if ( a_rSphere.GetRadius() < fDist )
			{
				return -1;
			}

			if ( fDist < -a_rSphere.GetRadius() )
			{
				a_iClipFlags &= ~iPlaneFlag;
			}

			iLeftPlanes &= ~iPlaneFlag;
		}

		iPlaneFlag = iPlaneFlag << 1;
		a_pPlanes++;

	} while ( TTRUE );
}

MEMBER_HOOK( 0x006bbb00, TSystemManager, TSystemManager_Update, void )
{
	if ( g_oSettings.bLimitFPS )
	{
		static THPTimer s_oFPSTimer;

		s_oFPSTimer.Update();
		CallOriginal();
		s_oFPSTimer.Update();

		if ( g_oSettings.iMaxFPS < 5 )
			g_oSettings.iMaxFPS = 5;

		const TFLOAT fTargetDeltaTime = 1.0f / g_oSettings.iMaxFPS;
		const TFLOAT fDelta           = s_oFPSTimer.GetDelta();

		TFLOAT flSleepTime = fTargetDeltaTime - fDelta;

		while ( flSleepTime > 0.0f )
		{
			s_oFPSTimer.Update();
			flSleepTime -= s_oFPSTimer.GetDelta();
		}
	}
	else CallOriginal();
}

MEMBER_HOOK( 0x006d5970, TOrderTable, TOrderTable_Flush, void )
{

	if ( m_pLastRegMat )
	{
		m_pShader->StartFlush();

		for ( auto it = m_pLastRegMat; it != TNULL; it = it->GetNextRegMat() )
		{
			it->Render();
		}

		m_pShader->EndFlush();
	}

	*(TUINT*)( 0x007d3124 ) = 0;
	m_pLastRegMat           = TNULL;
}

void AHooks::Initialise()
{
	// Apply other hooks
	InstallHook<TMemory_UnkMethod>();
	InstallHook<TMemory_Initialise>();
	InstallHook<TMemory_DestroyMemBlock>();
	InstallHook<TMemory_CreateMemBlock>();
	InstallHook<TMemory_SetGlobalBlock>();
	InstallHook<TMemory_Free>();
	InstallHook<TMemory_Alloc>();
	InstallHook<TMemory_GetMemInfo>();
	InstallHook<TMSWindow_SetPosition>();
	InstallHook<AGUISlideshow_ProcessInput>();
	InstallHook<FUN_0042ab30>();
	InstallHook<AGUI2_MainPostRenderCallback>();
	InstallHook<AGUI2_Constructor>();
	InstallHook<AGUI2_OnCreate>();
	InstallHook<AGUI2_OnUpdate>();
	InstallHook<AGameStateController_ProcessInput>();
	InstallHook<ATerrain_Render>();
	InstallHook<AModelLoader_AModelLoaderLoadTRBCallback>();

	InstallHook<ARenderer_CreateTRender>();
	InstallHook<ARenderer_OnCreate>();
	InstallHook<AFrontEndMiniGameState2_CTOR>();
	InstallHook<TRenderD3DInterface_OnD3DDeviceLost>();
	InstallHook<TRenderD3DInterface_OnD3DDeviceFound>();
	InstallHook<TModelRegistry_CreateModel>();
	InstallHook<TRenderInterface_SetLightColourMatrix>();
	InstallHook<AOptions_IsResolutionCompatible>();
	InstallHook<ADisplayModes_Win_DoesModeExist>();
	//InstallHook<TCameraObject_SetFOV>();
	InstallHook<TRenderD3DInterface_UpdateColourSettings>();
	//InstallHook<TRenderContext_CullSphereToFrustumSimple>();
	//InstallHook<TRenderContext_CullSphereToFrustum>();
	InstallHook<TTRB_Load>();

	// Fixing crashes and memory stumps of the original game
	InstallHook<CollObjectModel_DCTOR>();
	InstallHook<TModel_LoadTRB>();
	InstallHook<TModel_UnloadTRB>();

	// This might be unstable until all the memory stomps are fixed :(
	//InstallHook<AMaterialLibrary_LoadTTLData>();
	
	InstallHook<TSystemManager_Update>();
	InstallHook<TOrderTable_Flush>();
}

TBOOL AHooks::AddHook( Hook a_eHook, HookType a_eHookType, void* a_pCallback )
{
	switch ( a_eHook )
	{
		case Hook_AGUI2_MainPostRenderCallback:
			AHooks::GUI2::MainPostRenderCallback[ a_eHookType ].PushBack( TREINTERPRETCAST( AHooks::GUI2::t_MainPostRenderCallback, a_pCallback ) );
			return TTRUE;
		case Hook_AGUISlideshow_ProcessInput:
			AHooks::GUISlideshow::ProcessInput[ a_eHookType ].PushBack( TREINTERPRETCAST( AHooks::GUISlideshow::t_ProcessInput, a_pCallback ) );
			return TTRUE;
		case Hook_NewGameStarted:
			AHooks::Uncategorized::NewGameStarted[ a_eHookType ].PushBack( TREINTERPRETCAST( AHooks::Uncategorized::t_NewGameStarted, a_pCallback ) );
			return TTRUE;
		case Hook_ATerrain_Render:
			AHooks::Terrain::Render[ a_eHookType ].PushBack( TREINTERPRETCAST( AHooks::Terrain::t_Render, a_pCallback ) );
			return TTRUE;
		case Hook_AModelLoader_LoadTRBCallback:
			AHooks::ModelLoader::LoadTRBCallback[ a_eHookType ].PushBack( TREINTERPRETCAST( AHooks::ModelLoader::t_LoadTRBCallback, a_pCallback ) );
			return TTRUE;
		case Hook_MaterialLibrary_LoadTTLData:
			AHooks::MaterialLibrary::LoadTTLData[ a_eHookType ].PushBack( TREINTERPRETCAST( AHooks::MaterialLibrary::t_LoadTTLData, a_pCallback ) );
			return TTRUE;
		case Hook_TRenderInterface_SetLightColourMatrix:
			AHooks::RenderInterface::SetLightColourMatrix[ a_eHookType ].PushBack( TREINTERPRETCAST( AHooks::RenderInterface::t_SetLightColourMatrix, a_pCallback ) );
			return TTRUE;
	}

	return TFALSE;
}
