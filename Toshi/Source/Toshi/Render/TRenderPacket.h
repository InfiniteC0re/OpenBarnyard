#pragma once
#include "TMesh.h"
#include "TRegMaterial.h"

namespace Toshi
{
	class TRenderPacket
	{
	public:
		void SetModelViewMatrix(TMatrix44* pMatrix44)
		{
			m_ModelViewMatrix = *pMatrix44;
		}

		void SetMesh(TMesh* pMesh)
		{
			m_pMesh = pMesh;
		}

		void SetNext(TRenderPacket* pNextPacket)
		{
			m_pNextPacket = pNextPacket;
		}

		void SetMaterial(TMaterial* pMaterial)
		{
			m_pMaterial = pMaterial;
		}

		TRenderPacket* Next() const
		{
			return m_pNextPacket;
		}

		TMesh* GetMesh() const
		{
			return m_pMesh;
		}

		TMaterial* GetMaterial() const
		{
			return m_pMaterial;
		}

		TMatrix44* GetModelViewMatrix()
		{
			return &m_ModelViewMatrix;
		}

	private:
		TRenderPacket* m_pNextPacket; // 0x00
		TMesh* m_pMesh;               // 0x08
		TMatrix44 m_ModelViewMatrix;  // 0x10
		TMaterial* m_pMaterial;       // 0x50

	};
}