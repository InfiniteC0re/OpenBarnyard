#include "pch.h"
#include "AAssetLoader.h"
#include "AAssetStreaming.h"
#include "AMaterialLibraryManager.h"
#include "AKeyFrameLibraryManager.h"
#include "ATRBLoaderJob.h"
#include "ALoadScreen.h"

#include <Toshi/TString8.h>
#include <File/TTRB.h>
#include <Plugins/PPropertyParser/PBProperties.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static void TranslateLibraryName(Toshi::TString8& a_rOutName, const TCHAR* a_szLibraryName)
{
	TVALIDPTR(a_szLibraryName);
	TTODO("Take day/night cycle and current story progress into account");

	a_rOutName.Format("Data/%s.trb", a_szLibraryName);
}

TBOOL AAssetLoader::LoadAssetPackFromLibrary(const TCHAR* a_szLibraryName, TBOOL a_bStream)
{
	TPROFILER_SCOPE();

	TString8 libFileName;
	TranslateLibraryName(libFileName, a_szLibraryName);

	TTRB libTrb;
	auto eResult = libTrb.Load(libFileName);

	if (eResult == TTRB::ERROR_OK)
	{
		auto pProperties = PBProperties::LoadFromTRB(libTrb);
		TVALIDPTR(pProperties);

		// Load asset pack
		auto pModelLib = pProperties->GetOptionalProperty("modellib");

		if (pModelLib)
		{
			TBOOL bResult = Load(pModelLib->GetString(), AAssetType_AssetPack, a_bStream);
			TASSERT(bResult);
		}

		return TTRUE;
	}
	
	return TFALSE;
}

TBOOL AAssetLoader::CreateAssetsFromLibrary(const TCHAR* a_szLibraryName)
{
	TPROFILER_SCOPE();

	TString8 libFileName;
	TranslateLibraryName(libFileName, a_szLibraryName);

	TTRB libTrb;
	auto eResult = libTrb.Load(libFileName);

	if (eResult == TTRB::ERROR_OK)
	{
		auto pProperties = PBProperties::LoadFromTRB(libTrb);
		TVALIDPTR(pProperties);

		TFIXME("Create other assets");

		// Load material libraries
		auto pMatlibs = pProperties->GetOptionalProperty("matlib");

		if (pMatlibs)
		{
			AMaterialLibraryManager::GetSingleton()->LoadLibrariesFromProperties(
				pMatlibs,
				GetAssetTRB(AAssetType_AssetPack),
				TTRUE
			);

			g_oLoadScreen.Update(1.0f, TTRUE);
		}

		// Load keyframe libraries
		auto pKeyframes = pProperties->GetOptionalProperty("keylib");

		if (pKeyframes)
		{
			AKeyFrameLibraryManager::GetSingleton()->LoadLibrariesFromProperties(
				pKeyframes,
				GetAssetTRB(AAssetType_AssetPack)
			);

			g_oLoadScreen.Update(1.0f, TTRUE);
		}

		return TTRUE;
	}

	return TFALSE;
}

TBOOL AAssetLoader::Load(const TCHAR* a_szFileName, AAssetType a_eAssetType, TBOOL a_bStream)
{
	TPROFILER_SCOPE();

	if (!ms_pTRBFiles[a_eAssetType])
	{
		// Create TTRB if it doesn't exist
		ms_pTRBFiles[a_eAssetType] = new TTRB;
	}
	else
	{
		ms_pTRBFiles[a_eAssetType]->Close();
	}	

	if (a_bStream && AAssetStreaming::IsSingletonCreated())
	{
		ATRBLoaderJob trbLoaderJob;
		trbLoaderJob.InitJob(ms_pTRBFiles[a_eAssetType], a_szFileName);

		AAssetStreaming::GetSingleton()->AddMainThreadJob(&trbLoaderJob);

		while (AAssetStreaming::GetSingleton()->HasActiveJobs())
		{
			g_oLoadScreen.Update(1.0f, TTRUE);
			Sleep(5);
			AAssetStreaming::GetSingleton()->Update();
		}
	}
	else
	{
		auto eError = ms_pTRBFiles[a_eAssetType]->Load(a_szFileName);

		if (eError != TTRB::ERROR_OK)
		{
			delete ms_pTRBFiles[a_eAssetType];
			ms_pTRBFiles[a_eAssetType] = TNULL;

			return TFALSE;
		}
	}

	return TTRUE;
}

void AAssetLoader::Close(AAssetType a_eAssetType)
{
	if (ms_pTRBFiles[a_eAssetType])
	{
		ms_pTRBFiles[a_eAssetType]->Close();
		delete ms_pTRBFiles[a_eAssetType];
		ms_pTRBFiles[a_eAssetType] = TNULL;
	}
}

void* AAssetLoader::GetSymbolAddress(const TCHAR* a_szFileName, const TCHAR* a_szSymbolName, AAssetType a_eAssetType)
{
	if (ms_pTRBFiles[a_eAssetType])
	{
		TCHAR szName[256];
		Toshi::TStringManager::String8Format(szName, sizeof(szName), "%s_%s", a_szFileName, a_szSymbolName);
		return ms_pTRBFiles[a_eAssetType]->GetSymbolAddress(szName);
	}

	return TNULL;
}

