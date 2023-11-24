#include "pch.h"
#include "AModelLoader.h"

AModelLoader::AModelLoader()
{
	TIMPLEMENT();

	Toshi::TModel::SetLoaderTMDCallback(AModelLoaderLoadTMDCallback);
	Toshi::TModel::SetLoaderTRBCallback(AModelLoaderLoadTRBCallback);
}

TBOOL AModelLoader::AModelLoaderLoadTMDCallback(Toshi::TModel* a_pModel)
{
	return TFALSE;
}

TBOOL AModelLoader::AModelLoaderLoadTRBCallback(Toshi::TModel* a_pModel)
{
	TIMPLEMENT();
	return TTRUE;
}

