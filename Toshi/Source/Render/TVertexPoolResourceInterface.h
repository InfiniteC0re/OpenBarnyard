#pragma once
#include "TResource.h"
#include "TVertexFactoryResourceInterface.h"

TOSHI_NAMESPACE_START

class MODAPI TVertexPoolResourceInterface : public TResource
{
public:
	TDECLARE_CLASS( TVertexPoolResourceInterface, TResource );

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE           = 0,
		FLAGS_STATIC         = BITFLAG( 0 ), // shares a managed block, keeps a CPU copy
		FLAGS_DYNAMIC        = BITFLAG( 1 ), // own block, discarded on every lock
		FLAGS_DYNAMIC_APPEND = BITFLAG( 2 ), // appended to a block with D3DLOCK_NOOVERWRITE (DX8 pools turn it into FLAGS_DYNAMIC)
		FLAGS_USAGE_MASK     = FLAGS_STATIC | FLAGS_DYNAMIC | FLAGS_DYNAMIC_APPEND,
	};

public:
	struct LockBuffer
	{
		LockBuffer();

		TUINT   uiNumStreams;
		TUINT32 uiOffset;
		TBYTE*  apStreams[ TVertexFactoryFormat::MAX_NUM_STREAMS ];
	};

public:
	TVertexPoolResourceInterface();
	~TVertexPoolResourceInterface();

	virtual void  OnDestroy() override;
	virtual TBOOL Lock( LockBuffer* a_pLockBuffer )    = 0;
	virtual void  Unlock( TUINT16 a_uiNewNumVertices ) = 0;
	virtual TBOOL Create( TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT16 a_uiFlags );

	TVertexFactoryResourceInterface* GetFactory() const { return m_pFactory; }
	FLAGS                            GetFlags() const { return m_uiFlags; }
	TUINT16                          GetMaxVertices() const { return m_uiMaxVertices; }
	TUINT16                          GetNumVertices() const { return m_uiNumVertices; }
	TBOOL                            IsLocked() const { return m_uiLockCount != 0; }

protected:
	TVertexFactoryResourceInterface* m_pFactory;
	FLAGS                            m_uiFlags;
	TUINT16                          m_uiMaxVertices;
	TUINT16                          m_uiNumVertices;
	TUINT16                          m_uiLockCount;
};

TOSHI_NAMESPACE_END
