#pragma once
#include "Toshi/Render/TRender.h"
#include "Toshi/Core/TNodeTree.h"

namespace Toshi
{
	typedef uint8_t TResourceState;
	enum TResourceState_ : TResourceState
	{
		TResourceState_Valid       = BITFIELD(0),
		TResourceState_Created     = BITFIELD(1),
		TResourceState_Dying       = BITFIELD(2),
		TResourceState_Unk2        = BITFIELD(3),
		TResourceState_Dead        = BITFIELD(4),
		TResourceState_SceneObject = BITFIELD(5),
	};

	class TResource :
		public TGenericClassDerived<TResource, TObject, "TResource", TMAKEVERSION(1, 0), true>,
		public TNodeTree<TResource>::TNode
	{
	public:
		static constexpr size_t MAXNAMELEN = 14;

	public:
		virtual ~TResource();
		virtual bool Create();
		virtual bool Validate();
		virtual void Invalidate();
		virtual void DestroyResource();
		virtual void OnDestroy();

		bool IsDead() const { return m_State & TResourceState_Dead; }
		bool IsDying() const { return m_State & TResourceState_Dying; }
		bool IsValid() const { return m_State & TResourceState_Valid; }
		bool IsCreated() const { return m_State & TResourceState_Created; }
		bool IsSceneObject() const { return m_State & TResourceState_SceneObject; }
		bool IsInvalid() const { return ~m_State & TResourceState_Valid; }

		void SetState(TResourceState newState) { m_State = newState; }
		void AddState(TResourceState state) { m_State |= state; }
		
		void SetParent(TResource* parent) { m_Parent = parent; }

		const char* GetName() const { return m_Name; }
		void SetName(const char* name);

		TRender* GetRenderer() const
		{
			return m_Renderer;
		}

		TResource* Parent() const
		{
			return m_Parent == TNULL ? m_Tree.Root()->Parent() : m_Parent;
		}

		TResource* GetNextResource() const
		{
			return m_NextResource;
		}

		TResource* GetAttached() const
		{
			return m_Attached;
		}

		TResource* GetLastResource() const
		{
			return m_LastResource;
		}

		TNodeTree<TResource>::TNode* GetTree()
		{
			return m_Tree.Root();
		}

	private:
		TNodeTree<TResource> m_Tree; // 0x04
		TResource* m_NextResource;   // 0x08
		TResource* m_LastResource;   // 0x0C
		TResource* m_Parent;         // 0x10
		TResource* m_Attached;       // 0x14
		TRender* m_Renderer;         // 0x18
		char m_Name[MAXNAMELEN + 1]; // 0x1C
		TResourceState m_State;      // 0x2B
		unsigned int m_UId;          // 0x2C
	};
}

