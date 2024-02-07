#pragma once
#include <Render/TModel.h>

class AModelLoader : public Toshi::TObject,
	public Toshi::TSingleton<AModelLoader>
{
public:
	TDECLARE_CLASS(Toshi::TObject);

public:
	AModelLoader();

private:
	static TBOOL AModelLoaderLoadTMDCallback(Toshi::TModel * a_pModel);
	static TBOOL AModelLoaderLoadTRBCallback(Toshi::TModel * a_pModel);
};
