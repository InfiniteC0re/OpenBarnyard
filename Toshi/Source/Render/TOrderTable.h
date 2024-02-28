#pragma once
#include "TRenderPacket.h"
#include "Toshi/TNodeList.h"
#include "Toshi/T2SimpleArray.h"

namespace Toshi
{
	class TOrderTable;

	//-----------------------------------------------------------------------------
	// Purpose: tracks all of the buffered render commands of specified TMaterial
	//-----------------------------------------------------------------------------
	class TRegMaterial : public TNodeList<TRegMaterial>::TNode
	{
	public:
		typedef TUINT32 State;
		enum State_ : State
		{
			State_Registered = BITFLAG(0),
			State_Used = BITFLAG(1)
		};

	public:
		TRegMaterial()
		{
			m_State = 0;
			m_pOrderTable = TNULL;
			m_pMaterial = TNULL;
			m_pLastRenderPacket = TNULL;
			m_pNextRegMat = TNULL;
		}

		// Renders all of the render packets attached to this registered material
		void Render();

		// Allocates new render packet (command) for the specified mesh
		TRenderPacket* AddRenderPacket(TMesh* a_pMesh);

		State GetFlags() const { return m_State; }
		void SetFlags(State a_eFlags) { m_State = a_eFlags; }

		TOrderTable* GetOrderTable() const { return m_pOrderTable; }
		void SetOrderTable(TOrderTable* a_pOrderTable) { m_pOrderTable = a_pOrderTable; }

		TMaterial* GetMaterial() const { return m_pMaterial; }
		void SetMaterial(TMaterial* a_pMaterial) { m_pMaterial = a_pMaterial; }

		TRegMaterial* GetNextRegMat() const { return m_pNextRegMat; }
		void SetNextRegMat(TRegMaterial* a_pRegMat) { m_pNextRegMat = a_pRegMat; }

	private:
		State m_State;
		TOrderTable* m_pOrderTable;
		TMaterial* m_pMaterial;
		TRenderPacket* m_pLastRenderPacket;
		TRegMaterial* m_pNextRegMat;
	};

	//-----------------------------------------------------------------------------
	// Purpose: helps to order rendering models of specified shader using priority.
	// Registers TMaterial instances to render all render packets attached to them.
	//-----------------------------------------------------------------------------
	class TOrderTable : public TPriList<TOrderTable>::TNode
	{
	public:
		using t_PreFlushCallback = void(*)(void* a_pCustomData);
		using t_PostFlushCallback = void(*)(void* a_pCustomData);

	public:
		TOrderTable();
		~TOrderTable();

		// Renders all render packets of registered materials
		void Render();

		// Flushes the shader before rendering
		void Flush();

		// Registers material and returns registered material which can be used to alloc render commands
		TRegMaterial* RegisterMaterial(TMaterial* a_pMat);

		static void DeregisterMaterial(TRegMaterial* a_pRegMat);
		static void DeregisterAllMaterials();

		// Marks the registered material as used and links it to this order table.
		// Note: before registered material is used, it won't render
		void UseMaterial(TRegMaterial* a_pRegMat);

		// Registers this order table in the renderer and assigns priority to it
		TBOOL Create(TShader* a_pShader, TINT a_iPriority);

		// Allocate memory for storing render packets and registered materials
		static void CreateStaticData(TUINT a_uiMaxMaterials, TUINT a_uiMaxRenderPackets);

		// Allocates render packet from the memory allocated by CreateStaticData
		static TRenderPacket* AllocRenderPacket()
		{
			TRenderPacket* packet = &s_pRenderPackets[s_uiNumRenderPackets++];
			return s_uiNumRenderPackets < s_uiMaxRenderPackets ? &s_pRenderPackets[s_uiNumRenderPackets] : TNULL;
		}

	public:
		inline static TUINT s_uiMaxRenderPackets = 0;
		inline static TUINT s_uiNumRenderPackets = 0;
		inline static TUINT s_uiMaxNumRenderPackets = 0;
		inline static TUINT s_uiOrigMaxRenderPackets = 0;
		inline static T2SimpleArray<TRenderPacket> s_pRenderPackets;

		inline static TUINT s_uiMaxMaterials = 0;
		inline static TUINT s_uiNumRegisteredMaterials = 0;
		inline static T2SimpleArray<TRegMaterial> s_pRegMaterials;
		inline static TNodeList<TRegMaterial> s_llRegMatRegisteredList;
		inline static TNodeList<TRegMaterial> s_llRegMatFreeList;

	private:
		TShader* m_pShader;
		TRegMaterial* m_pLastRegMat;
	};
}