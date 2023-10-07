#include "ToshiPCH.h"
#include "Toshi/Render/TShader.h"

namespace Toshi {

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
		if (m_fnPreFlushCallback)
			m_fnPreFlushCallback(m_pPreFlushCallbackData);

		if (m_pLastRegMat != TNULL)
		{
			m_pShader->StartFlush();

			for (auto it = m_pLastRegMat; it != TNULL; it = it->GetNextUsedMaterial())
			{
				it->Render();
			}

			m_pShader->EndFlush();
		}

		if (m_fnPostFlushCallback)
			m_fnPostFlushCallback(m_pPostFlushCallbackData);

		s_uiNumRenderPackets = 0;
		m_pLastRegMat = TNULL;
	}

	TRegMaterial* TOrderTable::RegisterMaterial(TMaterial* pMat)
	{
		TASSERT(s_uiNumRenderPackets == 0);

		TRegMaterial* pCurrentRegMat = pMat->GetRegMaterial();
		if (pCurrentRegMat != TNULL)
			return pCurrentRegMat;

		TASSERT(TFALSE == s_llRegMatFreeList.IsEmpty());

		if (s_llRegMatFreeList.IsEmpty())
			return TNULL;

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
			pRegMat->SetMaterial(TNULL);
			pRegMat->SetOrderTable(TNULL);
			pMaterial->SetRegMaterial(TNULL);
		}
	}

	void TRegMaterial::Render()
	{
		m_pMaterial->PreRender();

		for (auto pPacket = m_pLastRenderPacket; pPacket != TNULL; pPacket = pPacket->Next())
			pPacket->GetMesh()->GetOwnerShader()->Render(pPacket);

		m_pLastRenderPacket = TNULL;
		SetFlags(m_State & ~State_Used);
		m_pMaterial->PostRender();
	}

	TRenderPacket* TRegMaterial::AddRenderPacket(TMesh* pMesh)
	{
		auto pPreviousPacket = m_pLastRenderPacket;

		m_pLastRenderPacket = TOrderTable::AllocRenderPacket();
		m_pLastRenderPacket->SetNext(pPreviousPacket);

		m_pOrderTable->UseMaterial(this);
		m_pLastRenderPacket->SetMesh(pMesh);
		m_pLastRenderPacket->SetMaterial(pMesh->GetMaterial());

		return m_pLastRenderPacket;
	}

}
