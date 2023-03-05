#pragma once
#include "Toshi/Core/TNodeList.h"
#include <Toshi/Render/TMaterial.h>
#include <Toshi/Render/TMesh.h>

namespace Toshi
{
	class TRegMaterial;

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

	class TOrderTable;

	class TRegMaterial : public TNodeList<TRegMaterial>::TNode
	{
	public:
		typedef uint32_t State;
		enum State_ : State
		{
			State_Unk1 = BITFIELD(0),
			State_Used = BITFIELD(1)
		};

	public:
		TRegMaterial() : TNode()
		{
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

		void Render();

		TRenderPacket* AddRenderPacket(TMesh* pMesh);

	private:
		State m_State;
		TOrderTable* m_pOrderTable;
		TMaterial* m_pMaterial;
		TRenderPacket* m_pRenderPacket;
		TRegMaterial* m_pNextUsedMaterial;
	};

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

		void Render();

		void DeregisterMaterial(TRegMaterial* pRegMat);

		void UseMaterial(TRegMaterial* pRegMat)
		{
			if (!HASFLAG(pRegMat->GetFlags() & TRegMaterial::State_Used))
			{
				pRegMat->SetFlags(TRegMaterial::State_Used);
				pRegMat->SetNextUsedMaterial(m_pLastRegMat);
				m_pLastRegMat = pRegMat;
			}
		}

		static TRenderPacket* AllocRenderPacket()
		{
			return s_uiNumRenderPackets++ < s_uiMaxRenderPackets ? &s_pRenderPackets[s_uiNumRenderPackets] : TNULL;
		}

		inline static uint32_t s_uiMaxRenderPackets = 0;
		inline static uint32_t s_uiNumRenderPackets = 0;
		inline static uint32_t s_uiOrigMaxRenderPackets = 0;
		inline static TRenderPacket* s_pRenderPackets = TNULL;

		inline static uint32_t s_uiNumMaterials = 0;
		inline static uint32_t s_uiMaxMaterials = 0;
		inline static TRegMaterial* s_pRegMaterials = TNULL;
		inline static TNodeList<TRegMaterial> s_llRegMatFreeList = TNodeList<TRegMaterial>();

	private:
		TRegMaterial* m_pLastRegMat; // 0x20
	};

	

	
}