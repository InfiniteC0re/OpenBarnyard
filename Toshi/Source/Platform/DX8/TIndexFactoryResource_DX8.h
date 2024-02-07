#pragma once
#include "TIndexPoolResource_DX8.h"
#include "TIndexBlockResource_DX8.h"
#include "Render/TIndexFactoryResourceInterface.h"

namespace Toshi {

	class TIndexFactoryResource : public TIndexFactoryResourceInterface
	{
	public:
		TDECLARE_CLASS(TIndexFactoryResourceInterface);

	public:
		virtual TIndexPoolResourceInterface* CreatePoolResource(TUINT16 a_uiMaxStaticIndices, TUINT16 a_uiFlags);

		TIndexBlockResource* FindBlockResource(TIndexPoolResource * a_pResource);
		TIndexBlockResource* CreateBlockResource(TUINT16 a_uiMaxIndices, TUINT32 a_uiFlags);
	};

}
