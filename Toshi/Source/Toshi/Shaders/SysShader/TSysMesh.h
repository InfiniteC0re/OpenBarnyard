#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TMesh.h"

namespace Toshi {
	
	class TResource;

	class TSysMesh : 
		public TGenericClassDerived<TSysMesh, TMesh, "TSysMesh", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		typedef uint32_t FLAGS;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE   = 0,
			FLAGS_LOCKED = 0x8000
		};

		struct TLockBuffer
		{
			int Unknown;             // 0x00
			void* pVertexBufferData; // 0x08
			void* pIndexBufferData;  // 0x2C
		};

	public:
		TSysMesh();

		virtual TBOOL Validate() override;
		virtual void Invalidate() override;
		virtual TBOOL Create(FLAGS uiFlags, uint16_t uiMaxVertices, uint16_t uiMaxIndices);
		virtual TBOOL Lock(TLockBuffer* pLockBuffer);
		virtual void Unlock(uint16_t uiNumVertices, uint16_t uiNumIndices);
		virtual TResource* GetVertexPool() const;
		virtual TResource* GetIndexPool() const;
		virtual void SetZBias(int zBias) = 0;
		virtual void SetAlphaRef() { }

		TBOOL Allocate();
		void Deallocate();

	protected:
		uint32_t m_uiFlags;
		uint16_t m_uiMaxVertices;
		uint16_t m_uiMaxIndices;
		TResource* m_pVertexPool;
		TResource* m_pIndexPool;
	};

}