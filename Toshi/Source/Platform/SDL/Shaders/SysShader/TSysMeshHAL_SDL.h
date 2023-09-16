#pragma once
#include "Toshi/Shaders/SysShader/TSysMesh.h"

namespace Toshi {

	class TSysMeshHAL : 
		public TGenericClassDerived<TSysMeshHAL, TSysMesh, "TSysMeshHAL", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		virtual TBOOL Render() override;
		virtual TBOOL Create(FLAGS uiFlags, uint16_t uiMaxVertices, uint16_t uiMaxIndices) override;
		virtual TBOOL Lock(TLockBuffer* pBufferLock) override;
		virtual void Unlock(uint16_t uiNumVertices, uint16_t uiNumIndices) override;
		virtual void SetZBias(int zBias) override;
		virtual void OnDestroy() override;

		/*ID3D11Buffer* GetVertexBuffer() const
		{
			return m_pVertexBuffer;
		}

		ID3D11Buffer* GetIndexBuffer() const
		{
			return m_pIndexBuffer;
		}*/

		uint16_t GetNumVertices() const
		{
			return m_NumVertices;
		}

		uint16_t GetIndexCount() const
		{
			return m_IndexCount;
		}

	private:
		inline static TBOOL s_bStopRendering = TFALSE;

	private:
		/*ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;*/
		uint16_t m_NumVertices;
		uint16_t m_IndexCount;
		// ...
	};

}