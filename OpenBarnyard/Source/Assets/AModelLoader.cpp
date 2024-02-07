#include "pch.h"
#include "AModelLoader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS(AModelLoader);

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

