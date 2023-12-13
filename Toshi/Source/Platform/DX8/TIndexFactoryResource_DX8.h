#pragma once
#include "TIndexPoolResource_DX8.h"
#include "TIndexBlockResource_DX8.h"
#include "Toshi/Render/TIndexFactoryResourceInterface.h"

namespace Toshi {

	TOBJECT(TIndexFactoryResource, TIndexFactoryResourceInterface, TTRUE)
	{
	public:
		virtual TIndexPoolResourceInterface* CreatePoolResource(TUINT16 a_uiMaxStaticIndices, TUINT16 a_uiFlags);

		TIndexBlockResource* FindBlockResource(TIndexPoolResource * a_pResource);
		TIndexBlockResource* CreateBlockResource(TUINT16 a_uiMaxIndices, TUINT32 a_uiFlags);
	};

}
