#include "ToshiPCH.h"
#include "TRegMaterial.h"
#include "TOrderTable.h"
#include "TRenderPacket.h"

namespace Toshi
{
	TRenderPacket* TRegMaterial::AddRenderPacket(TMesh* pMesh)
	{
		m_pRenderPacket = TOrderTable::AllocRenderPacket();
		m_pRenderPacket->SetNext(m_pRenderPacket);

		m_pOrderTable->UseMaterial(this);
		m_pRenderPacket->SetMesh(pMesh);
		m_pRenderPacket->SetMaterial(pMesh->GetMaterial());

		return m_pRenderPacket;
	}
}