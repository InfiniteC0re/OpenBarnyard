#include "ToshiPCH.h"
#include "TIndexFactoryResourceInterface.h"

namespace Toshi {

	TIndexFactoryResourceInterface::TIndexFactoryResourceInterface()
	{
		m_uiNumPools = 0;
		m_uiMaxStaticIndices = 0;
		m_uiFlags = 0;
	}

	TBOOL TIndexFactoryResourceInterface::Create(TIndexFactoryFormat* a_pIndexFormat, TUINT16 a_uiMaxStaticIndices, TUINT32 a_uiFlags)
	{
		m_IndexFormat = *a_pIndexFormat;
		m_uiMaxStaticIndices = a_uiMaxStaticIndices;
		m_uiFlags = a_uiFlags;
		return TResource::Create();
	}

}

