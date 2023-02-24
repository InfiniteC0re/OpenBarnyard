#pragma once
#include "TRenderPacket.h"
#include "TRegMaterial.h"
#include "Toshi/Core/TFreeList.h"
#include "Toshi/Core/TNodeList.h"

namespace Toshi
{
	class TOrderTable
	{
	public:
		TOrderTable()
		{
			m_pLastRegMat = TNULL;
			TIMPLEMENT();
		}

		void CreateStaticData(uint32_t maxMaterials, uint32_t maxRenderPackets);

		~TOrderTable()
		{
			TIMPLEMENT();
		}

		void UseMaterial(TRegMaterial* pRegMat)
		{
			if ((pRegMat->GetFlags() & TRegMaterial::State_Used) == 0)
			{
				pRegMat->SetFlags(TRegMaterial::State_Used);
				pRegMat->SetNextUsedMaterial(m_pLastRegMat);
				m_pLastRegMat = pRegMat;
			}
		}

		static TRenderPacket* AllocRenderPacket()
		{
			TIMPLEMENT();
			return TNULL;
		}

		static uint32_t s_uiMaxRenderPackets;
		static uint32_t s_uiNumRenderPackets;
		static uint32_t s_uiOrigMaxRenderPackets;
		static TRenderPacket* s_pRenderPackets;
		
		static uint32_t s_uiMaxMaterials;
		static TNodeList<TRegMaterial*> s_pRegMaterials;
		static TFreeList s_RegMatFreeList;

	private:
		TRegMaterial* m_pLastRegMat; // 0x20
	};
}