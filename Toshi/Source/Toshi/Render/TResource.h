#pragma once

#include "Toshi/Core/TRender.h"
#include "Toshi/Core/TNodeTree.h"

namespace Toshi
{
	class TResource : TObject, TNodeTree<TResource>::TNode
	{

	public:

		const int FLAGS_CREATED = 2;
		const int FLAGS_DYING = 4;
		const int FLAGS_DEAD = 16;
		const int FLAGS_SCENEOBJECT = 32;
		const int FLAGS_INVALID = 254;

		TRender* m_pRenderer; // 0x18
		char* m_cName; // 0x1C
		// 0x20
		// 0x24
		// 0x28
		// 0x2A
		uint8_t m_Flags; // 0x2B
		unsigned int m_uiUId; // 0x2C

		virtual bool Create();
		virtual bool Validate();
		virtual void DestroyResource();

		inline bool IsCreated() { return (m_Flags & FLAGS_CREATED) != 0; }
		inline bool IsDying() { return (m_Flags & FLAGS_DYING) != 0; }
		inline bool IsSceneObject() { return (m_Flags & FLAGS_SCENEOBJECT) != 0; }

		inline TRender* GetRenderer() { return m_pRenderer; }
		TResource* Parent() const;

	};

}

