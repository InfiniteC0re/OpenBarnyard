#pragma once
#include "TVertexPoolResource_DX8.h"

#include "Toshi/Core/TFreeList.h"
#include "Toshi/Render/TResource.h"
#include "Toshi/Render/TVertexFactoryResourceInterface.h"

#include <d3d8.h>

namespace Toshi {

	TOBJECT(TVertexBlockResource, TResource, TTRUE)
	{
	public:
		void* operator new(size_t s) { return ms_oFreeList.New(sizeof(TVertexBlockResource)); }
		void* operator new(size_t s, void* where) { return where; }

		void operator delete(void* ptr) { ms_oFreeList.Delete(ptr); }
		void operator delete(void* ptr, void* where) { delete ptr; }

		static TFreeList ms_oFreeList;

	public:
		TVertexBlockResource();

		virtual TBOOL Validate() override;
		virtual void Invalidate() override;
		virtual TBOOL TryInvalidate() override;
		virtual TBOOL TryValidate() override;

		TBOOL AttachPool(TVertexPoolResource* a_pPool);
		TBOOL DettachPool(TVertexPoolResource* a_pPool);
		TBOOL CanFit(TVertexPoolResource* a_pPoolResource) const;

		void ChildVertexUsedChanged(TINT a_iChange);
		
		TBOOL Lock(TVertexPoolResourceInterface::LockBuffer * a_pLockBuffer, TUINT16 a_uiNumVertices);
		void Unlock();

		TBOOL Create(TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT32 a_uiFlags);
		TBOOL CreateHAL();

		void DestroyHAL();

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
		TUINT m_uiFlags;
		TUINT16 m_uiMaxVertices;
		TUINT m_uiOffset;
		TUINT m_uiVerticesUsed;
		TUINT m_uiLockCount;
		TUINT m_Unk1;
		TUINT m_uiNumStreams;
		TUINT16 m_Unk2;
		IDirect3DVertexBuffer8* m_apVertexBuffers[TVertexFactoryFormat::MAX_NUM_STREAMS];
	};

}
