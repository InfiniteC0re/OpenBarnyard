#include "pch.h"
#include "AHooks.h"
#include "AImGUI.h"
#include "HookHelpers.h"
#include "AModLoaderTask.h"

#include <BYardSDK/AGameStateController.h>
#include <BYardSDK/THookedRenderD3DInterface.h>
#include <BYardSDK/AAssetLoader.h>
#include <BYardSDK/AGUI2.h>
#include <BYardSDK/ARenderer.h>

#include <Toshi/Input/TInputDeviceKeyboard.h>
#include <Toshi/Render/TCameraObject.h>
#include <Platform/Windows/TMSWindow.h>
#include <Platform/DX8/TRenderInterface_DX8.h>

TOSHI_NAMESPACE_USING

TUINT g_uiWindowWidth = 0;
TUINT g_uiWindowHeight = 0;
TBOOL g_bBikeFOVPatch = TFALSE;
TFLOAT g_fOriginalFOV = 0.0f;

MEMBER_HOOK(0x006b4a20, TMemory, TMemory_Free, TBOOL, void* a_pMem)
{
	return this->Free(a_pMem);
}

MEMBER_HOOK(0x006b5230, TMemory, TMemory_Alloc, void*, TUINT a_uiSize, TINT a_uiAlignment, TMemory::MemBlock* a_pMemBlock, const char* a_szUnused1, TINT a_iUnused2)
{
	return this->Alloc(a_uiSize, a_uiAlignment, a_pMemBlock, a_szUnused1, a_iUnused2);
}

HOOK(0x006b4ba0, TMemory_GetMemInfo, void, TMemory::MemInfo& a_rMemInfo, TMemory::MemBlock* a_pBlock)
{
	auto pBlock = (*(TMemory**)0x007ce1d4)->GetGlobalBlock();
	TMemory::GetMemInfo(a_rMemInfo, pBlock);
}

class AOptions { };

MEMBER_HOOK(0x00662d90, AOptions, AOptions_IsResolutionCompatible, TBOOL, TUINT a_uiWidth, TUINT a_uiHeight)
{
	TBOOL* pIsWindowed = (TBOOL*)((TUINT(this) + 0x20));
	TINT* pWidth = (TINT*)((TUINT(this) + 0x24));
	TINT* pHeight = (TINT*)((TUINT(this) + 0x28));

	if (*pIsWindowed == TFALSE)
	{
		RECT windowRect;
		GetWindowRect(GetDesktopWindow(), &windowRect);

		if (windowRect.right > a_uiWidth || windowRect.bottom > a_uiHeight)
		{
			const int msgBoxResult = MessageBoxW(NULL, L"It seems that you aren't using widescreen patch.\nDo you want the game to launch in your native resolution?", L"Barnyard", MB_YESNO);

			if (msgBoxResult == IDYES)
			{
				a_uiWidth = windowRect.right;
				a_uiHeight = windowRect.bottom;

				HKEY hkey;
				LSTATUS status = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\THQ\\Barnyard", NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL);

				if (status == 0)
				{
					RegSetValueExA(hkey, "Width", NULL, REG_DWORD_LITTLE_ENDIAN, (BYTE*)&a_uiWidth, sizeof(a_uiWidth));
					RegSetValueExA(hkey, "Height", NULL, REG_DWORD_LITTLE_ENDIAN, (BYTE*)&a_uiHeight, sizeof(a_uiHeight));
					RegCloseKey(hkey);

					TOSHI_INFO("Applied widescreen patch.");
				}
				else
				{
					TOSHI_ERROR("Unable to open Software\\THQ\\Barnyard for writing (Status: {0})", status);
				}
			}
		}
	}

	constexpr TFLOAT ASPECT_RATIO_5_4 = 5.0f / 4.0f;
	constexpr TFLOAT ASPECT_RATIO_25_16 = 25.0f / 16.0f;
	constexpr TFLOAT ASPECT_RATIO_16_10 = 16.0f / 10.0f;
	constexpr TFLOAT ASPECT_RATIO_15_9 = 15.0f / 9.0f;
	constexpr TFLOAT ASPECT_RATIO_16_9 = 16.0f / 9.0f;

	TFLOAT fCurrentAspectRatio = TFLOAT(a_uiWidth) / TFLOAT(a_uiHeight);
	TFLOAT* pFOV = (TFLOAT*)0x007822ac;

	if (TMath::Abs(fCurrentAspectRatio - ASPECT_RATIO_5_4) <= 0.1f)
	{
		*pFOV = 0.994199f;
	}
	else if (TMath::Abs(fCurrentAspectRatio - ASPECT_RATIO_25_16) <= 0.1f)
	{
		*pFOV = 1.18425f;
	}
	else if (TMath::Abs(fCurrentAspectRatio - ASPECT_RATIO_16_10) <= 0.1f)
	{
		*pFOV = 1.2244f;
		g_bBikeFOVPatch = TTRUE;
	}
	else if (TMath::Abs(fCurrentAspectRatio - ASPECT_RATIO_15_9) <= 0.1f)
	{
		*pFOV = 1.24655f;
		g_bBikeFOVPatch = TTRUE;
	}
	else if (TMath::Abs(fCurrentAspectRatio - ASPECT_RATIO_16_9) <= 0.1f)
	{
		*pFOV = 1.313f;
		g_bBikeFOVPatch = TTRUE;
	}

	*pWidth = a_uiWidth;
	*pHeight = a_uiHeight;

	g_uiWindowWidth = a_uiWidth;
	g_uiWindowHeight = a_uiHeight;

	g_fOriginalFOV = *pFOV;

	return TFALSE;
}

