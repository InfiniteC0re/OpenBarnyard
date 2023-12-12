#pragma once
#include "TVertexPoolResource_DX8.h"
#include "TVertexBlockResource_DX8.h"
#include "Toshi/Render/TVertexFactoryResourceInterface.h"

namespace Toshi {

	TOBJECT(TVertexFactoryResource, TVertexFactoryResourceInterface, TTRUE)
	{
	public:
		virtual TVertexPoolResourceInterface* CreatePoolResource(TUINT16 a_uiMaxStaticVertices, TUINT16 a_uiFlags);
		
		TVertexBlockResource* FindBlockResource(TVertexPoolResource * a_pResource);
		TVertexBlockResource* CreateBlockResource(TUINT16 a_uiMaxVertices, TUINT32 a_uiFlags);
	};
	
}
