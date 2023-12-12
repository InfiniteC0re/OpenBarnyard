#include "pch.h"
#include "AAssetLoader.h"
#include "AAssetStreaming.h"
#include "ATRBLoaderJob.h"

TOSHI_NAMESPACE_USING

TBOOL AAssetLoader::Load(const char* a_szFileName, AAssetType a_eAssetType, TBOOL a_bUseStreaming)
{
	return TTRUE;
}

void AAssetLoader::Close(AAssetType a_eAssetType)
{
	
}

void* AAssetLoader::GetSymbolAddress(const char* a_szFileName, const char* a_szSymbolName, AAssetType a_eAssetType)
{
	return TNULL;
}
