#pragma once
#include <Toshi/Render/TModel.h>

TOBJECT(AModelLoader, Toshi::TObject, TTRUE),
	public Toshi::TSingleton<AModelLoader>
{
public:
	AModelLoader();

private:
	static TBOOL AModelLoaderLoadTMDCallback(Toshi::TModel * a_pModel);
	static TBOOL AModelLoaderLoadTRBCallback(Toshi::TModel * a_pModel);
};