class ADisplayModes_Win { };

MEMBER_HOOK(0x00614d70, ADisplayModes_Win, ADisplayModes_Win_DoesModeExist, TBOOL, TINT& a_rWidth, TINT& a_rHeight, TUINT& a_rColourDepth)
{
	return TTRUE;
}

MEMBER_HOOK(0x006c66b0, TRenderD3DInterface, TRenderD3DInterface_UpdateColourSettings, void)
{
	return;
}

MEMBER_HOOK(0x006bb000, TTRB, TTRB_Load, TINT, const char* a_szFileName, TUINT a_uiUnk)
{
	TString8 filepath = a_szFileName;

	for (TUINT i = 0; i < filepath.Length(); i++)
		if (filepath[i] == '/') filepath[i] = '\\';

	filepath.MakeLower();

	auto pModLoaderTask = AGlobalModLoaderTask::Get();
	auto pMods = &pModLoaderTask->GetMods();
	TBOOL bFound = TFALSE;

	for (auto it = pMods->Begin(); it != pMods->End(); it++)
	{
		auto pFileOverrides = it->GetFileOverrides();

		if (pFileOverrides != TNULL)
		{
			auto pOrigFileName = pFileOverrides->GetOptionalProperty(filepath);
			
			if (pOrigFileName)
			{
				filepath = GetModsDirectory();
				filepath += pOrigFileName->GetString();
				bFound = TTRUE;
				break;
			}
		}

		if (bFound) break;
	}

	return CallOriginal(filepath, a_uiUnk);
}

MEMBER_HOOK(0x006cd220, Toshi::TCameraObject, TCameraObject_SetFOV, TFLOAT, TFLOAT a_fFOV)
{
	if (g_bBikeFOVPatch)
	{
		if (a_fFOV != g_fOriginalFOV)
		{
			//a_fFOV *= 1.04f;
		}

		//a_fFOV = g_fOriginalFOV;
	}

	return CallOriginal(a_fFOV);
}

