#pragma once
#include "TResource.h"
#include "TVertexFactoryResourceInterface.h"

namespace Toshi {

	class TVertexPoolResourceInterface : public TResource
	{
	public:
		TDECLARE_CLASS(TVertexPoolResourceInterface, TResource);

	public:
		struct LockBuffer
		{
			LockBuffer();

			TUINT uiNumStreams;
			TUINT32 uiOffset;
			TBYTE* apStreams[TVertexFactoryFormat::MAX_NUM_STREAMS];
		};

	public:
		TVertexPoolResourceInterface();

		virtual void OnDestroy() override;
		virtual TBOOL Lock(LockBuffer* a_pLockBuffer) = 0;
		virtual void Unlock(TUINT16 a_uiNewNumVertices) = 0;
		virtual TBOOL Create(TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT16 a_uiFlags);

		TVertexFactoryResourceInterface* GetFactory() const { return m_pFactory; }
		TUINT16 GetFlags() const { return m_uiFlags; }
		TUINT16 GetMaxVertices() const { return m_uiMaxVertices; }
		TUINT16 GetNumVertices() const { return m_uiNumVertices; }
		TBOOL IsLocked() const { return m_uiLockCount != 0; }

	protected:
		TVertexFactoryResourceInterface* m_pFactory;
		TUINT16 m_uiFlags;
		TUINT16 m_uiMaxVertices;
		TUINT16 m_uiNumVertices;
		TUINT16 m_uiLockCount;
	};

}
