#pragma once
#include "Toshi/Core/TNodeList.h"
#include "Toshi/Render/TMaterial.h"
#include "Toshi/Render/TMesh.h"

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

		TMatrix44& GetModelViewMatrix()
		{
			return m_ModelViewMatrix;
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
			State_Registered = BITFIELD(0),
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

		TMaterial* GetMaterial() const
		{
			return m_pMaterial;
		}

	private:
		State m_State;
		TOrderTable* m_pOrderTable;
		TMaterial* m_pMaterial;
		TRenderPacket* m_pRenderPacket;
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
			m_iPriority = 0;
			m_pShader = TNULL;
			m_fnPreFlushCallback = TNULL;
			m_pPreFlushCallbackData = TNULL;
			m_fnPostFlushCallback = TNULL;
			m_pPostFlushCallbackData = TNULL;
			m_pLastRegMat = TNULL;
		}

		~TOrderTable()
		{
			if (s_pRegMaterials)
			{
				delete[] s_pRegMaterials;
				s_pRegMaterials = TNULL;
			}

			if (s_pRenderPackets)
			{
				TFree(s_pRenderPackets);
				s_pRenderPackets = TNULL;
			}
		}

		static void CreateStaticData(uint32_t maxMaterials, uint32_t maxRenderPackets);

		TBOOL Create(TShader* pShader, int priority, uint8_t index);
		void Render();
		void Flush();

		TRegMaterial* RegisterMaterial(TMaterial* pMat);
		static void DeregisterMaterial(TRegMaterial* pRegMat);

		void UseMaterial(TRegMaterial* pRegMat)
		{
			if (!HASFLAG(pRegMat->GetFlags() & TRegMaterial::State_Used))
			{
				pRegMat->SetFlags(TRegMaterial::State_Used);
				pRegMat->SetNextUsedMaterial(m_pLastRegMat);
				m_pLastRegMat = pRegMat;
			}
		}

		void SetPreFlushCallback(t_PreFlushCallback fnCallback, void* pCustomData = nullptr)
		{
			m_fnPreFlushCallback = fnCallback;
			m_pPreFlushCallbackData = pCustomData;
		}

		void SetPostFlushCallback(t_PostFlushCallback fnCallback, void* pCustomData = nullptr)
		{
			m_fnPostFlushCallback = fnCallback;
			m_pPostFlushCallbackData = pCustomData;
		}

		static TRenderPacket* AllocRenderPacket()
		{
			TRenderPacket* packet = &s_pRenderPackets[s_uiNumRenderPackets++];
			return s_uiNumRenderPackets < s_uiMaxRenderPackets ? &s_pRenderPackets[s_uiNumRenderPackets] : TNULL;
		}

	public:
		inline static uint32_t s_uiMaxRenderPackets = 0;
		inline static uint32_t s_uiNumRenderPackets = 0;
		inline static uint32_t s_uiOrigMaxRenderPackets = 0;
		inline static TRenderPacket* s_pRenderPackets = TNULL;

		inline static uint32_t s_uiNumMaterials = 0;
		inline static uint32_t s_uiMaxMaterials = 0;
		inline static uint32_t s_uiNumRegisteredMaterials = 0;
		inline static TRegMaterial* s_pRegMaterials = TNULL;
		inline static TNodeList<TRegMaterial> s_llRegMatRegisteredList;
		inline static TNodeList<TRegMaterial> s_llRegMatFreeList;

	private:
		TShader* m_pShader;
		t_PreFlushCallback m_fnPreFlushCallback;
		void* m_pPreFlushCallbackData;
		t_PostFlushCallback m_fnPostFlushCallback;
		void* m_pPostFlushCallbackData;
		TRegMaterial* m_pLastRegMat;
		uint8_t m_iIndex;
	};
}