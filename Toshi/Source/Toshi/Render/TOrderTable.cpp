#include "ToshiPCH.h"
#include "TOrderTable.h"

namespace Toshi
{
    void TOrderTable::CreateStaticData(uint32_t maxMaterials, uint32_t maxRenderPackets)
    {
        s_uiOrigMaxRenderPackets = s_uiMaxRenderPackets = maxRenderPackets;
        s_uiNumRenderPackets = 0;
        TTODO("INT_009a89c8 = 0;");

        s_pRenderPackets = (TRenderPacket*)TMemalign(32, maxRenderPackets * sizeof(TRenderPacket));

        s_uiMaxMaterials = maxMaterials;
        s_pRegMaterials = new TRegMaterial[s_uiMaxMaterials];

        for (size_t i = 0; i < s_uiMaxMaterials; i++)
        {
            s_pRegMaterials[i].SetOrderTable(TNULL);
            s_llRegMatFreeList.InsertTail(s_pRegMaterials[i]);
        }
    }

	void TOrderTable::Render()
	{
		for (auto it = m_pLastRegMat; it != TNULL; it = m_pLastRegMat->Next())
		{
			m_pLastRegMat->Render();
		}
		s_uiNumRenderPackets = 0;
		m_pLastRegMat = TNULL;
	}

	void TOrderTable::DeregisterMaterial(TRegMaterial* pRegMat)
	{
		if (HASFLAG(pRegMat->GetFlags() & TRegMaterial::State_Unk1))
		{
			TASSERT(s_uiNumRenderPackets == 0);
			TIMPLEMENT();
		}
	}

	void TRegMaterial::Render()
	{
		m_pMaterial->PreRender();

		for (auto it = m_pRenderPacket; it != TNULL; it = m_pRenderPacket->Next())
		{
			TIMPLEMENT();
		}

		m_pRenderPacket = TNULL;
		SetFlags(m_State & ~State_Used);
		m_pMaterial->PostRender();

	}

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