#pragma once
#include "Toshi/TFreeList.h"
#include "Render/TResource.h"
#include "Render/TIndexPoolResourceInterface.h"
#include "Render/TIndexFactoryResourceInterface.h"

#include <d3d8.h>

namespace Toshi {

	class TIndexPoolResource;

	class TIndexBlockResource : public TResource
	{
	public:
		TDECLARE_CLASS( TIndexBlockResource, TResource );
		TDECLARE_FREELIST_ALLOCATOR( TIndexBlockResource );

		static TIndexBlockResource* CreateNew()
		{
			return new TIndexBlockResource();
		}

	public:
		struct HALBuffer
		{
			TUINT16 uiIndexOffset = 0;
			TUINT16 uiUnknown;
			IDirect3DIndexBuffer8* pIndexBuffer = TNULL;
		};

	private:
		TIndexBlockResource();

	public:
		virtual TBOOL Validate() override;
		virtual void Invalidate() override;
		virtual TBOOL TryInvalidate() override;
		virtual TBOOL TryValidate() override;

		TBOOL AttachPool(TIndexPoolResource* a_pPool);
		TBOOL DettachPool(TIndexPoolResource* a_pPool);
		TBOOL CanFit(TIndexPoolResource* a_pPoolResource) const;

		void ChildIndexUsedChanged(TINT a_iChange);

		TBOOL Lock(TIndexPoolResourceInterface::LockBuffer* a_pLockBuffer, TUINT16 a_uiNumIndices);
		void Unlock();

		TBOOL Create(TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT32 a_uiFlags);

		TBOOL CreateHAL();
		void DestroyHAL();

		TBOOL GetHALBuffer(HALBuffer* a_pHALBuffer) const;

	private:
		inline static TINT s_iHALMemoryUsage;

	public:
		TIndexFactoryResourceInterface* m_pFactory;
		TUINT m_uiFlags;
		TUINT16 m_uiMaxIndices;
		TUINT m_uiOffset;
		TUINT m_uiIndicesUsed;
		TUINT m_uiLockCount;
		TUINT m_Unk1;
		HALBuffer m_HALBuffer;
	};

}
