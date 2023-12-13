#pragma once
#include "TResource.h"
#include "TIndexFactoryResourceInterface.h"

namespace Toshi {

	TOBJECT(TIndexPoolResourceInterface, TResource, TFALSE)
	{
	public:
		struct LockBuffer
		{
			TUINT uiOffset;
			TIndexType* pBuffer;
		};

	public:
		TIndexPoolResourceInterface();

		virtual void OnDestroy() override;
		virtual TBOOL Lock(LockBuffer* a_pLockBuffer) = 0;
		virtual void Unlock(TUINT16 a_uiNewNumIndices) = 0;
		virtual TBOOL Create(TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT16 a_uiFlags);

		TIndexFactoryResourceInterface* GetFactory() const { return m_pFactory; }
		TUINT16 GetFlags() const { return m_uiFlags; }
		TUINT16 GetMaxIndices() const { return m_uiMaxIndices; }
		TUINT16 GetNumIndices() const { return m_uiNumIndices; }
		TBOOL IsLocked() const { return m_uiLockCount != 0; }

	protected:
		TIndexFactoryResourceInterface* m_pFactory;
		TUINT16 m_uiFlags;
		TUINT16 m_uiMaxIndices;
		TUINT16 m_uiNumIndices;
		TUINT16 m_uiLockCount;
	};

}
