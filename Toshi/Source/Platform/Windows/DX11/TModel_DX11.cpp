#pragma once
#include "ToshiPCH.h"
#include "TModel_DX11.h"

namespace Toshi
{
	/*
	bool TModel::Create(const char* name, bool a_bLoadImmediately)
	{
		TASSERT(TFALSE == IsCreated());
		m_Flags |= 1;
		TASSERT(TFALSE == a_bLoadImmediately);
		m_modelName = name;
		return true;
	}*/
	
	void TModelHAL::Create(TTMDWin::TTRBWinHeader* a_tmdHeader)
	{
		m_tmdHeader = a_tmdHeader;
		m_txsModelName = a_tmdHeader->m_txsModelName;
	}

	void TModel::CreateResource(const char* a_resourceName)
	{
		//T2Resource::CreateResource(a_resourceName, this, ResourceCallback, this);
		
	}

}