MEMBER_HOOK(0x006c1d40, TModelRegistry, TModelRegistry_CreateModel, TModelRegistryEntry*, const char* a_szFileName, TModelPtr& a_rModelRef, TTRB* a_pAssetTRB)
{
	TString8 filepath = a_szFileName;

	for (TUINT i = 0; i < filepath.Length(); i++)
		if (filepath[i] == '/') filepath[i] = '\\';

	TString8 inputFileName = filepath.GetString(filepath.FindReverse('\\', -1) + 1);
	inputFileName.Truncate(inputFileName.FindReverse('.', -1));

	auto pModLoaderTask = AGlobalModLoaderTask::Get();
	auto pMods = &pModLoaderTask->GetMods();
	TTRB* pFoundAsset = TNULL;

	for (auto it = pMods->Begin(); it != pMods->End(); it++)
	{
		auto pAsset = it->GetAssetFile();

		if (pAsset != TNULL)
		{
			auto iNumSymbols = pAsset->GetNumSymbols();

			for (TINT i = 0; i < iNumSymbols; i++)
			{
				auto szSymbolName = pAsset->GetSymbolName(i);

				if (TStringManager::String8FindString(szSymbolName, inputFileName) == szSymbolName)
				{
					pFoundAsset = pAsset;
					break;
				}
			}
		}

		if (pFoundAsset) break;
	}

	if (TStringManager::String8FindString(a_szFileName, "busha.trb"))
	{
		TOSHI_INFO("Warning!");
	}

	return CallOriginal(a_szFileName, a_rModelRef, pFoundAsset ? pFoundAsset : a_pAssetTRB);
}

MEMBER_HOOK(0x006bf6b0, TRenderInterface, TRenderInterface_SetLightColourMatrix, void, TMatrix44* a_pLightColour)
{
	for (TUINT i = 0; i < AHooks::RenderInterface::SetLightColourMatrix[HookType_Before].Size(); i++)
	{
		AHooks::RenderInterface::SetLightColourMatrix[HookType_Before][i](this, a_pLightColour);
	}

	CallOriginal(a_pLightColour);

	for (TUINT i = 0; i < AHooks::RenderInterface::SetLightColourMatrix[HookType_After].Size(); i++)
	{
		AHooks::RenderInterface::SetLightColourMatrix[HookType_After][i](this, a_pLightColour);
	}
}

class AFrontEndMiniGameState2 {};

MEMBER_HOOK(0x00409ce0, AFrontEndMiniGameState2, AFrontEndMiniGameState2_CTOR, void, TBOOL a_bHideVariantSelector)
{
	CallOriginal(TFALSE);
}

MEMBER_HOOK(0x006c6de0, TRenderD3DInterface, TRenderD3DInterface_OnD3DDeviceLost, void)
{
	CallOriginal();
	AImGUI::GetSingleton()->OnD3DDeviceLost();
}

MEMBER_HOOK(0x006c6e80, TRenderD3DInterface, TRenderD3DInterface_OnD3DDeviceFound, void)
{
	CallOriginal();
	AImGUI::GetSingleton()->OnD3DDeviceFound();
}

MEMBER_HOOK(0x006154c0, ARenderer, ARenderer_CreateTRender, TBOOL)
{
	TBOOL bResult = CallOriginal();

	TRenderInterface::SetSingletonExplicit(
		THookedRenderD3DInterface::GetSingleton()
	);

	AGlobalModLoaderTask::Get()->OnRenderInterfaceReady();

	return bResult;
}

MEMBER_HOOK(0x0060c7c0, ARenderer, ARenderer_OnCreate, TBOOL)
{
	TBOOL bResult = CallOriginal();

	AGlobalModLoaderTask::Get()->OnAppRendererReady();

	return bResult;
}

MEMBER_HOOK(0x00615d20, AMaterialLibrary, AMaterialLibrary_LoadTTLData, TBOOL, AMaterialLibrary::TTL* a_pTTL)
{
	for (TUINT i = 0; i < AHooks::MaterialLibrary::LoadTTLData[HookType_Before].Size(); i++)
	{
		if (AHooks::MaterialLibrary::LoadTTLData[HookType_Before][i](this, a_pTTL))
		{
			return TTRUE;
		}
	}

	if (!CallOriginal(a_pTTL))
	{
		for (TUINT i = 0; i < AHooks::MaterialLibrary::LoadTTLData[HookType_After].Size(); i++)
		{
			if (AHooks::MaterialLibrary::LoadTTLData[HookType_After][i](this, a_pTTL))
			{
				return TTRUE;
			}
		}

		return TFALSE;
	}

	return TTRUE;
}

MEMBER_HOOK(0x006da4d0, TMSWindow, TMSWindow_SetPosition, void, TUINT x, TUINT y, TUINT width, TUINT height)
{
	if (IsWindowed())
	{
		// Fix window size when in windowed mode
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = width;
		rect.bottom = height;

		if (TRUE == AdjustWindowRect(
			&rect,
			GetWindowLongA(GetHWND(), GWL_STYLE),
			FALSE
		))
		{
			x = rect.left;
			y = rect.top;
			width = rect.right;
			height = rect.bottom;
		}
	}
	
	CallOriginal(x, y, width, height);
}

