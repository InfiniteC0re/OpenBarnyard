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

		static void CreateStaticData(uint32_t maxMaterials, uint32_t maxRenderPackets);

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

		inline static uint32_t s_uiMaxRenderPackets = 0;
		inline static uint32_t s_uiNumRenderPackets = 0;
		inline static uint32_t s_uiOrigMaxRenderPackets = 0;
		inline static TRenderPacket* s_pRenderPackets = TNULL;

		inline static uint32_t s_uiMaxMaterials = 0;
		inline static TRegMaterial* s_pRegMaterials = TNULL;
		inline static TNodeList<TRegMaterial> s_RegMatFreeList = TNodeList<TRegMaterial>();

	private:
		TRegMaterial* m_pLastRegMat; // 0x20
	};
}