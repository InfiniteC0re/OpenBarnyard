#pragma once
#include "TIndexPoolResource_DX11.h"
#include "TIndexBlockResource_DX11.h"
#include "Render/TIndexFactoryResourceInterface.h"

TOSHI_NAMESPACE_START

class MODAPI TIndexFactoryResource : public TIndexFactoryResourceInterface
{
public:
	TDECLARE_CLASS( TIndexFactoryResource, TIndexFactoryResourceInterface );

public:
	virtual TIndexPoolResourceInterface* CreatePoolResource( TUINT16 a_uiMaxStaticIndices, TUINT16 a_uiFlags );

	TIndexBlockResource* FindBlockResource( TIndexPoolResource* a_pResource );
	TIndexBlockResource* CreateBlockResource( TUINT16 a_uiMaxIndices, TUINT32 a_uiFlags );
};

TOSHI_NAMESPACE_END
