#pragma once
#include "TVertexBlockResource_DX8.h"

#include "Render/TVertexPoolResourceInterface.h"

TOSHI_NAMESPACE_START

class TVertexBlockResource;

class TVertexPoolResource : public TVertexPoolResourceInterface
{
public:
	TDECLARE_CLASS( TVertexPoolResource, TVertexPoolResourceInterface );

public:
	friend TVertexBlockResource;

public:
	TVertexPoolResource();

	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;
	virtual void  OnDestroy() override;
	virtual TBOOL Lock( LockBuffer* a_pLockBuffer ) override;
	virtual void  Unlock( TUINT16 a_uiNewNumVertices ) override;
	virtual TBOOL Create( TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT16 a_uiFlags ) override;

	TBYTE* GetManagedStream( TUINT a_uiStream );

	TResource*            GetParent();
	TVertexBlockResource* GetVertexBlock();

	TBOOL GetHALBuffer( TVertexBlockResource::HALBuffer* a_pHALBuffer );

#ifndef TOSHI_MODLOADER_CLIENT
private:
#endif

	TUINT16 m_uiVertexOffset;
	TBYTE*  m_apManagedStreams[ TVertexFactoryFormat::MAX_NUM_STREAMS ];
	TUINT   m_uiNumLocksAllTime;
};

TOSHI_NAMESPACE_END
