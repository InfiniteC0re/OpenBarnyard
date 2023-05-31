#pragma once
#include "Toshi/Shaders/SysShader/TSysMesh.h"

#include <d3d11.h>

namespace Toshi {

	class TSysMeshHAL : 
		public TGenericClassDerived<TSysMeshHAL, TSysMesh, "TSysMeshHAL", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		ID3D11Buffer* GetVertexBuffer() const
		{
			return m_pVertexBuffer;
		}

		ID3D11Buffer* GetIndexBuffer() const
		{
			return m_pIndexBuffer;
		}

		uint16_t GetIndexCount() const
		{
			return m_IndexCount;
		}

	private:
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		// ...
		uint16_t m_IndexCount;
		// ...
	};

}