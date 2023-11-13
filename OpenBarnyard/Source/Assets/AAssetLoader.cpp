#include "pch.h"
#include "AAssetLoader.h"
#include "AAssetStreaming.h"
#include "ATRBLoaderJob.h"
#include "ALoadScreen.h"

TOSHI_NAMESPACE_USING

void AAssetLoader::Load(const char* a_szFileName, AssetType a_eAssetType, TBOOL a_bUseStreaming)
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

		AAssetStreaming::GetSingleton()->AddMainThreadJob2(&trbLoaderJob);

		while (AAssetStreaming::GetSingleton()->HasActiveJobs())
		{
			ALoadScreen::GetGlobalInstance()->Update(1.0f, TTRUE);
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
		}
	}
}
