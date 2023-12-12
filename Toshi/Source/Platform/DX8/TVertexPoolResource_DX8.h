#pragma once
#include "Toshi/Render/TVertexPoolResourceInterface.h"

namespace Toshi {

	class TVertexBlockResource;

	TOBJECT(TVertexPoolResource, TVertexPoolResourceInterface, TTRUE)
	{
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

		unsigned char* GetManagedStream(TUINT a_uiStream);

		TResource* GetParent();
		TVertexBlockResource* GetVertexBlock();

	private:
		TUINT16 m_uiVertexOffset;
		unsigned char* m_apManagedStreams[TVertexFactoryFormat::MAX_NUM_STREAMS];
		TUINT m_uiNumLocksAllTime;
	};

}