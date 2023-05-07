#include "ToshiPCH.h"
#include "TOrderTable.h"
#include "TShader.h"

namespace Toshi
{
    void TOrderTable::CreateStaticData(uint32_t maxMaterials, uint32_t maxRenderPackets)
    {
        s_uiOrigMaxRenderPackets = s_uiMaxRenderPackets = maxRenderPackets;
        s_uiNumRenderPackets = 0;
		s_uiNumMaterials = 0;

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

	void TOrderTable::Flush()
	{
		TTODO("(*(code *)param_1->field3_0x10)(param_1,param_1->field4_0x14);");

		if (m_pLastRegMat != TNULL)
		{
			TTODO("(**(code **)(*(int *)param_1->field2_0xc + 0x14))();");

			for (auto it = m_pLastRegMat; it != TNULL; it = m_pLastRegMat->Next())
			{
				m_pLastRegMat->Render();
			}

			TTODO("(**(code **)(*(int *)param_1->field2_0xc + 0x18))();");
		}

		TTODO("(*(code *)param_1->field5_0x18)(param_1,param_1->field6_0x1c);");

		s_uiNumRenderPackets = 0;
		m_pLastRegMat = TNULL;
	}

	TRegMaterial* TOrderTable::RegisterMaterial(TMaterial* pMat)
	{
		TASSERT(s_uiNumRenderPackets == 0);

		TRegMaterial* pCurrentRegMat = pMat->GetRegMaterial();
		if (pCurrentRegMat != nullptr)
			return pCurrentRegMat;

		TASSERT(TFALSE == s_llRegMatFreeList.IsEmpty());

		if (s_llRegMatFreeList.IsEmpty())
			return nullptr;

		TRegMaterial* pRegMat = s_llRegMatFreeList.RemoveHead()->As<TRegMaterial>();
		s_llRegMatRegisteredList.InsertTail(*pRegMat);
		s_uiNumRegisteredMaterials += 1;

		pRegMat->SetFlags(TRegMaterial::State_Registered);
		pRegMat->SetMaterial(pMat);
		pRegMat->SetOrderTable(this);
		pMat->SetRegMaterial(pRegMat);

		return pRegMat;
	}

	void TOrderTable::DeregisterMaterial(TRegMaterial* pRegMat)
	{
		if (HASFLAG(pRegMat->GetFlags() & TRegMaterial::State_Registered))
		{
			TASSERT(s_uiNumRenderPackets == 0);
			
			auto pMaterial = pRegMat->GetMaterial();

			pRegMat->Remove();
			s_llRegMatFreeList.InsertHead(*pRegMat);
			s_uiNumRegisteredMaterials -= 1;

			pRegMat->SetFlags(0);
			pRegMat->SetMaterial(nullptr);
			pRegMat->SetOrderTable(nullptr);
			pMaterial->SetRegMaterial(nullptr);
		}
	}

	void TRegMaterial::Render()
	{
		m_pMaterial->PreRender();

		for (auto pPacket = m_pRenderPacket; pPacket != TNULL; pPacket = m_pRenderPacket->Next())
		{
			pPacket->GetMesh()->GetOwnerShader()->Render(pPacket);
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