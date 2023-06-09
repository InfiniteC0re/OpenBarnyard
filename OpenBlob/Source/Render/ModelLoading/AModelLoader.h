#pragma once
#include <Toshi/File/TTRB.h>
#include <Toshi/Render/TModel.h>
#include <Toshi/Render/TTMDWin.h>

class AModelLoader :
	public Toshi::TSingleton<AModelLoader>
{
public:
	AModelLoader();

private:
	static TBOOL LoadTRBCallback2(Toshi::TModel* pModel, Toshi::TTMDWin::TTRBWinHeader* pHeader);
	
	static TBOOL AModelLoaderLoadTRBCallback(Toshi::TTRB& pTRB, Toshi::TModel* pModel);
	static TBOOL AModelLoaderLoadTRBCallback2(Toshi::TModel* pModel, Toshi::TTMDWin::TTRBWinHeader* pHeader);
};
