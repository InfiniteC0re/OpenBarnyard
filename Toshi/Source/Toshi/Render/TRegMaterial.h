#pragma once
#include "TMaterial.h"
#include "TMesh.h"

namespace Toshi
{
	class TRenderPacket;
	class TOrderTable;

	class TRegMaterial
	{
	public:
		typedef uint32_t State;
		enum State_ : State
		{
			State_Unk1 = BITFIELD(0),
			State_Used = BITFIELD(1),
		};

	public:
		TRegMaterial()
		{
			m_Unk1 = TNULL;
			m_Unk2 = this;
			m_Unk3 = this;
			m_State = 0;
			m_pOrderTable = TNULL;
			m_pMaterial = TNULL;
			m_pRenderPacket = TNULL;
			m_pNextUsedMaterial = TNULL;
		}

		State GetFlags() const
		{
			return m_State;
		}

		void SetFlags(State flags)
		{
			m_State = flags;
		}

		void SetOrderTable(TOrderTable* pOrderTable)
		{
			m_pOrderTable = pOrderTable;
		}

		void SetMaterial(TMaterial* pMaterial)
		{
			m_pMaterial = pMaterial;
		}

		void SetNextUsedMaterial(TRegMaterial* pRegMat)
		{
			m_pNextUsedMaterial = pRegMat;
		}

		TRenderPacket* AddRenderPacket(TMesh* pMesh);

	private:
		void* m_Unk1;
		void* m_Unk2;
		void* m_Unk3;
		State m_State;
		TOrderTable* m_pOrderTable;
		TMaterial* m_pMaterial;
		TRenderPacket* m_pRenderPacket;
		TRegMaterial* m_pNextUsedMaterial;
	};
}