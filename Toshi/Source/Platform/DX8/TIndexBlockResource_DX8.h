#pragma once
#include "TIndexPoolResource_DX8.h"

#include "Toshi/Core/TFreeList.h"
#include "Toshi/Render/TResource.h"
#include "Toshi/Render/TIndexFactoryResourceInterface.h"

#include <d3d8.h>

namespace Toshi {

	TOBJECT(TIndexBlockResource, TResource, TTRUE)
	{
	public:
		void* operator new(size_t s) { return ms_oFreeList.New(sizeof(TIndexBlockResource)); }
		void* operator new(size_t s, void* where) { return where; }

		void operator delete(void* ptr) { ms_oFreeList.Delete(ptr); }
		void operator delete(void* ptr, void* where) { delete ptr; }

		static TFreeList ms_oFreeList;

	public:
		TIndexBlockResource();

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
		TUINT16 m_Unk2;
		IDirect3DIndexBuffer8* m_pIndexBuffer;
	};

}
