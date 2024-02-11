#pragma once
#include "Toshi/TNodeTree.h"

namespace Toshi {

	class TRenderInterface;

	typedef uint8_t TResourceState;
	enum TResourceState_ : TResourceState
	{
		TResourceState_Valid       = BITFIELD(0),
		TResourceState_Created     = BITFIELD(1),
		TResourceState_Dying       = BITFIELD(2),
		TResourceState_External    = BITFIELD(3),
		TResourceState_Dead        = BITFIELD(4),
		TResourceState_SceneObject = BITFIELD(5),
	};

	class TResource :
		public TObject,
		public TNodeTree<TResource>::TNode
	{
	public:
		TDECLARE_CLASS(TObject);

		static constexpr size_t MAXNAMELEN = 14;
		friend TRenderInterface;

		using t_RecurseCb = TBOOL(*)(TResource* a_pResource, void* a_pUserData);

	public:
		TResource();
		~TResource();

		virtual TBOOL Create();
		virtual TBOOL Validate();
		virtual void Invalidate();
		virtual void DestroyResource();
		virtual TBOOL TryInvalidate();
		virtual TBOOL TryValidate();
		virtual void OnDestroy();

		TBOOL IsDead() const { return m_State & TResourceState_Dead; }
		TBOOL IsDying() const { return m_State & TResourceState_Dying; }
		TBOOL IsValid() const { return m_State & TResourceState_Valid; }
		TBOOL IsExternal() const { return m_State & TResourceState_External; }
		TBOOL IsCreated() const { return m_State & TResourceState_Created; }
		TBOOL IsSceneObject() const { return m_State & TResourceState_SceneObject; }
		TBOOL IsInvalid() const { return ~m_State & TResourceState_Valid; }

		void SetState(TResourceState newState) { m_State = newState; }
		void AddState(TResourceState state) { m_State |= state; }
		
		void SetParent(TResource* a_pParent);

		const TCHAR* GetName() const { return m_Name; }
		void SetName(const TCHAR* name);

		TBOOL RecurseSimple(t_RecurseCb a_pCallback, TResource* a_pResource, void* a_pUserData);
		static TBOOL Recurse(t_RecurseCb a_pCallback, TResource* a_pResource, TBOOL a_bFlag, void* a_pUserData);

		TRenderInterface* GetRenderer() const;
		void SetRenderer(TRenderInterface* pRenderer);

		TUINT32 GetUId() const;
		void SetUId(TUINT32 uid);

	private:
		TRenderInterface* m_pRenderer; // 0x18
		TCHAR m_Name[MAXNAMELEN + 1];  // 0x1C
		TResourceState m_State;        // 0x2B
		TUINT32 m_UId;                 // 0x2C
	};

}

