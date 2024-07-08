#include "ToshiPCH.h"
#include "Render/TShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	void TOrderTable::CreateStaticData(TUINT a_uiMaxMaterials, TUINT a_uiMaxRenderPackets)
	{
		s_uiMaxRenderPackets = a_uiMaxRenderPackets;
		s_uiNumRenderPackets = 0;
		
		s_pRenderPackets.Create(a_uiMaxRenderPackets);
		s_uiOrigMaxRenderPackets = a_uiMaxRenderPackets;

		s_pRegMaterials.Create(a_uiMaxMaterials);
		s_uiMaxMaterials = a_uiMaxMaterials;

		for (size_t i = 0; i < s_uiMaxMaterials; i++)
		{
			s_pRegMaterials[i].SetOrderTable(TNULL);
			s_llRegMatFreeList.InsertTail(s_pRegMaterials[i]);
		}
	}

	TRenderPacket* TOrderTable::AllocRenderPacket()
	{
		TASSERT(s_uiNumRenderPackets + 1 < s_uiMaxRenderPackets);
		TRenderPacket* packet = &s_pRenderPackets[s_uiNumRenderPackets++];
		return s_uiNumRenderPackets < s_uiMaxRenderPackets ? &s_pRenderPackets[s_uiNumRenderPackets] : TNULL;
	}

	TOrderTable::TOrderTable()
	{
		m_pLastRegMat = TNULL;
		m_pShader = TNULL;
	}

	TOrderTable::~TOrderTable()
	{
		if (s_pRegMaterials.GetArray())
		{
			DeregisterAllMaterials();
			s_llRegMatFreeList.RemoveAll();

			s_pRegMaterials.Destroy();
		}

		s_pRenderPackets.Destroy();
	}

	void TOrderTable::Render()
	{
		TPROFILER_SCOPE();

		for (auto it = m_pLastRegMat; it != TNULL; it = it->GetNextRegMat())
		{
			it->Render();
		}

		s_uiNumRenderPackets = 0;
		m_pLastRegMat = TNULL;
	}

	void TOrderTable::Flush()
	{
		TPROFILER_SCOPE();

		if (s_uiMaxNumRenderPackets < s_uiNumRenderPackets)
		{
			s_uiMaxNumRenderPackets = s_uiNumRenderPackets;
		}

		m_pShader->StartFlush();

		for (auto it = m_pLastRegMat; it != TNULL; it = it->GetNextRegMat())
		{
			it->Render();
		}

		m_pShader->EndFlush();

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
		if (HASANYFLAG(pRegMat->GetFlags(), TRegMaterial::State_Registered))
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

	void TOrderTable::DeregisterAllMaterials()
	{
		while (!s_llRegMatRegisteredList.IsEmpty())
		{
			DeregisterMaterial(s_llRegMatRegisteredList.Head());
		}
	}

	void TOrderTable::UseMaterial(TRegMaterial* a_pRegMat)
	{
		if (!HASANYFLAG(a_pRegMat->GetFlags(), TRegMaterial::State_Used))
		{
			a_pRegMat->SetFlags(a_pRegMat->GetFlags() | TRegMaterial::State_Used);
			a_pRegMat->SetNextRegMat(m_pLastRegMat);
			m_pLastRegMat = a_pRegMat;
		}
	}

	void TRegMaterial::Render()
	{
		TPROFILER_SCOPE();

		m_pMaterial->PreRender();

		for (auto pPacket = m_pLastRenderPacket; pPacket != TNULL; pPacket = pPacket->GetNextPacket())
		{
			pPacket->GetMesh()->GetOwnerShader()->Render(pPacket);
		}

		m_pLastRenderPacket = TNULL;
		SetFlags(m_State & ~State_Used);
		m_pMaterial->PostRender();
	}

	TRenderPacket* TRegMaterial::AddRenderPacket(TMesh* pMesh)
	{
		auto pPreviousPacket = m_pLastRenderPacket;

		m_pLastRenderPacket = TOrderTable::AllocRenderPacket();
		m_pLastRenderPacket->SetNextPacket(pPreviousPacket);

		m_pOrderTable->UseMaterial(this);
		m_pLastRenderPacket->SetMesh(pMesh);
		m_pLastRenderPacket->SetMaterial(pMesh->GetMaterial());

		return m_pLastRenderPacket;
	}

}
