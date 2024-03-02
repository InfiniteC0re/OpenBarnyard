#include "pch.h"
#include "AAssetLoader.h"
#include "AAssetStreaming.h"
#include "ATRBLoaderJob.h"
#include "ALoadScreen.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL AAssetLoader::Load(const TCHAR* a_szFileName, AAssetType a_eAssetType, TBOOL a_bUseStreaming)
{
	if (!ms_pTRBFiles[a_eAssetType])
	{
		// Create TTRB if it doesn't exist
		ms_pTRBFiles[a_eAssetType] = new TTRB;
	}
	else
	{
		ms_pTRBFiles[a_eAssetType]->Close();
	}	

	if (a_bUseStreaming && AAssetStreaming::IsSingletonCreated())
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
