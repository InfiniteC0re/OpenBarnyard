#pragma once
#include "TVertexPoolResource_DX11.h"
#include "TVertexBlockResource_DX11.h"
#include "Render/TVertexFactoryResourceInterface.h"

TOSHI_NAMESPACE_START

class MODAPI TVertexFactoryResource : public TVertexFactoryResourceInterface
{
public:
	TDECLARE_CLASS( TVertexFactoryResource, TVertexFactoryResourceInterface );

public:
	~TVertexFactoryResource();

public:
	virtual TVertexPoolResourceInterface* CreatePoolResource( TUINT16 a_uiMaxStaticVertices, TUINT16 a_uiFlags );

	TVertexBlockResource* FindBlockResource( TVertexPoolResource* a_pResource );
	TVertexBlockResource* CreateBlockResource( TUINT16 a_uiMaxVertices, TUINT32 a_uiFlags );
};

TOSHI_NAMESPACE_END
