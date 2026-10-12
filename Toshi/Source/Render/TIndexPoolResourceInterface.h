#pragma once
#include "TResource.h"
#include "TIndexFactoryResourceInterface.h"

TOSHI_NAMESPACE_START

class TIndexPoolResourceInterface : public TResource
{
public:
	TDECLARE_CLASS( TIndexPoolResourceInterface, TResource );

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE           = 0,
		FLAGS_STATIC         = BITFLAG( 0 ), // shares a managed block, keeps a CPU copy
		FLAGS_DYNAMIC        = BITFLAG( 1 ), // own block, discarded on every lock
		FLAGS_DYNAMIC_APPEND = BITFLAG( 2 ), // appended to a block with D3DLOCK_NOOVERWRITE (DX8 pools turn it into FLAGS_DYNAMIC)
		FLAGS_TRISTRIP       = BITFLAG( 3 ),
		FLAGS_TRILIST        = BITFLAG( 4 ),
		FLAGS_USAGE_MASK     = FLAGS_STATIC | FLAGS_DYNAMIC | FLAGS_DYNAMIC_APPEND,
	};

public:
	struct LockBuffer
	{
		TUINT       uiOffset;
		TIndexType* pBuffer;
	};

public:
	TIndexPoolResourceInterface();
	~TIndexPoolResourceInterface();

	virtual void  OnDestroy() override;
	virtual TBOOL Lock( LockBuffer* a_pLockBuffer )   = 0;
	virtual void  Unlock( TUINT16 a_uiNewNumIndices ) = 0;
	virtual TBOOL Create( TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT16 a_uiFlags );

	TIndexFactoryResourceInterface* GetFactory() const { return m_pFactory; }
	FLAGS                           GetFlags() const { return m_uiFlags; }
	TUINT16                         GetMaxIndices() const { return m_uiMaxIndices; }
	TUINT16                         GetNumIndices() const { return m_uiNumIndices; }
	TBOOL                           IsLocked() const { return m_uiLockCount != 0; }

protected:
	TIndexFactoryResourceInterface* m_pFactory;
	FLAGS                           m_uiFlags;
	TUINT16                         m_uiMaxIndices;
	TUINT16                         m_uiNumIndices;
	TUINT16                         m_uiLockCount;
};

TOSHI_NAMESPACE_END