MEMBER_HOOK(0x0059dac0, AGUISlideshow, AGUISlideshow_ProcessInput, TBOOL, TInputInterface::InputEvent* a_pEvent)
{
	for (TUINT i = 0; i < AHooks::GUISlideshow::ProcessInput[HookType_Before].Size(); i++)
	{
		if (AHooks::GUISlideshow::ProcessInput[HookType_Before][i](this, a_pEvent))
		{
			return TTRUE;
		}
	}

	// Make all slides skippable:
	m_eFlags |= AGUISlideshow::Flags_Skippable;

	if (!ProcessInput(a_pEvent))
	{
		for (TUINT i = 0; i < AHooks::GUISlideshow::ProcessInput[HookType_After].Size(); i++)
		{
			if (AHooks::GUISlideshow::ProcessInput[HookType_After][i](this, a_pEvent))
			{
				return TTRUE;
			}
		}

		return TFALSE;
	}

	return TTRUE;
}

HOOK(0x0042ab30, FUN_0042ab30, void)
{
	for (TUINT i = 0; i < AHooks::Uncategorized::NewGameStarted[HookType_Before].Size(); i++)
	{
		AHooks::Uncategorized::NewGameStarted[HookType_Before][i]();
	}

	CallOriginal();

	for (TUINT i = 0; i < AHooks::Uncategorized::NewGameStarted[HookType_After].Size(); i++)
	{
		AHooks::Uncategorized::NewGameStarted[HookType_After][i]();
	}
}

HOOK(0x00635410, AGUI2_MainPostRenderCallback, void)
{
	for (TUINT i = 0; i < AHooks::GUI2::MainPostRenderCallback[HookType_Before].Size(); i++)
	{
		AHooks::GUI2::MainPostRenderCallback[HookType_Before][i]();
	}

	CallOriginal();

	for (TUINT i = 0; i < AHooks::GUI2::MainPostRenderCallback[HookType_After].Size(); i++)
	{
		AHooks::GUI2::MainPostRenderCallback[HookType_After][i]();
	}
}

MEMBER_HOOK(0x00635440, AGUI2, AGUI2_Constructor, AGUI2*)
{
	CallOriginal();
	return this;
}

MEMBER_HOOK(0x006355a0, AGUI2, AGUI2_OnCreate, TBOOL)
{
	CallOriginal();

	if (g_uiWindowWidth >= 1280 && g_uiWindowWidth >= 768)
	{
		AGUI2::GetContext()->GetRootElement()->SetDimensions(936, 702);
	}

	return TTRUE;
}

MEMBER_HOOK(0x006357d0, AGUI2, AGUI2_OnUpdate, TBOOL, TFLOAT a_fDeltaTime)
{
	if (m_bShowMemStatsInfo) m_oMemStats.Show();
	else m_oMemStats.Hide();

	if (m_bShowFPSInfo) m_oFPS.Show();
	else m_oFPS.Hide();

	if (m_bShowPlayerInfo) m_oPlayerInfo.Show();
	else m_oPlayerInfo.Hide();

	if (m_bShowTexturesInfo) m_oTexturesInfo.Show();
	else m_oTexturesInfo.Hide();

	return CallOriginal(a_fDeltaTime);
}

MEMBER_HOOK(0x004293d0, AGameStateController, AGameStateController_ProcessInput, TBOOL, TInputInterface::InputEvent* a_pInputEvent)
{
	if (a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN &&
		a_pInputEvent->GetDoodad() == TInputDeviceKeyboard::KEY_GRAVE)
	{
		AImGUI::GetSingleton()->Toggle();
	}
	else if (a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN &&
		a_pInputEvent->GetDoodad() == TInputDeviceKeyboard::KEY_F5)
	{
		*(TBOOL*)((*(TUINT*)(0x0079b2ec) + 0x88)) = !(*(TBOOL*)((*(TUINT*)(0x0079b2ec) + 0x88)));
	}

	return CallOriginal(a_pInputEvent);
}

