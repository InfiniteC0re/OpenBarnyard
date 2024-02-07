#pragma once
#include "TVertexPoolResource_DX8.h"
#include "TVertexBlockResource_DX8.h"
#include "Render/TVertexFactoryResourceInterface.h"

namespace Toshi {

	class TVertexFactoryResource : public TVertexFactoryResourceInterface
	{
	public:
		TDECLARE_CLASS(TVertexFactoryResourceInterface);

	public:
		virtual TVertexPoolResourceInterface* CreatePoolResource(TUINT16 a_uiMaxStaticVertices, TUINT16 a_uiFlags);
		
		TVertexBlockResource* FindBlockResource(TVertexPoolResource * a_pResource);
		TVertexBlockResource* CreateBlockResource(TUINT16 a_uiMaxVertices, TUINT32 a_uiFlags);
	};
	
}
