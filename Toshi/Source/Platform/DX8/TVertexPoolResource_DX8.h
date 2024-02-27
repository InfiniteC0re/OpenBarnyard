#pragma once
#include "TVertexBlockResource_DX8.h"

#include "Render/TVertexPoolResourceInterface.h"

namespace Toshi {

	class TVertexBlockResource;

	class TVertexPoolResource : public TVertexPoolResourceInterface
	{
	public:
		TDECLARE_CLASS(TVertexPoolResourceInterface);

	public:
		friend TVertexBlockResource;

	public:
		TVertexPoolResource();

		virtual TBOOL Validate() override;
		virtual void Invalidate() override;
		virtual void OnDestroy() override;
		virtual TBOOL Lock(LockBuffer * a_pLockBuffer) override;
		virtual void Unlock(TUINT16 a_uiNewNumVertices) override;
		virtual TBOOL Create(TVertexFactoryResourceInterface * a_pFactory, TUINT16 a_uiMaxVertices, TUINT16 a_uiFlags) override;

		TBYTE* GetManagedStream(TUINT a_uiStream);

		TResource* GetParent();
		TVertexBlockResource* GetVertexBlock();

		TBOOL GetHALBuffer(TVertexBlockResource::HALBuffer* a_pHALBuffer);

		static TVertexPoolResource* Upcast(TVertexPoolResourceInterface* a_pResource)
		{
			TASSERT(a_pResource->IsA(&ms_oClass));
			return TSTATICCAST(TVertexPoolResource*, a_pResource);
		}

	private:
		TUINT16 m_uiVertexOffset;
		TBYTE* m_apManagedStreams[TVertexFactoryFormat::MAX_NUM_STREAMS];
		TUINT m_uiNumLocksAllTime;
	};

}