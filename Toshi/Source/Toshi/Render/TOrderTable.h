#pragma once
#include "TRenderPacket.h"
#include "TRegMaterial.h"

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

	private:
		TRegMaterial* m_pLastRegMat; // 0x20
	};
}