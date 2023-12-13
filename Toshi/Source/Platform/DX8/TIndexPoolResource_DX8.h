#pragma once
#include "TIndexBlockResource_DX8.h"
#include "Toshi/Render/TIndexPoolResourceInterface.h"

namespace Toshi {

	class TIndexBlockResource;

	TOBJECT(TIndexPoolResource, TIndexPoolResourceInterface, TTRUE)
	{
	public:
		friend TIndexBlockResource;

	public:
		TIndexPoolResource();

		virtual TBOOL Validate() override;
		virtual void Invalidate() override;
		virtual void OnDestroy() override;
		virtual TBOOL Lock(LockBuffer * a_pLockBuffer) override;
		virtual void Unlock(TUINT16 a_uiNewNumIndices) override;
		virtual TBOOL Create(TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT16 a_uiFlags) override;

		TUINT16* GetIndices() { return (m_uiLockCount == 0) ? m_pIndices : TNULL; }

		TResource* GetParent();
		TIndexBlockResource* GetIndexBlock();

		TBOOL GetHALBuffer(TIndexBlockResource::HALBuffer* a_pHALBuffer);

	private:
		TUINT16 m_uiIndexOffset;
		TIndexType* m_pIndices;
		TUINT m_uiNumLocksAllTime;
	};

}