MEMBER_HOOK(0x005ea8b0, ATerrain, ATerrain_Render, void)
{
	for (TUINT i = 0; i < AHooks::Terrain::Render[HookType_Before].Size(); i++)
	{
		AHooks::Terrain::Render[HookType_Before][i](this);
	}
	
	CallOriginal();

	for (TUINT i = 0; i < AHooks::Terrain::Render[HookType_After].Size(); i++)
	{
		AHooks::Terrain::Render[HookType_After][i](this);
	}
}

HOOK(0x006114d0, AModelLoader_AModelLoaderLoadTRBCallback, TBOOL, TModel* a_pModel)
{
	TBOOL bRes;

	for (TUINT i = 0; i < AHooks::ModelLoader::LoadTRBCallback[HookType_Before].Size(); i++)
	{
		bRes |= AHooks::ModelLoader::LoadTRBCallback[HookType_Before][i](a_pModel);
	}

	bRes |= CallOriginal(a_pModel);

	for (TUINT i = 0; i < AHooks::ModelLoader::LoadTRBCallback[HookType_After].Size(); i++)
	{
		bRes |= AHooks::ModelLoader::LoadTRBCallback[HookType_After][i](a_pModel);
	}

	return bRes;
}

void AHooks::Initialise()
{
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
	InstallHook<AMaterialLibrary_LoadTTLData>();
	InstallHook<ARenderer_CreateTRender>();
	InstallHook<ARenderer_OnCreate>();
	InstallHook<AFrontEndMiniGameState2_CTOR>();
	InstallHook<TRenderD3DInterface_OnD3DDeviceLost>();
	InstallHook<TRenderD3DInterface_OnD3DDeviceFound>();
	InstallHook<TModelRegistry_CreateModel>();
	InstallHook<TRenderInterface_SetLightColourMatrix>();
	InstallHook<AOptions_IsResolutionCompatible>();
	InstallHook<ADisplayModes_Win_DoesModeExist>();
	InstallHook<TCameraObject_SetFOV>();
	InstallHook<TRenderD3DInterface_UpdateColourSettings>();
	InstallHook<TTRB_Load>();
}

TBOOL AHooks::AddHook(Hook a_eHook, HookType a_eHookType, void* a_pCallback)
{
	switch (a_eHook)
	{
	case Hook_AGUI2_MainPostRenderCallback:
		AHooks::GUI2::MainPostRenderCallback[a_eHookType].PushBack(TSTATICCAST(AHooks::GUI2::t_MainPostRenderCallback, a_pCallback));
		return TTRUE;
	case Hook_AGUISlideshow_ProcessInput:
		AHooks::GUISlideshow::ProcessInput[a_eHookType].PushBack(TSTATICCAST(AHooks::GUISlideshow::t_ProcessInput, a_pCallback));
		return TTRUE;
	case Hook_NewGameStarted:
		AHooks::Uncategorized::NewGameStarted[a_eHookType].PushBack(TSTATICCAST(AHooks::Uncategorized::t_NewGameStarted, a_pCallback));
		return TTRUE;
	case Hook_ATerrain_Render:
		AHooks::Terrain::Render[a_eHookType].PushBack(TSTATICCAST(AHooks::Terrain::t_Render, a_pCallback));
		return TTRUE;
	case Hook_AModelLoader_LoadTRBCallback:
		AHooks::ModelLoader::LoadTRBCallback[a_eHookType].PushBack(TSTATICCAST(AHooks::ModelLoader::t_LoadTRBCallback, a_pCallback));
		return TTRUE;
	case Hook_MaterialLibrary_LoadTTLData:
		AHooks::MaterialLibrary::LoadTTLData[a_eHookType].PushBack(TSTATICCAST(AHooks::MaterialLibrary::t_LoadTTLData, a_pCallback));
		return TTRUE;
	case Hook_TRenderInterface_SetLightColourMatrix:
		AHooks::RenderInterface::SetLightColourMatrix[a_eHookType].PushBack(TSTATICCAST(AHooks::RenderInterface::t_SetLightColourMatrix, a_pCallback));
		return TTRUE;
	}

	return TFALSE;
}
