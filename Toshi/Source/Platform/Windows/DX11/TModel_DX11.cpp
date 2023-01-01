#pragma once
#include "ToshiPCH.h"
#include "Toshi/Render/TModel.h"

namespace Toshi
{
	bool TModel::Create(const char* name, bool a_bLoadImmediately)
	{
		TASSERT(TFALSE == IsCreated(), "");
		m_Flags |= 1;
		TASSERT(TFALSE == a_bLoadImmediately, "");
		m_modelName = name;
		return true;
	}
}