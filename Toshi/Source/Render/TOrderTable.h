#pragma once
#include "TRenderPacket.h"
#include "Toshi/TNodeList.h"
#include "Toshi/T2SimpleArray.h"

namespace Toshi
{
	class TOrderTable;

	class TRegMaterial : public TNodeList<TRegMaterial>::TNode
	{
	public:
		typedef TUINT32 State;
		enum State_ : State
		{
			State_Registered = BITFIELD(0),
			State_Used = BITFIELD(1)
		};

	public:
		TRegMaterial()
		{
			m_State = 0;
			m_pOrderTable = TNULL;
			m_pMaterial = TNULL;
			m_pLastRenderPacket = TNULL;
			m_pNextUsedMaterial = TNULL;
		}

		void Render();

		TRenderPacket* AddRenderPacket(TMesh* pMesh);

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

		TRegMaterial* GetNextUsedMaterial() const
		{
			return m_pNextUsedMaterial;
		}

		TMaterial* GetMaterial() const
		{
			return m_pMaterial;
		}

	private:
		State m_State;
		TOrderTable* m_pOrderTable;
		TMaterial* m_pMaterial;
		TRenderPacket* m_pLastRenderPacket;
		TRegMaterial* m_pNextUsedMaterial;
	};

	class TOrderTable : public TPriList<TOrderTable>::TNode
	{
	public:
		using t_PreFlushCallback = void(*)(void* pCustomData);
		using t_PostFlushCallback = void(*)(void* pCustomData);

	public:
		TOrderTable()
		{
			m_pLastRegMat = TNULL;
			m_pShader = TNULL;
			m_pLastRegMat = TNULL;
		}

		~TOrderTable()
		{
			if (s_pRegMaterials.GetArray())
			{
				DeregisterAllMaterials();
				s_llRegMatFreeList.RemoveAll();

				s_pRegMaterials.Destroy();
			}

			s_pRenderPackets.Destroy();
		}

		void Render();
		void Flush();

		TRegMaterial* RegisterMaterial(TMaterial* pMat);

		static void DeregisterMaterial(TRegMaterial* pRegMat);
		static void DeregisterAllMaterials();

		void UseMaterial(TRegMaterial* pRegMat)
		{
			if (!HASFLAG(pRegMat->GetFlags() & TRegMaterial::State_Used))
			{
				pRegMat->SetFlags(pRegMat->GetFlags() | TRegMaterial::State_Used);
				pRegMat->SetNextUsedMaterial(m_pLastRegMat);
				m_pLastRegMat = pRegMat;
			}
		}

	public:
		TBOOL Create(TShader* a_pShader, TINT a_iPriority);

	public:
		static void CreateStaticData(TUINT32 maxMaterials, TUINT32 maxRenderPackets);

		static TRenderPacket* AllocRenderPacket()
		{
			TRenderPacket* packet = &s_pRenderPackets[s_uiNumRenderPackets++];
			return s_uiNumRenderPackets < s_uiMaxRenderPackets ? &s_pRenderPackets[s_uiNumRenderPackets] : TNULL;
		}

	public:
		inline static TUINT32 s_uiMaxRenderPackets = 0;
		inline static TUINT32 s_uiNumRenderPackets = 0;
		inline static TUINT32 s_uiMaxNumRenderPackets = 0;
		inline static TUINT32 s_uiOrigMaxRenderPackets = 0;
		inline static T2SimpleArray<TRenderPacket> s_pRenderPackets;

		inline static TUINT32 s_uiMaxMaterials = 0;
		inline static TUINT32 s_uiNumRegisteredMaterials = 0;
		inline static T2SimpleArray<TRegMaterial> s_pRegMaterials;
		inline static TNodeList<TRegMaterial> s_llRegMatRegisteredList;
		inline static TNodeList<TRegMaterial> s_llRegMatFreeList;

	private:
		TShader* m_pShader;
		TRegMaterial* m_pLastRegMat;
	};
}