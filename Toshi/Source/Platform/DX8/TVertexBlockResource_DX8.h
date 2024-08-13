#pragma once
#include "Toshi/TFreeList.h"
#include "Render/TResource.h"
#include "Render/TVertexPoolResourceInterface.h"
#include "Render/TVertexFactoryResourceInterface.h"

#include <d3d8.h>
#include "Core/TMemoryDebugOff.h"

TOSHI_NAMESPACE_START

class TVertexPoolResource;

class TVertexBlockResource : public TResource
{
public:
	TDECLARE_CLASS( TVertexBlockResource, TResource );
	TDECLARE_FREELIST_ALLOCATOR( TVertexBlockResource );

	static TVertexBlockResource* CreateNew()
	{
		return new TVertexBlockResource();
	}

public:
	struct HALBuffer
	{
		HALBuffer();

		TUINT					uiNumStreams;
		TUINT16					uiVertexOffset;
		IDirect3DVertexBuffer8* apVertexBuffers[ TVertexFactoryFormat::MAX_NUM_STREAMS ];
	};

public:
	TVertexBlockResource();

	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;
	virtual TBOOL TryInvalidate() override;
	virtual TBOOL TryValidate() override;

	TBOOL AttachPool( TVertexPoolResource* a_pPool );
	TBOOL DettachPool( TVertexPoolResource* a_pPool );
	TBOOL CanFit( TVertexPoolResource* a_pPoolResource ) const;

	void ChildVertexUsedChanged( TINT a_iChange );

	TBOOL Lock( TVertexPoolResourceInterface::LockBuffer* a_pLockBuffer, TUINT16 a_uiNumVertices );
	void  Unlock();

	TBOOL Create( TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT32 a_uiFlags );

	TBOOL CreateHAL();
	void  DestroyHAL();

	TBOOL GetHALBuffer( HALBuffer* a_pHALBuffer ) const;

private:
	inline static TINT s_iCurrentNumHALCreated;
	inline static TINT s_iTotalNumHALCreated;
	inline static TINT s_iTotalNumHALDestroyed;
	inline static TINT s_iCurrentVertexBufferBytesAllocated;
	inline static TINT s_iTotalVertexBufferBytesAllocated;
	inline static TINT s_iHALMemoryUsage;
	inline static TINT s_iWastedVertexBufferBytesAllocated;

protected:
	TVertexFactoryResourceInterface* m_pFactory;
	TUINT							 m_uiFlags;
	TUINT16							 m_uiMaxVertices;
	TUINT							 m_uiOffset;
	TUINT							 m_uiVerticesUsed;
	TUINT							 m_uiLockCount;
	TUINT							 m_Unk1;
	HALBuffer						 m_HALBuffer;
};

TOSHI_NAMESPACE_END
