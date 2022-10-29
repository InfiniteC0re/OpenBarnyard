#pragma once

#include "Toshi/Render/TRender.h"
#include "Toshi/Core/TNodeTree.h"

namespace Toshi
{
	enum TResourceState
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
		virtual bool Create();
		virtual bool Validate();
		virtual void DestroyResource();

		inline bool IsDying() const { return m_State & TResourceState_Dying; }
		inline bool IsValid() const { return m_State & TResourceState_Valid; }
		inline bool IsCreated() const { return m_State & TResourceState_Created; }
		inline bool IsSceneObject() const { return m_State & TResourceState_SceneObject; }
		
		inline bool IsInvalid() const { return ~m_State & TResourceState_Valid; }

		inline TRender* GetRenderer() const { return m_pRenderer; }
		TResource* Parent() const;

	public:
		TRender* m_pRenderer; // 0x18
		char* m_cName; // 0x1C
		// 0x20
		// 0x24
		// 0x28
		// 0x2A
		uint8_t m_State; // 0x2B
		unsigned int m_uiUId; // 0x2C
	};

}

