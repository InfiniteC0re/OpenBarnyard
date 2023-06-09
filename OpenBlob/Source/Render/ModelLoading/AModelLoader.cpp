#include "pch.h"
#include "AModelLoader.h"

using namespace Toshi;

AModelLoader::AModelLoader()
{
	TModel::SetTRBLoadCallback(AModelLoaderLoadTRBCallback);
}

TBOOL AModelLoader::AModelLoaderLoadTRBCallback(TTRB& pTRB, TModel* pModel)
{
	return TFALSE;
}

TBOOL AModelLoader::LoadTRBCallback2(Toshi::TModel* pModel, Toshi::TTMDWin::TTRBWinHeader* pHeader)
{
	pModel->m_iLODCount = pHeader->m_iLODCount;
	pModel->m_fUnknown = pHeader->m_fUnknown;

	for (size_t i = 0; i < pHeader->m_iLODCount; i++)
	{
		auto& lod = pModel->m_LODLevels[i];

		TTODO("...");
	}

	return TTRUE;
}

TBOOL AModelLoader::AModelLoaderLoadTRBCallback2(Toshi::TModel* pModel, Toshi::TTMDWin::TTRBWinHeader* pHeader)
{
	return LoadTRBCallback2(pModel, pHeader);
}