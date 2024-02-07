#pragma once
#include "Toshi/TNodeTree.h"

namespace Toshi
{
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
		TResource()
		{
			m_State = 0;
			m_UId = 0;
			m_Renderer = TNULL;
			m_Name[0] = 0;
		}

		virtual ~TResource();
		virtual TBOOL Create();
		virtual TBOOL Validate();
		virtual void Invalidate();
		virtual void DestroyResource();
		virtual TBOOL TryInvalidate() { return TTRUE; }
		virtual TBOOL TryValidate() { return TTRUE; }
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

		const char* GetName() const { return m_Name; }
		void SetName(const char* name);

		TBOOL RecurseSimple(t_RecurseCb a_pCallback, TResource* a_pResource, void* a_pUserData);
		static TBOOL Recurse(t_RecurseCb a_pCallback, TResource* a_pResource, TBOOL a_bFlag, void* a_pUserData);

		TRenderInterface* GetRenderer() const
		{
			return m_Renderer;
		}

		void SetRenderer(TRenderInterface* pRenderer)
		{
			m_Renderer = pRenderer;
		}

		TResource* Parent() const
		{
			return TNode::Parent();
		}

		TResource* GetNextResource() const
		{
			return TNode::Next();
		}

		TResource* GetChild() const
		{
			return TNode::Child();
		}

		TResource* GetLastResource() const
		{
			return TNode::Prev();
		}

		TNodeTree<TResource>* GetTree()
		{
			return TNode::m_Tree;
		}

		uint32_t GetUId() const
		{
			return m_UId;
		}

		void SetUId(uint32_t uid)
		{
			m_UId = uid;
		}

	private:
		TRenderInterface* m_Renderer;         // 0x18
		char m_Name[MAXNAMELEN + 1]; // 0x1C
		TResourceState m_State;      // 0x2B
		uint32_t m_UId;              // 0x2C
	};
}

